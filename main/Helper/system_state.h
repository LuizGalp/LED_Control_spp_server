/*
 * system_state.h
 *
 *  Created on: 4 de fev. de 2026
 *      Author: GalpTelegestao
 */

#ifndef MAIN_HELPER_SYSTEM_STATE_H_
#define MAIN_HELPER_SYSTEM_STATE_H_

/**
 * @brief Inicia o pino do botao
 */
void button_init(void);


/**
 * @brief Controla a ativação do modo de PAREAMENTO
 */
void button_pairing_task(void*pvParameters);

#include <stdbool.h>
#include <stdint.h>
/* ===== API DE ESTADO ===== */
/**
 * @brief Passa o estado de pareamento
 */
bool button_is_pairing_allowed(void);


/**
 * @brief Recebe mudança no estado de pareamento
 */
bool teste_pairing_mode(bool mode);

#endif /* MAIN_HELPER_SYSTEM_STATE_H_ */
