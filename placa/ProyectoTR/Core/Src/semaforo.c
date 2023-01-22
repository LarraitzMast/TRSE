/*******************************************************************************
 * @file           : semaforo.c
 * @brief          : Funciones correspondiente a la gestion de semaforo & sirena
 *******************************************************************************/

#include "semaforo.h"


/*******************************
 * 			SEMAFORO
********************************/

/* Maquina de estado del ciclo SEMAFORO */

uint8_t semaforo_state = STATE_SEM_NORMAL;

void stateSemaforo(char rxCharacter) {

	switch (rxCharacter) {
	case BLOQUEO_AMB:
		if (semaforo_state == STATE_SEM_NORMAL) {
			semaforo_state = STATE_SEM_NORMAL_BLOQUEO_AMB_NORMAL; // Set funcionamiento normal bloqueo ambulancia
			activarFagSemaforo(); // Activar flag evento sirena
		}
		break;
	case PIR_AMB:
		if (semaforo_state == STATE_SEM_NORMAL_BLOQUEO_AMB_NORMAL) {
			semaforo_state = STATE_SEM_BLOQUEO_AMB; // Set  bloqueo ambulancia
			//activarFagSemaforo();
			suspenderTarea(); // Suspender tarea SemNormalTask
			runSemaforoBloqueoAmbulancia(); // Ejecutar un ciclo de bloqueo ambulancia
		}
		break;
	case BLOQUEO_TOTAL:
		if (semaforo_state == STATE_SEM_NORMAL) {
			semaforo_state = STATE_SEM_BLOQUEO_TOTAL; // Set bloqueo total
			suspenderTarea(); // Suspender tarea SemNormalTask
			runSemaforoBloqueoTotal(); // Ejecutar un ciclo de bloqueo total
		}
		break;
	case DESBLOQUEO:
		if ((semaforo_state == STATE_SEM_BLOQUEO_AMB)
				|| (semaforo_state == STATE_SEM_BLOQUEO_TOTAL)) {
			semaforo_state = STATE_SEM_NORMAL; // Set funcionamiento normal
			desactivarFagSemaforo(); // Desactivar flag evento sirena
			activarTarea(); // Activar tarea SemNormalTask

		}
		break;

	}

}


//Ciclo funcionamiento normal maquina
void runSemaforoNormal() {
	setVerde();
	osDelay(T_VERDE);
	setAmbar();
	osDelay(T_AMBAR);
	setRojo();
	osDelay(T_ROJO);
}

//Ciclo funcionamiento bloqueo ambulancia
void runSemaforoBloqueoAmbulancia() {
	setRojo();
	osDelay(T_AMBULANCIA);
	sendUARTMensaje(BLOQUEO_ACK);
}

//Ciclo funcionamiento bloqueo total
void runSemaforoBloqueoTotal() {
	setRojo();
	osDelay(T_TOTAL);
	sendUARTMensaje(BLOQUEO_ACK);
}


// Semaforo: estado rojo
void setRojo() {
	HAL_GPIO_WritePin(REG_SEM_R, PIN_SEM_R, GPIO_PIN_SET);	    // Rojo
	HAL_GPIO_WritePin(REG_SEM_Y, PIN_SEM_Y, GPIO_PIN_RESET);	// Ambar
	HAL_GPIO_WritePin(REG_SEM_G, PIN_SEM_G, GPIO_PIN_RESET);	// Verde
	sendUARTMensaje(SEM_ROJO);
}

// Semaforo: estado amarillo
void setAmbar() {
	HAL_GPIO_WritePin(REG_SEM_R, PIN_SEM_R, GPIO_PIN_RESET);	// Rojo
	HAL_GPIO_WritePin(REG_SEM_Y, PIN_SEM_Y, GPIO_PIN_SET);		// Ambar
	HAL_GPIO_WritePin(REG_SEM_G, PIN_SEM_G, GPIO_PIN_RESET);	// Verde
	sendUARTMensaje(SEM_AMBAR);
}

// Semaforo: estado verde
void setVerde() {
	HAL_GPIO_WritePin(REG_SEM_R, PIN_SEM_R, GPIO_PIN_RESET);	// Rojo
	HAL_GPIO_WritePin(REG_SEM_Y, PIN_SEM_Y, GPIO_PIN_RESET);	// Ambar
	HAL_GPIO_WritePin(REG_SEM_G, PIN_SEM_G, GPIO_PIN_SET);		// Verde
	sendUARTMensaje(SEM_VERDE);
}


/*******************************
 * 			SIRENA
********************************/

void runSirena() {
	if ((HAL_GPIO_ReadPin(REG_AMB_R, PIN_AMB_R) == GPIO_PIN_RESET)
			&& (HAL_GPIO_ReadPin(REG_AMB_B, PIN_AMB_B) == GPIO_PIN_RESET)) {
		HAL_GPIO_WritePin(REG_AMB_R, PIN_AMB_R, GPIO_PIN_SET); //Sirena ambulancia Rojo
	} else {
		HAL_GPIO_TogglePin(REG_AMB_R, PIN_AMB_R); //Sirena ambulancia  Rojo
		HAL_GPIO_TogglePin(REG_AMB_B, PIN_AMB_B); //Sirena ambulancia  Azul
	}
	osDelay(T_ONOFF);
}

void stopSirena() {
	HAL_GPIO_WritePin(REG_AMB_R, PIN_AMB_R, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(REG_AMB_B, PIN_AMB_B, GPIO_PIN_RESET);
}


