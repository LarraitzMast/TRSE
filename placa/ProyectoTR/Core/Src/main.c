/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "semaforo.h"
#include "keyboard.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

uint8_t rx_buffer;
uint8_t is_sirena;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Definitions for sendUART */
osThreadId_t sendUARTHandle;
const osThreadAttr_t sendUART_attributes = {
  .name = "sendUART",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for semNormalTask */
osThreadId_t semNormalTaskHandle;
const osThreadAttr_t semNormalTask_attributes = {
  .name = "semNormalTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for semGestionTask */
osThreadId_t semGestionTaskHandle;
const osThreadAttr_t semGestionTask_attributes = {
  .name = "semGestionTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sirenaTask */
osThreadId_t sirenaTaskHandle;
const osThreadAttr_t sirenaTask_attributes = {
  .name = "sirenaTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for keyboardTask */
osThreadId_t keyboardTaskHandle;
const osThreadAttr_t keyboardTask_attributes = {
  .name = "keyboardTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for keys */
osMessageQueueId_t keysHandle;
const osMessageQueueAttr_t keys_attributes = {
  .name = "keys"
};
/* Definitions for semaforo */
osMessageQueueId_t semaforoHandle;
const osMessageQueueAttr_t semaforo_attributes = {
  .name = "semaforo"
};
/* Definitions for uartMutex */
osMutexId_t uartMutexHandle;
const osMutexAttr_t uartMutex_attributes = {
  .name = "uartMutex"
};
/* Definitions for semEvent */
osEventFlagsId_t semEventHandle;
const osEventFlagsAttr_t semEvent_attributes = {
  .name = "semEvent"
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void StartSendUART(void *argument);
void StartSemNormalTask(void *argument);
void StartSemGestionTask(void *argument);
void StartSirenaTask(void *argument);
void StartKeyboardTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* Envio mensaje UART ---------------------------------------------------------*/
//El dato enviado es un caracter. Funcion utilizada por todas las tareas por lo
//que el uso de recurso comun esta protegido por mutex

void sendUARTMensaje(uint8_t msg) {
	osMutexAcquire(uartMutexHandle, osWaitForever);
	HAL_UART_Transmit(&huart2, &msg, sizeof(msg), 50);
	osMutexRelease(uartMutexHandle);
}

/* Recepcion mensaje UART ---------------------------------------------------------*/
//Interrupcion UART. El caracter recibido puede estar relacinado
//con la gestion del keyboard o semaforo. Los caracteres recibidos
//se almacenan en las colas keysHandle y semaforoHandle

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	osMessageQueuePut(semaforoHandle, &rx_buffer, 0, 0);
	osMessageQueuePut(keysHandle, &rx_buffer, 0, 0);
	HAL_UART_Receive_IT(&huart2, &rx_buffer, sizeof(rx_buffer));
}

/* Interrupicon EXIT ---------------------------------------------------------*/
//Interrupcion por teclado o PIR :
//		* Interrupcion del teclado por linea. La obtencion del key
//		se realiza en el mismo teclado el valor se almacena en la cola keysHandle
//      * Interrupcion PIR: el valor de combio de estado se almacena la cola semaforoHandle
//
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	uint8_t row = 0;
	char msg = PIR_AMB;

	switch (GPIO_Pin) {

	case PIN_KEY_ROW_0:
		if (delayKey())
			return;
		row = 0;
		handleKeyBoard(row);
		break;
	case PIN_KEY_ROW_1:
		if (delayKey())
			return;
		row = 1;
		handleKeyBoard(row);
		break;
	case PIN_KEY_ROW_2:
		if (delayKey())
			return;
		row = 2;
		handleKeyBoard(row);
		break;
	case PIN_KEY_ROW_3:
		if (delayKey())
			return;
		row = 3;
		handleKeyBoard(row);
		break;

	case PIN_PIR:
		osMessageQueuePut(semaforoHandle, &msg, 0, 0);
		break;
	}
}

/* Gestion recursos desde semaforo.c y keyboard.c ---------------------------------------------------------*/
//Escritura caracter cola  keysHandle
void msgKey(char msg) {
	osMessageQueuePut(keysHandle, &msg, 0, 0);
}

//Activar Sirena
void activarFagSemaforo() {
	osEventFlagsSet(semEventHandle, FLAG_EVENTO_SIRENA);
}

//Desactivar Sirena
void desactivarFagSemaforo() {
	is_sirena = 0;
	osEventFlagsClear(semEventHandle, FLAG_EVENTO_SIRENA);
}

//Suspender tarea semNormalTask
void suspenderTarea() {
	osThreadSuspend(semNormalTaskHandle); // Boqueo tarea semNormalTask
}

//Activar tarea semNormalTask
void activarTarea() {
	osThreadResume(semNormalTaskHandle); // Reanudar tarea semNormalTask
}


/**
 * Crea los objetos del sistema operativo.
 */
void createOsObjects() {
	uartMutexHandle = osMutexNew(&uartMutex_attributes);

	keysHandle = osMessageQueueNew(4, sizeof(char), &keys_attributes);
	semaforoHandle = osMessageQueueNew(4, sizeof(char), &semaforo_attributes);

	semEventHandle = osEventFlagsNew(&semEvent_attributes);
}

/**
 * Crea las diferentes tareas
 */
void createOsThreads() {

	semNormalTaskHandle = osThreadNew(StartSemNormalTask, NULL,
			&semNormalTask_attributes);

	semGestionTaskHandle = osThreadNew(StartSemGestionTask, NULL,
			&semGestionTask_attributes);

	sirenaTaskHandle = osThreadNew(StartSirenaTask, NULL,
			&sirenaTask_attributes);

	keyboardTaskHandle = osThreadNew(StartKeyboardTask, NULL,
			&keyboardTask_attributes);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

	// Pone a 1 los pines de columnas del teclado
	for (int i = 0; i < 4; i++)
		writeColum(i, GPIO_PIN_SET);

	// Prepara el UART para recibir un caracter
	HAL_UART_Receive_IT(&huart2, (uint8_t*) &rx_buffer, sizeof(rx_buffer));

	// Inicializa recursos
	createOsObjects();
	createOsThreads();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/**
* @}
*/
/**
* @}
*/

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_10 | GPIO_PIN_3,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : PC0 PC1 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PA5 PA8 PA9 PA10 */
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PA6 PA7 */
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PC5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB10 PB3 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_10 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : PC8 PC9 */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}


/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartSendUART */
/**
 * @brief  Function implementing the sendUART thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartSendUART */
void StartSendUART(void *argument) {
	/*char rxKey = 0;
	 for (;;) {
	 // IMPORTANTE Solo para uso de test
	 // BORRAR
	 osMessageQueueGet(keysHandle, &rxKey, NULL, osWaitForever);
	 //HAL_UART_Transmit(&huart2, &rxKey, sizeof(char), 50);
	 }*/
}

/* USER CODE BEGIN Header_StartSemNormalTask */
/**
 * Ejecucion funcion normal del semaforo.
 * Tarea que se suspende y se activa.
 *
/* USER CODE END Header_StartSemNormalTask */
void StartSemNormalTask(void *argument) {
	for (;;) {
		runSemaforoNormal();
	}
}

/* USER CODE BEGIN Header_StartSemGestionTask */
/**
 * Gestion estado del semaforo.
*/
/* USER CODE END Header_StartSemGestionTask */
void StartSemGestionTask(void *argument) {
	char rxChar;
	for (;;) {
		osMessageQueueGet(semaforoHandle, &rxChar, NULL, osWaitForever);
		stateSemaforo(rxChar);
	}
}

/* USER CODE BEGIN Header_StartSirenaTask */
/**
 * Gestion estado de la sirena. Posee funciones que
 * se activa o se desactivan desde la tarea semGestionTask con
 * el flag del evento semEventHandle.
 *
 */
/* USER CODE END Header_StartSirenaTask */
void StartSirenaTask(void *argument) {
	desactivarFagSemaforo();
	for (;;) {
		osEventFlagsWait(semEventHandle, FLAG_EVENTO_SIRENA, osFlagsWaitAny, osWaitForever);
		is_sirena = 1;
		sendUARTMensaje(SIRENA_START);
		while (is_sirena) {
			runSirena();
		}
		sendUARTMensaje(SIRENA_OFF);
		stopSirena();


	}
}

/* USER CODE BEGIN Header_StartKeyboardTask */
/**
 * Gestion estado del teclado.
 */
/* USER CODE END Header_StartKeyboardTask */
void StartKeyboardTask(void *argument) {
	char rxChar;

	for (;;) {
		osMessageQueueGet(keysHandle, &rxChar, NULL, osWaitForever);
		stateKeyBoard(rxChar);

	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
