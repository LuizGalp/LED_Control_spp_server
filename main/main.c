/*
 *  MAIN.C
 */
#include "nvs_flash.h"
#include "nimble/nimble_port_freertos.h"
#include "BleControl/ble_spp_server.h"
#include "Helper/system_state.h"
#include "LedControl/LedFunctions.h"
#include "nimble/nimble_port.h"
#include "esp_peripheral.h"


void app_main(void)
{
	/* Initialize NVS — it is used to store PHY calibration data */
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	    ESP_ERROR_CHECK(nvs_flash_erase());
	    ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	leds_init();
	ble_spp_init();
	button_init();
	xTaskCreate(
		    button_pairing_task,
		    "button_pairing_task",
		    2048,
		    NULL,
		    5,
		    NULL
		);



}
