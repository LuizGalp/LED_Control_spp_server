/*
 * system_state.c 
 * Faz o controle do botao e começo e fim do modo de pareamento
 */
 #include "driver/gpio.h"
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "esp_log.h"
 #include "system_state.h"
 #include "LedControl/LedFunctions.h"
 #include "BleControl/ble_spp_server.h"
#include "rom/ets_sys.h"
#include <stdbool.h>
#include <stdint.h>
 /* ===== Define Botao ===== */
 #define GPIO_BUTTON   2

 /* ===== VARIAVEIS ===== */
 #define PAIRING_HOLD_TIME_MS     	5000
 // 5 segundos 
 #define PAIRING_RESET_TIME_MS     	7000      
 // 7 segundos 
 #define PAIRING_TIMEOUT_MS      60000 
 // 2 minutos
 #define BLINK_INTERVAL_MS       1000    
 // 1 segundo
 
 /* ===================== GLOBAL STATE ===================== */
 volatile  bool ble_pairing_allowed = false;
 //para o ble saber está no modo de paring
 static uint32_t pairing_time_ms = 0;
 volatile bool pairing_mode = false;
 volatile bool reset_mode = false;
 // para o ble saber que é para dar reset
 //Obs:separar depois um .c só para controle de botão talvez fique mais organizado

 /* ===================== VARIAVEIS LOCAIS ===================== */
 static uint32_t button_hold_ms = 0;
 static uint32_t blink_time_ms = 0;
 static bool parar_log = false;
 /* ===================== PROTOTIPOS ===================== */

 /* ===================== COMEÇO ===================== */
 //adicionando botão de controle
 void button_init(void)
 {
     gpio_config_t io_conf = {
         .pin_bit_mask = 1ULL << GPIO_BUTTON,
         .mode         = GPIO_MODE_INPUT,
         .pull_up_en   = GPIO_PULLUP_ENABLE,   // botão puxa para GND
         .pull_down_en = GPIO_PULLDOWN_DISABLE,
         .intr_type    = GPIO_INTR_DISABLE
     };
     gpio_config(&io_conf);
 }
 #define BUT_ON 0
 #define BUT_OFF 1
 
 void button_pairing_task(void *pvParameters)
 {
     const uint32_t poll_delay_ms = 50;

     while (1) {
         int button_state = gpio_get_level(GPIO_BUTTON);

 		/* ======================================
         * CONTAGEM DO BOTÃO (SEMPRE)
         * ====================================== */
        if (button_state == 0) 
		{  // botão pressionado (pull-up)
            button_hold_ms += poll_delay_ms;

            /* ===== 5s → ENTRA EM PAREAMENTO ===== */
            if (button_hold_ms >= PAIRING_HOLD_TIME_MS && button_hold_ms < PAIRING_RESET_TIME_MS && pairing_mode == false && parar_log==false) {

                leds_on();

                ESP_LOGI("PAIRING", "Solte para ativar o modo de pareamento");
				parar_log=true;
            }

            /* ===== 7s → RESET BLE ===== */
            if (button_hold_ms >= PAIRING_RESET_TIME_MS && parar_log==true) {
				leds_off();
				parar_log = false;
                ESP_LOGW("BLE", "Factory reset triggered by button");			
            }

        } else {
            /* ======================================
             * BOTÃO SOLTO → RESET CONTADOR
             * ====================================== */
			 if (button_hold_ms >= PAIRING_HOLD_TIME_MS && button_hold_ms < PAIRING_RESET_TIME_MS && pairing_mode == false)
			 {
				parar_log=false;
				pairing_mode = true;
				ble_pairing_allowed = true;
				pairing_time_ms = 0;
				blink_time_ms = 0;
				ESP_LOGI("PAIRING", "Modo de pareamento ATIVADO");

			 }
			 if (button_hold_ms >= PAIRING_RESET_TIME_MS)
			 {
				ble_forget_all_bonds();
			 }
            button_hold_ms = 0;
        }
         
 		
 		

         /* ======================================
          * MODO DE PAREAMENTO ATIVO
          * ====================================== */
         if (pairing_mode) {
             pairing_time_ms += poll_delay_ms;
             blink_time_ms += poll_delay_ms;

             /* Pisca LED a cada 1 segundo */
             if (blink_time_ms >= BLINK_INTERVAL_MS) {
                 blink_time_ms = 0;
                 pairing_blink_leds();
             }

             /* Sai do pareamento após 2 minutos */
             if (pairing_time_ms >= PAIRING_TIMEOUT_MS) {
                 pairing_mode = false;
 				ble_pairing_allowed= false;
                 pairing_time_ms = 0;
                 blink_time_ms = 0;
                 leds_off();
                 ESP_LOGI("PAIRING", "Modo de pareamento FINALIZADO");
             }
         }

         /* ======================================
          * DELAY COOPERATIVO (FreeRTOS)
          * ====================================== */
         vTaskDelay(pdMS_TO_TICKS(poll_delay_ms));
     }
 }
 
 
 /* ===================== COMPARTILHANDO AS VARIAVEIS GLOBAIS  ===================== */
 bool button_is_pairing_allowed(void)
 {
     return ble_pairing_allowed;
 }
 bool teste_pairing_mode(bool mode)
 {
	 pairing_mode = mode;
	 
	 if (pairing_mode == false)
	 {
		ble_pairing_allowed = false;
		pairing_time_ms = 0;
		blink_time_ms = 0;
		leds_off();
		ESP_LOGI("PAIRING", "Modo pareamento forçado OFF");		
	 }

	 return pairing_mode;
 }

