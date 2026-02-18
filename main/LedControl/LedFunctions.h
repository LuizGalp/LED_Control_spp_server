#ifndef LED_GPIO_H
#define LED_GPIO_H


/**
 * @brief Inicializa o pino do LED
 */
 void leds_init(void);

/**
 * @brief Liga o LED
 */
 void leds_on(void);
 
/**
 * @brief Desliga o LED
 */
 void leds_off(void);

 /**
  * @brief Apenas ativa e desativa os Leds
  */
 void pairing_blink_leds(void);

 
 /**
  * @brief recebe um char e pisca de acordo com um case
  */
 void led_handle_ble_command(char c);


#endif /* LED_GPIO_H */
