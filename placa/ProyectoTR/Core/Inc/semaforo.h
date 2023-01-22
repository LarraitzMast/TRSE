/**
  ******************************************************************************
  * @file           : semaforo.h
  * @brief          : Header for semaforo.c file.
  *                   Fichero que contine distintas definiciones para la gestion del
  *                   semaforo.
  ******************************************************************************/
 #ifndef __SEMAFORO_H__
 #define __SEMAFORO_H__

#ifdef __cplusplus
 extern "C" {
#endif


#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "main.h"


// Tiempo para retardo
#define T_VERDE 		1500 	// 15000
#define T_AMBAR 		500 	// 5000
#define T_ROJO 			1500 	// 15000
#define T_AMBULANCIA 	3000 	// 30000
#define T_TOTAL 		4000 	// 40000
#define T_ONOFF 		500 	// 500

// Estado semaforo
#define STATE_SEM_NORMAL 					1
#define STATE_SEM_NORMAL_BLOQUEO_AMB_NORMAL 2
#define STATE_SEM_BLOQUEO_AMB 				3
#define STATE_SEM_BLOQUEO_TOTAL 			4


// Evento semaforo
#define EVENTO_SIRENA 			1

// Definición de pines
#define REG_SEM_R		GPIOB
#define PIN_SEM_R		GPIO_PIN_10
#define REG_SEM_Y		GPIOA
#define PIN_SEM_Y		GPIO_PIN_8
#define REG_SEM_G		GPIOA
#define PIN_SEM_G		GPIO_PIN_9

#define REG_AMB_R		GPIOA
#define PIN_AMB_R		GPIO_PIN_5
#define REG_AMB_B		GPIOB
#define PIN_AMB_B		GPIO_PIN_3

#define REG_PIR			GPIOC
#define PIN_PIR			GPIO_PIN_5



//Declaracion de funciones
void setRojo();
void setAmbar();
void setVerde();
void runSemaforoNormal();
void runSemaforoBloqueoAmbulancia();
void runSemaforoBloqueoTotal();
void runSirena();
void stopSirena();
void stateSemaforo(char rxCharacter);

#ifdef __cplusplus
}
#endif
#endif /* _SEMAFORO_H */
