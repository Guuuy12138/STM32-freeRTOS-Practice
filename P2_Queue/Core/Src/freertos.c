/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include"stdio.h"
#include"string.h"
#include"usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for buttonTask */
osThreadId_t buttonTaskHandle;
const osThreadAttr_t buttonTask_attributes = {
  .name = "buttonTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DataTask */
osThreadId_t DataTaskHandle;
const osThreadAttr_t DataTask_attributes = {
  .name = "DataTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for bntQueue */
osMessageQueueId_t bntQueueHandle;
const osMessageQueueAttr_t bntQueue_attributes = {
  .name = "bntQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartbuttonTask(void *argument);
void StartDataTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of bntQueue */
  bntQueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &bntQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of buttonTask */
  buttonTaskHandle = osThreadNew(StartbuttonTask, NULL, &buttonTask_attributes);

  /* creation of DataTask */
  DataTaskHandle = osThreadNew(StartDataTask, NULL, &DataTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartbuttonTask */
/**
  * @brief  Function implementing the buttonTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartbuttonTask */
void StartbuttonTask(void *argument)
{
  /* USER CODE BEGIN StartbuttonTask */
  uint32_t bntCount = 0;
  /* Infinite loop */
  for(;;)
  {
    if(HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_RESET) {
      osDelay(10);
      if (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_RESET) {
        bntCount++;
        osMessageQueuePut(bntQueueHandle, &bntCount, 0, osWaitForever);
      }
      while (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_RESET) {
        osDelay(10);
      }
    }else {
      osDelay(10);
    }
  }
  /* USER CODE END StartbuttonTask */
}

/* USER CODE BEGIN Header_StartDataTask */
/**
* @brief Function implementing the DataTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDataTask */
void StartDataTask(void *argument)
{
  /* USER CODE BEGIN StartDataTask */
  uint32_t dataCount = 0;
  char msg[50];
  /* Infinite loop */
  for(;;)
  {
    uint32_t btncout = 0;
    osMessageQueueGet(bntQueueHandle, &btncout, NULL, osWaitForever);
    dataCount++;
    osDelay(1000);
    sprintf(msg,"按键次数：%d 数据处理：%d",(int)btncout,(int)dataCount);
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg),1000);
  }
  /* USER CODE END StartDataTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

