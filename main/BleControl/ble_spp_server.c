/* BLE  ble_spp_server.c */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nvs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble_spp_server.h"
#include "driver/uart.h"
//#include "Helper/system_state.h"
#include "modlog/modlog.h"
//#include "esp_peripheral.h"
#include "nimble/ble.h"
#include "store/config/ble_store_config.h"
#include "esp_log.h"

/* ===================== BLE / NIMBLE ===================== */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_sm.h"
#include "services/gap/ble_svc_gap.h"
#include "store/config/ble_store_config.h"
//
#include <stdbool.h>
#include "nimble/ble.h"
#include "modlog/modlog.h"
//
#include <inttypes.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "syscfg/syscfg.h"
#include "host/ble_hs.h"
#include "store/config/ble_store_config.h"
//#include "ble_store_config_priv.h"
//

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "BleControl/bleprph.h"
#include "services/ans/ble_svc_ans.h"
/* ===================== ESP-IDF ===================== */
#include "esp_system.h"
#include "esp_random.h"
#include "driver/uart.h"
#include "modlog/modlog.h"
//#include "esp_central.h"
#include "nvs_flash.h"
/* ===================== APP ===================== */
//#include "BleControl/ble_spp_client.h"
//É necessario chamar a Led aqui para ativala ao receber notificação
//e pisca-la quando entrar em modo de pareamento 
#include "LedControl/LedFunctions.h"
#include "Helper/system_state.h"


