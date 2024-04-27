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
#include "stm32f1xx_hal.h"

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
#define KEYPAD_IN1_Pin GPIO_PIN_2
#define KEYPAD_IN1_GPIO_Port GPIOC
#define KEYPAD_IN2_Pin GPIO_PIN_3
#define KEYPAD_IN2_GPIO_Port GPIOC
#define K1_Pin GPIO_PIN_0
#define K1_GPIO_Port GPIOA
#define K1_EXTI_IRQn EXTI0_IRQn
#define KEYPAD_IN3_Pin GPIO_PIN_4
#define KEYPAD_IN3_GPIO_Port GPIOC
#define KEYPAD_IN4_Pin GPIO_PIN_5
#define KEYPAD_IN4_GPIO_Port GPIOC
#define KEYPAD_OUT3_Pin GPIO_PIN_10
#define KEYPAD_OUT3_GPIO_Port GPIOB
#define KEYPAD_OUT3_EXTI_IRQn EXTI15_10_IRQn
#define KEYPAD_OUT4_Pin GPIO_PIN_11
#define KEYPAD_OUT4_GPIO_Port GPIOB
#define KEYPAD_OUT4_EXTI_IRQn EXTI15_10_IRQn
#define ROTEN_A_Pin GPIO_PIN_12
#define ROTEN_A_GPIO_Port GPIOB
#define ROTEN_A_EXTI_IRQn EXTI15_10_IRQn
#define ROTEN_B_Pin GPIO_PIN_13
#define ROTEN_B_GPIO_Port GPIOB
#define LCD_BL_Pin GPIO_PIN_12
#define LCD_BL_GPIO_Port GPIOD
#define KEYPAD_OUT1_Pin GPIO_PIN_8
#define KEYPAD_OUT1_GPIO_Port GPIOB
#define KEYPAD_OUT1_EXTI_IRQn EXTI9_5_IRQn
#define KEYPAD_OUT2_Pin GPIO_PIN_9
#define KEYPAD_OUT2_GPIO_Port GPIOB
#define KEYPAD_OUT2_EXTI_IRQn EXTI9_5_IRQn
#define LCD_RST_Pin GPIO_PIN_1
#define LCD_RST_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
