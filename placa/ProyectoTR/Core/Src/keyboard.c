/*******************************************************************************
 * @file           : keyboard.c
 * @brief          : Funciones correspondiente a la gestion del teclado
 *******************************************************************************/

#include "keyboard.h"

/* MAQUINA DE ESTADO*/

uint32_t current_millis = 0, previous_millis = 0;
uint8_t key_state = SENDING_TLF_NUMBER;

void stateKeyBoard(char rxCharacter) {

	//Si demanda tipo de emergencia siempre activar estado tipo de emergencia
	if (rxCharacter == ASK_TO_EMERGENCY) {
		key_state = SENDING_EMERGENCY;
		return;
	}

	switch (key_state) {
	case SENDING_TLF_NUMBER: // Estado donde se envia solo numeros
		if (!isANumber(rxCharacter))
			break;
		sendUARTMensaje(rxCharacter);
		break;
	case SENDING_EMERGENCY: // Estado donde se envia solo letras o tipo de emergencia
		if (!isAnEmergencyType(rxCharacter))
			break;
		key_state = SENDING_TLF_NUMBER;
		sendUARTMensaje(rxCharacter);
	}

}

int8_t isANumber(char character) {
	return character >= '0' && character <= '9';
}

int8_t isAnEmergencyType(char character) {
	return character >= 'A' && character <= 'D';
}

/* TRATAMIENTO DETECCION Y ENVION DEL KEY*/

char keypad[4][4] = { { '1', '2', '3', 'A' }, { '4', '5', '6', 'B' }, { '7',
		'8', '9', 'C' }, { '*', '0', '#', 'D' } };

//Retardo pare evitar rebotes
uint8_t delayKey() {
	current_millis = HAL_GetTick();
	if (current_millis - previous_millis <= 250) {
		return 1;
	}
	return 0;
}

//Enviar key del teclado
void handleKeyBoard(uint8_t row) {
	uint8_t column = 0;

	//Lectura de columana
	for (int i = 0; i < 4; i++) {
		writeColum(i, GPIO_PIN_RESET);
	}

	for (int i = 0; i < 4; i++) {
		writeColum(i, GPIO_PIN_SET);
		if (readRow(row) == GPIO_PIN_SET) {
			column = i;
		}
		writeColum(i, GPIO_PIN_RESET);
	}

	//Enviar key
	char msg = keypad[row][column];
	msgKey(msg);

	//Poner a espera parametros
	for (int i = 0; i < 4; i++) {
		writeColum(i, GPIO_PIN_SET);
	}

	previous_millis = current_millis;
}

//Escribir pinout columna keypad
void writeColum(uint8_t numOfCol, GPIO_PinState PinState) {
	switch (numOfCol) {
	case 0:
		HAL_GPIO_WritePin(REG_KEY_COL_0, PIN_KEY_COL_0, PinState);
		break;

	case 1:
		HAL_GPIO_WritePin(REG_KEY_COL_1, PIN_KEY_COL_1, PinState);
		break;

	case 2:
		HAL_GPIO_WritePin(REG_KEY_COL_2, PIN_KEY_COL_2, PinState);
		break;

	case 3:
		HAL_GPIO_WritePin(REG_KEY_COL_3, PIN_KEY_COL_3, PinState);
		break;
	}
}

//Leer pinout columna keypad
uint8_t readRow(uint8_t numOfRow) {
	uint8_t rowValue = 0;
	switch (numOfRow) {
	case 0:
		rowValue = HAL_GPIO_ReadPin(REG_KEY_ROW_0, PIN_KEY_ROW_0);
		break;

	case 1:
		rowValue = HAL_GPIO_ReadPin(REG_KEY_ROW_1, PIN_KEY_ROW_1);
		break;

	case 2:
		rowValue = HAL_GPIO_ReadPin(REG_KEY_ROW_2, PIN_KEY_ROW_2);
		break;

	case 3:
		rowValue = HAL_GPIO_ReadPin(REG_KEY_ROW_3, PIN_KEY_ROW_3);
		break;
	}
	return rowValue;
}


