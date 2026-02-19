/*
 *  MAIN.C
 */
#include "nvs.h"
#include "nvs_flash.h"
#include "nimble/nimble_port_freertos.h"
#include "BleControl/ble_spp_server.h"
#include "Helper/system_state.h"
#include "LedControl/LedFunctions.h"
#include "nimble/nimble_port.h"
#include "esp_peripheral.h"

//TESTE DA NVS
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "NVS_TEST";

void nvs_write_test(void)
{
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS (%s)", esp_err_to_name(err));
        return;
    }

    uint32_t test_value = 123456;

    err = nvs_set_u32(handle, "counter", test_value);
    if (err == ESP_OK) {
        nvs_commit(handle);
        ESP_LOGI(TAG, "Value written: %lu", test_value);
    } else {
        ESP_LOGE(TAG, "Write failed (%s)", esp_err_to_name(err));
    }

    nvs_close(handle);
}
void nvs_read_test(void)
{
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS (%s)", esp_err_to_name(err));
        return;
    }

    uint32_t value = 0;

    err = nvs_get_u32(handle, "counter", &value);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Value read: %lu", value);
    } else {
        ESP_LOGW(TAG, "Key not found (%s)", esp_err_to_name(err));
    }

    nvs_close(handle);
}


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
	button_init();
	ble_spp_init();
	nvs_read_test();
	xTaskCreate(
		    button_pairing_task,
		    "button_pairing_task",
		    2048,
		    NULL,
		    5,
		    NULL
		);


}
