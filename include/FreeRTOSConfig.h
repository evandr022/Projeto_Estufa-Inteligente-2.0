#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// --- Configurações Básicas ---
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      133000000 // Frequência do Pico (133MHz)
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                (128)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   3
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               10
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5
#define configSTACK_DEPTH_TYPE                  uint16_t
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   (64 * 1024) // 64KB de memória para o RTOS
#define configAPPLICATION_ALLOCATED_HEAP        0
#define INCLUDE_vTaskDelay                      1

// --- Hooks (Funções de retorno) ---
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

// --- Temporizadores de Software ---
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE
#define INCLUDE_xTimerPendFunctionCall          1

// --- Mapeamento para funções padrão do Pico SDK ---
#include "pico/stdlib.h"
#define vLoggingPrintf                          printf

// A definição correta para asserts no Pico
#define configASSERT( x )                       if( ( x ) == 0 ) { portDISABLE_INTERRUPTS(); for( ;; ); }

// 1. Habilita a função de suspender tarefas exigida pelo LWIP
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xSemaphoreGetMutexHolder        1
#define INCLUDE_vTaskDelete                     1

// 2. Resolve o problema de compatibilidade de nome da versão do FreeRTOS
#define portTICK_RATE_MS                        portTICK_PERIOD_MS

// PREVENÇÃO: O LWIP também costuma exigir semáforos contadores para as "mailboxes" de rede.
// Se não tiver essa linha, adicione-a ou mude para 1:
#define configUSE_COUNTING_SEMAPHORES           1

#endif /* FREERTOS_CONFIG_H */