#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/sockets.h"    
#include "lwip/ip_addr.h"
#include "http_post.hpp"

// Bibliotecas FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Bibliotecas do Projeto 
#include "lcd_i2c.hpp"
#include "BH1750.hpp"
#include "AHT10.hpp"
#include "Servo.hpp"

// ----- CONFIGURAÇÃO ----- //
#define I2C_PORT i2c1
#define PIN_SDA 2
#define PIN_SCL 3
#define PIN_SERVO 15

// ----- OBJETOS GLOBAIS ----- //
LcdI2C lcd(I2C_PORT, PIN_SDA, PIN_SCL);
BH1750 sensorLuz(I2C_PORT);
AHT10 sensorClima(I2C_PORT);
Servo servoTeto(PIN_SERVO);

// Variáveis de Dados Globais
float g_lux = -1.0;
float g_temp = 0.0;
float g_hum = 0.0;
bool g_bh1750_ok = false;
bool g_aht10_ok = false;

SemaphoreHandle_t i2cMutex;

// Função auxiliar LCD
void print_safe(const char* texto) {
    char buffer_limpo[17];
    snprintf(buffer_limpo, 17, "%-16s", texto); 
    lcd.print(buffer_limpo);
}

// --------------------------------------------------------------------------
// TAREFA 1: Sensores
// --------------------------------------------------------------------------
void sensorTask(void *pvParameters) {
    // Timeout de 1000ms para evitar travamento total
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        if(sensorClima.begin()) {
            g_aht10_ok = true;
            printf("AHT10 Inicializado via Lib!\n");
        }
        xSemaphoreGive(i2cMutex);
    }

    while (true) {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            // Leitura BH1750
            if (g_bh1750_ok) {
                float l = sensorLuz.readLightLevel();
                if (l >= 0) g_lux = l;
            }

            // Leitura AHT10
            float h, t;
            if (sensorClima.readSensor(&h, &t)) {
                g_temp = t;
                g_hum = h;
                g_aht10_ok = true; 
            } else {
                printf("Erro leitura AHT10\n");
            }

            printf("Sensores: Lux=%.1f T=%.1f H=%.1f\n", g_lux, g_temp, g_hum);
            xSemaphoreGive(i2cMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// --------------------------------------------------------------------------
// TAREFA 2: Display
// --------------------------------------------------------------------------
void displayTask(void *pvParameters) {
    int estadoTela = 0;
    char buffer[20];

    while (true) {
        // Tenta pegar o controle do barramento I2C
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
            
            // Sempre volta para a primeira linha, primeira coluna
            lcd.set_cursor(0, 0); 
            
            switch (estadoTela) {
                case 0: // LUX
                    if (g_bh1750_ok) {
                        // O %-16s ou colocar espaços manuais garante que o texto 
                        // antigo seja apagado sem precisar dar lcd.clear()
                        snprintf(buffer, sizeof(buffer), "Lux: %.1f       ", g_lux);
                    } else {
                        snprintf(buffer, sizeof(buffer), "Erro Sens. Luz  ");
                    }
                    print_safe(buffer);
                    break;
                    
                case 1: // TEMP
                    if (g_aht10_ok) {
                        snprintf(buffer, sizeof(buffer), "Temp: %.1f C    ", g_temp);
                    } else {
                        snprintf(buffer, sizeof(buffer), "Erro Sens. Temp "); 
                    }
                    print_safe(buffer);
                    break;
                    
                case 2: // UMID
                    if (g_aht10_ok) {
                        snprintf(buffer, sizeof(buffer), "Umid: %.1f %%    ", g_hum);
                    } else {
                        snprintf(buffer, sizeof(buffer), "Erro Sens. Umid ");
                    }
                    print_safe(buffer);
                    break;
            }
            
            // Libera o barramento I2C para os sensores poderem ler os novos dados
            xSemaphoreGive(i2cMutex);
        }
        
        // Passa para a próxima tela
        estadoTela++;
        if (estadoTela > 2) estadoTela = 0;
        
        // Atualiza a tela a cada 2 segundos. 
        // Esse atraso é importante para dar tempo aos sensores de lerem as variações.
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
// --------------------------------------------------------------------------
// TAREFA 3: Controle do Servo (Teto)
// --------------------------------------------------------------------------
void servoTask(void *pvParameters) {
    int angulo = 0;
    while (true) {
        // Alterna entre 0 e 90 graus a cada 2 segundos
        angulo = (angulo == 0) ? 90 : 0;
        servoTeto.setAngle(angulo);
        
        // Aguarda 2 segundos antes de mudar de novo
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// --------------------------------------------------------------------------
// TAREFA 4: Conectividade (HTTP POST)
// --------------------------------------------------------------------------
void netTask(void *pvParameters) {
    // Pequeno atraso para deixar as outras tarefas iniciarem com folga
    vTaskDelay(pdMS_TO_TICKS(1000)); 

    // Inicializa a arquitetura Wi-Fi
    if (cyw43_arch_init()) {
        printf("Falha ao inicializar Wi-Fi\n");
        vTaskDelete(NULL);
    }
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    
    // Conecta usando as macros que estão definidas no seu http_post.h
    if (cyw43_arch_wifi_connect_blocking(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK)) {
        printf("Erro ao conectar no Wi-Fi\n");
    } else {
        printf("Wi-Fi Conectado!\n");
        http_init(); 
    }

    while (true) {
        if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP) {
            
            const char* status_teto = (g_temp > 30.0f) ? "ABERTO" : "FECHADO";
            
            printf("Enviando HTTP POST para o Node-RED...\n");
            http_post_json(g_temp, g_hum, status_teto);
            
        } else {
            printf("Aguardando conexao Wi-Fi...\n");
        }
        
        // Dispara os dados a cada 10 segundos
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}


// ----- HOOKS DO FREERTOS ----- //
extern "C" {
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
        #ifdef PICO_DEFAULT_LED_PIN
        gpio_init(PICO_DEFAULT_LED_PIN); gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        while(1) { gpio_put(PICO_DEFAULT_LED_PIN, 1); sleep_ms(50); gpio_put(PICO_DEFAULT_LED_PIN, 0); sleep_ms(50); }
        #else
        while(1);
        #endif
    }

    void vApplicationMallocFailedHook(void) {
        #ifdef PICO_DEFAULT_LED_PIN
        gpio_init(PICO_DEFAULT_LED_PIN); gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        while(1) { gpio_put(PICO_DEFAULT_LED_PIN, 1); sleep_ms(1000); gpio_put(PICO_DEFAULT_LED_PIN, 0); sleep_ms(1000); }
        #else
        while(1);
        #endif
    }
}

// ----- FUNÇÃO PRINCIPAL ----- //
int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("=== Estufa Inteligente c/ HTTP POST ===\n");

    // Inicializa I2C
    i2c_init(I2C_PORT, 100000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);

    // Mutex do I2C
    i2cMutex = xSemaphoreCreateMutex();

    // Setup Display
    lcd.init();
    lcd.set_cursor(0,0);
    print_safe("Sistema Ligado");

    // Setup Sensor de Luz
    if (sensorLuz.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23) || 
        sensorLuz.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C)) {
        g_bh1750_ok = true;
    }

    sleep_ms(1000);

    // Setup Servo
    servoTeto.setAngle(0);

    // Cria as Tarefas do FreeRTOS
    xTaskCreate(sensorTask, "Sensores", 2048, NULL, 1, NULL);
    xTaskCreate(displayTask, "Display", 2048, NULL, 1, NULL);
    xTaskCreate(servoTask, "Servo", 1024, NULL, 1, NULL);
    xTaskCreate(netTask, "Rede_HTTP", 4096, NULL, 1, NULL);

    // Inicia o Escalonador
    vTaskStartScheduler();

    while (true) {};
}