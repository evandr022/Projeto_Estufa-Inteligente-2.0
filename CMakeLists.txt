# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Projeto_Estufa_Inteligente C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# =================================================================
#  ADICIONAR FREERTOS (NOVO)
# =================================================================
# 1. Configura a biblioteca de interface para o ficheiro de configuração
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE include)

# Isto permite que o FreeRTOSConfig.h encontre "pico/stdlib.h"
target_link_libraries(freertos_config INTERFACE pico_stdlib)

# CORREÇÃO AQUI: Mudado de PUBLIC para INTERFACE
target_compile_definitions(freertos_config INTERFACE PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

# 2. Define opções do FreeRTOS
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Kernel)
set(FREERTOS_PORT GCC_RP2040 CACHE STRING "" FORCE)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE) # Define Heap 4 automaticamente

# 3. Adiciona o Kernel
add_subdirectory(${FREERTOS_KERNEL_PATH})
# =================================================================

# Add executable. Default name is the project name, version 0.1

add_library(library
    src/BH1750.cpp
    src/lcd_i2c.cpp
    src/AHT10.cpp
    src/Servo.cpp
    src/http_post.cpp
)

target_include_directories(library PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(library 
    pico_stdlib
    hardware_i2c
    hardware_pwm
    freertos_kernel
    pico_cyw43_arch_lwip_sys_freertos
    pico_lwip_freertos
)

add_executable(Projeto_Estufa_Inteligente
    main.cpp
)

target_link_libraries(Projeto_Estufa_Inteligente
    pico_stdlib
    pico_cyw43_arch_lwip_sys_freertos
    pico_lwip_freertos
    hardware_pio
    hardware_i2c
    hardware_adc
    hardware_pwm
    library
    freertos_kernel
)

target_compile_definitions(Projeto_Estufa_Inteligente PRIVATE
    PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=0
    PICO_LWIP_LOCAL_CUSTOM_LWIPOPTS_INCLUDE_PATH=1
)

target_include_directories(Projeto_Estufa_Inteligente PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/include
)

 #if (PICO_CYW43_SUPPORTED)
    # Esta função configura internamente os caminhos de include para o pico/cyw43_arch.h
     #target_link_libraries(Projeto_Estufa_Inteligente pico_cyw43_arch_lwip_sys_freertos)
 #endif()

pico_set_program_name(Projeto_Estufa_Inteligente "Projeto_Estufa_Inteligente")
pico_set_program_version(Projeto_Estufa_Inteligente "2.0")

pico_enable_stdio_uart(Projeto_Estufa_Inteligente 1)
pico_enable_stdio_usb(Projeto_Estufa_Inteligente 1)

pico_add_extra_outputs(Projeto_Estufa_Inteligente)