static int ble_spp_server_gap_event(struct ble_gap_event *event, void *arg);
static uint8_t own_addr_type;
int gatt_svr_register(void);
QueueHandle_t spp_common_uart_queue = NULL;
bool conn_handle_subs[CONFIG_BT_NIMBLE_MAX_CONNECTIONS + 1];
uint16_t ble_spp_svc_gatt_read_val_handle;
void ble_store_config_init();
/**
 * Logs information about a connection to the console.
 */


 static void
 ble_spp_server_print_conn_desc(struct ble_gap_conn_desc *desc)
 {
     MODLOG_DFLT(INFO, "handle=%d our_ota_addr_type=%d our_ota_addr=%s",
                 desc->conn_handle, desc->our_ota_addr.type, 
                 addr_str(desc->our_ota_addr.val));

     MODLOG_DFLT(INFO, " our_id_addr_type=%d our_id_addr=%s",
                 desc->our_id_addr.type, 
                 addr_str(desc->our_id_addr.val));

     MODLOG_DFLT(INFO, " peer_ota_addr_type=%d peer_ota_addr=%s",
                 desc->peer_ota_addr.type, 
                 addr_str(desc->peer_ota_addr.val));

     MODLOG_DFLT(INFO, " peer_id_addr_type=%d peer_id_addr=%s",
                 desc->peer_id_addr.type, 
                 addr_str(desc->peer_id_addr.val));

     MODLOG_DFLT(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                 "encrypted=%d authenticated=%d bonded=%d\n",
                 desc->conn_itvl, desc->conn_latency,
                 desc->supervision_timeout,
                 desc->sec_state.encrypted,
                 desc->sec_state.authenticated,
                 desc->sec_state.bonded);
 }

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void
ble_spp_server_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    const char *name;
    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(BLE_SVC_SPP_UUID16)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, ble_spp_server_gap_event, NULL);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * ble_spp_server uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unused by
 *                                  ble_spp_server.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
 
 static int
 ble_spp_server_gap_event(struct ble_gap_event *event, void *arg)
 {
     struct ble_gap_conn_desc desc;
     int rc;

     switch (event->type) {

     /* =====================================================
      * CONNECT EVENT
      * ===================================================== */
     case BLE_GAP_EVENT_CONNECT: {

		MODLOG_DFLT(INFO, "connection %s; status=%d ",
		                    event->connect.status == 0 ? "established" : "failed",
		                    event->connect.status);

		if (event->connect.status == 0)//Se está conectado...
		{
			rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
			assert(rc == 0);
			
			ESP_LOGI("SEC","Bonded=%d Encrypted=%d Authenticated=%d", desc.sec_state.bonded,desc.sec_state.encrypted,desc.sec_state.authenticated);
			
			
				ESP_LOGI("SEC","Inicializa a segurança antes");
				ble_gap_security_initiate(event->connect.conn_handle);
			
			ble_spp_server_print_conn_desc(&desc);
		}

		 /*Se a conexão falhar e ou poder ter mais conexão volta a proucurar*/
		 if (event->connect.status != 0 || CONFIG_BT_NIMBLE_MAX_CONNECTIONS > 1) {
		 	ble_spp_server_advertise();
		 }
         return 0;
     }

     /* =====================================================
      * REPEAT PAIRING EVENT
      * ===================================================== */
     case BLE_GAP_EVENT_REPEAT_PAIRING:

         ESP_LOGW("SEC", "Repeat pairing detected → deleting old bond");
		 ESP_LOGI("SEC", "========================================================= estava pareado =========================================================");
		 ESP_LOGI("SEC", "id do server; peer_id_addr=%d",desc.peer_id_addr);

         rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
         if (rc == 0) {
             ble_store_util_delete_peer(&desc.peer_id_addr);
         }

         return BLE_GAP_REPEAT_PAIRING_RETRY;

     /* =====================================================
      * ENCRYPTION CHANGE EVENT
      * ===================================================== */
     case BLE_GAP_EVENT_ENC_CHANGE:
	 /* Ao usar a função ble_gap_security_initiate() entra aqui */
         if (event->enc_change.status == 0) {
			
             ESP_LOGI("SEC","Encryption successful; conn_handle=%d",event->enc_change.conn_handle);
			 //Segurança / exigencia de pareamento ou bounding
			 if (desc.sec_state.bonded || button_is_pairing_allowed())
			 {
			 		ESP_LOGI("SEC","Segurança ativada, permite pareamento");
					//Se está aqui quer dizer que já está conectado pode parar de proucurar
					teste_pairing_mode(false);
					ESP_LOGI("SEC", "Pairing mode disabled after success.");
					//apos conectar com segurança o bonded deve ser igual a 1
					ESP_LOGI("SEC","Bonded=%d",desc.sec_state.bonded);
					ESP_LOGI("SEC","peers id addr=%d",desc.peer_id_addr);
					ESP_LOGI("SEC","key_size=%d",desc.sec_state.key_size);
			 }else{
			 		//Se não está pareando e nem já salvo desconecta para proucurar por outro
			 		ESP_LOGI("SEC","Fora do modo de pareamento e sem bounding");
			 		ble_gap_terminate(event->connect.conn_handle, BLE_ERR_AUTH_FAIL);
			 }

			         
         } else {
			ESP_LOGE("SEC","Encryption failed; status=%d",event->enc_change.status);
			ble_gap_terminate(event->enc_change.conn_handle,
                               BLE_ERR_AUTH_FAIL);
         }
         return 0;

     /* =====================================================
      * SUBSCRIBE EVENT
      * ===================================================== */
     case BLE_GAP_EVENT_SUBSCRIBE:

         rc = ble_gap_conn_find(event->subscribe.conn_handle, &desc);

         if (rc == 0 && desc.sec_state.encrypted) {

             conn_handle_subs[event->subscribe.conn_handle] = true;

             ESP_LOGI("GATT",
                      "Client subscribed (encrypted link)");

         } else {

             ESP_LOGW("GATT",
                      "Subscription rejected (not encrypted)");

             conn_handle_subs[event->subscribe.conn_handle] = false;
         }

         return 0;

     /* =====================================================
      * DISCONNECT EVENT
      * ===================================================== */
     case BLE_GAP_EVENT_DISCONNECT:

         MODLOG_DFLT(INFO,
                     "Disconnect; reason=%d",
                     event->disconnect.reason);

         ble_spp_server_print_conn_desc(&event->disconnect.conn);

         conn_handle_subs[event->disconnect.conn.conn_handle] = false;

         /* Sempre voltar a anunciar */
         ble_spp_server_advertise();

         return 0;

     /* =====================================================
      * MTU EVENT
      * ===================================================== */
     case BLE_GAP_EVENT_MTU:

         MODLOG_DFLT(INFO,
                     "MTU updated; conn_handle=%d mtu=%d",
                     event->mtu.conn_handle,
                     event->mtu.value);

         return 0;

     /* =====================================================
      * PHY UPDATE EVENT
      * ===================================================== */
     case BLE_GAP_EVENT_PHY_UPDATE_COMPLETE:

         MODLOG_DFLT(INFO,
                     "PHY updated; status=%d rx=%d tx=%d",
                     event->phy_updated.status,
                     event->phy_updated.rx_phy,
                     event->phy_updated.tx_phy);

         return 0;

     /* =====================================================
      * CONNECTION PARAM UPDATE
      * ===================================================== */
     case BLE_GAP_EVENT_CONN_UPDATE:

         rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
         if (rc == 0) {
             ble_spp_server_print_conn_desc(&desc);
         }

         return 0;

     default:
         return 0;
     }
 }



