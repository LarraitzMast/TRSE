/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
// Protocolo de comunicacion
#define BLOQUEO_AMB 	    'E'	// Bloqueo ambulancia
#define PIR_AMB 	    	'P'	// Deteccion ambulancioa
#define BLOQUEO_TOTAL 	    'F' // Bloqueo total
#define BLOQUEO_ACK 	    'K' // Confirmacion bloqueo
#define DESBLOQUEO  	    'H' // Desbloqueo
#define SEM_ROJO 		    'R' // Semaforo rojo encendido
#define SEM_AMBAR 		    'Y' // Semaforo ambar encendido
#define SEM_VERDE  		    'G' // Semaforo verde encendido
#define SIRENA_START 	    'I' // Sirena encender
#define SIRENA_OFF  	    'J' // Sirena apagado
#define ASK_TO_EMERGENCY    '?'

#define FLAG_EVENTO_SIRENA    1

void suspenderTarea();
void activarTarea();
void msgKey(char msg);
void activarFagSemaforo();
void desactivarFagSemaforo();

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
