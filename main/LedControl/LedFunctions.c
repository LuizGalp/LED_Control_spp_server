#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LedControl/LedFunctions.h"
#include "esp_log.h"

/* ===== PINOS ORIGINAIS ===== */
#define GPIO_LED_INTERNO 48


#define GPIO_ACTIVE_LEVEL   1
#define GPIO_INACTIVE_LEVEL 0

/* ===== VARIAVEIS ===== */
#define PAIRING_HOLD_TIME_MS     5000   
// 5 segundos 
#define PAIRING_TIMEOUT_MS      60000 
// 2 minutos
#define BLINK_INTERVAL_MS       1000    
// 1 segundo

/* ===================== GLOBAL STATE ===================== */

static bool led_blink_state = false;

/* ========================== */
//prototipo
void pairing_blink_leds(void);


void led_init_gpio(gpio_num_t gpio)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << gpio,
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(gpio, GPIO_INACTIVE_LEVEL);
}

/* ===== FUNÇÕES SIMPLES ===== */

void leds_init(void)
{
    led_init_gpio(GPIO_LED_INTERNO);

}

void leds_on(void)
{
    gpio_set_level(GPIO_LED_INTERNO, GPIO_ACTIVE_LEVEL);
}

void leds_off(void)
{
    gpio_set_level(GPIO_LED_INTERNO, GPIO_INACTIVE_LEVEL);
}


static const char *TAG_LED = "LED_CMD";

void pairing_blink_leds(void)
{
    led_blink_state = !led_blink_state;

    if (led_blink_state) {
        leds_on();
		ESP_LOGI(TAG_LED, "LED ON");
    } else {
        leds_off();
		ESP_LOGI(TAG_LED, "LED OFF");
    }
}

	

