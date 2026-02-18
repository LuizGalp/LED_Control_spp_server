/*
 * ble_spp_server.h
 */

#ifndef H_BLESPPSERVER_
#define H_BLESPPSERVER_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 16 Bit SPP Service UUID */
#define BLE_SVC_SPP_UUID16                                  0xABF0

/* 16 Bit SPP Service Characteristic UUID */
#define BLE_SVC_SPP_CHR_UUID16                              0xABF1

struct ble_hs_adv_fields;
struct ble_gap_conn_desc;
struct ble_hs_cfg;
union ble_store_value;
union ble_store_key;

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;
extern uint16_t ble_spp_svc_gatt_read_val_handle;
void ble_spp_init(void);
void ble_spp_server_host_task(void *param);
void ble_forget_all_bonds(void);


#ifdef __cplusplus
}
#endif

#endif
