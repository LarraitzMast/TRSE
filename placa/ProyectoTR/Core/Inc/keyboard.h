/******************************************************************************
  * @file           : keyboard.h
  * @brief          : Header for keyboard.c file.
  *                   Fichero que contine distintas definiciones para la gestion del
  *                   teclado.
  *******************************************************************************/
 #ifndef __KEYBOARD_H__
 #define __KEYBOARD_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "main.h"

// Estado del keyboard
#define SENDING_TLF_NUMBER  1
#define SENDING_EMERGENCY   2

// Definición de pines
#define REG_KEY_COL_0	GPIOA
#define PIN_KEY_COL_0	GPIO_PIN_10
#define REG_KEY_COL_1	GPIOB
#define PIN_KEY_COL_1	GPIO_PIN_0
#define REG_KEY_COL_2	GPIOC
#define PIN_KEY_COL_2	GPIO_PIN_1
#define REG_KEY_COL_3	GPIOC
#define PIN_KEY_COL_3	GPIO_PIN_0

#define REG_KEY_ROW_0	GPIOA
#define PIN_KEY_ROW_0	GPIO_PIN_6
#define REG_KEY_ROW_1	GPIOA
#define PIN_KEY_ROW_1	GPIO_PIN_7
#define REG_KEY_ROW_2	GPIOC
#define PIN_KEY_ROW_2	GPIO_PIN_8
#define REG_KEY_ROW_3	GPIOC
#define PIN_KEY_ROW_3	GPIO_PIN_9




void writeColum(uint8_t numOfCol, GPIO_PinState PinState);
uint8_t readRow(uint8_t numOfRow);
uint8_t delayKey();
void handleKeyBoard(uint8_t row);
int8_t isANumber(char character);
int8_t isAnEmergencyType(char character);
void stateKeyBoard(char rxCharacter);

#ifdef __cplusplus
}
#endif
#endif /* _KEYBOARD_H */