static void
ble_spp_server_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

static void
ble_spp_server_on_sync(void)
{
    int rc;

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);
	MODLOG_DFLT(INFO, "Device Address: %s\n", addr_str(addr_val));
    /* Begin advertising. */
    ble_spp_server_advertise();
}

void ble_spp_server_host_task(void *param)
{
    MODLOG_DFLT(INFO, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}


void ble_server_uart_task(void *pvParameters)
{
    MODLOG_DFLT(INFO, "BLE server UART_task started\n");
    uart_event_t event;
    int rc = 0;
    for (;;) {
        //Waiting for UART event.
        if (xQueueReceive(spp_common_uart_queue, (void * )&event, (TickType_t)portMAX_DELAY))            {
            switch (event.type) {
            //Event of UART receiving data
            case UART_DATA:
                if (event.size) {
                    uint8_t *ntf;
                    ntf = (uint8_t *)malloc(sizeof(uint8_t) * event.size);
                    memset(ntf, 0x00, event.size);
                    uart_read_bytes(UART_NUM_0, ntf, event.size, portMAX_DELAY);

                    for (int i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
                        /* Check if client has subscribed to notifications */
						
                        if (conn_handle_subs[i]) {
                            struct os_mbuf *txom;
                            txom = ble_hs_mbuf_from_flat(ntf, event.size);
                            rc = ble_gatts_notify_custom(i, ble_spp_svc_gatt_read_val_handle,
                                                         txom);
                            if (rc == 0) {
                                MODLOG_DFLT(INFO, "Notification sent successfully");
                            } else {
                                MODLOG_DFLT(INFO, "Error in sending notification rc = %d", rc);
                            }
                        }
                    }

		    free(ntf);
                }
                break;
            default:
                break;
            }
        }
    }
    vTaskDelete(NULL);
}


void ble_forget_all_bonds(void)
{
	ESP_LOGW("SYS", "Performing Total Factory Reset...");
		
	    // Desinicializa o BLE para liberar a NVS
	    nimble_port_stop();
	    
	    // Apaga a partição NVS padrão
	    nvs_flash_deinit();
	    nvs_flash_erase();
	    ESP_LOGI("SYS", "Flash erased. Restarting...");
	    esp_restart();
}


static void ble_spp_uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_RTS,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };
    //Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_0, 4096, 8192, 10, &spp_common_uart_queue, 0);
    //Set UART parameters
    uart_param_config(UART_NUM_0, &uart_config);
    //Set UART pins
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    xTaskCreate(ble_server_uart_task, "uTask", 4096, (void *)UART_NUM_0, 8, NULL);
}


void ble_spp_init(void)
{
	esp_err_t ret;
	ret = nimble_port_init();
	    if (ret != ESP_OK) {
	        MODLOG_DFLT(ERROR, "Failed to init nimble %d \n", ret);
	        return;
	    }

	    /* Initialize connection_handle array */
	    for (int i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
	        conn_handle_subs[i] = false;
	    }

	    /* Initialize uart driver and start uart task */
	    ble_spp_uart_init();

	    /* Initialize the NimBLE host configuration. */
	    ble_hs_cfg.reset_cb = ble_spp_server_on_reset;
	    ble_hs_cfg.sync_cb = ble_spp_server_on_sync;
	    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
	    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

		ble_hs_cfg.sm_bonding = 1;
		ble_hs_cfg.sm_mitm    = 0;
		ble_hs_cfg.sm_sc      = 1;   
		ble_hs_cfg.sm_io_cap  = BLE_SM_IO_CAP_NO_IO;
		ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
		ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
				
	    int rc;
		/* Register custom service */
	    rc = gatt_svr_init();
	    assert(rc == 0);

	    /* Set the default device name. */
	    rc = ble_svc_gap_device_name_set("nimble-ble-spp-svr");
	    assert(rc == 0);
		
		ble_store_config_init();
		
		nimble_port_freertos_init(ble_spp_server_host_task);
}