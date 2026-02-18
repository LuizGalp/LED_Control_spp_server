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


struct ble_hs_cfg;
struct ble_gatt_register_ctxt;
extern uint16_t ble_spp_svc_gatt_read_val_handle;
void ble_spp_init(void);
void ble_spp_server_host_task(void *param);
void ble_forget_all_bonds(void);

/* Estados do BLE */
typedef enum {
    BLE_STATE_IDLE = 0,
    BLE_STATE_PAIRING,
    BLE_STATE_BONDED
} ble_state_t;

/* Controle de estado */
void ble_set_state(ble_state_t new_state);
ble_state_t ble_get_state(void);
//fim da adição de paring

#ifdef __cplusplus
}
#endif

#endif
