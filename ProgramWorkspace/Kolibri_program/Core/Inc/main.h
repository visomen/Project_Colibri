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
#include "stm32h7xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define VCC1_Pin GPIO_PIN_11
#define VCC1_GPIO_Port GPIOE
#define GND1_Pin GPIO_PIN_14
#define GND1_GPIO_Port GPIOE
#define LPn2_Pin GPIO_PIN_12
#define LPn2_GPIO_Port GPIOB
#define INT2_Pin GPIO_PIN_13
#define INT2_GPIO_Port GPIOB
#define LPn4_Pin GPIO_PIN_14
#define LPn4_GPIO_Port GPIOB
#define INT4_Pin GPIO_PIN_15
#define INT4_GPIO_Port GPIOB
#define LPn3_Pin GPIO_PIN_8
#define LPn3_GPIO_Port GPIOD
#define INT3_Pin GPIO_PIN_9
#define INT3_GPIO_Port GPIOD
#define M0_Pin GPIO_PIN_14
#define M0_GPIO_Port GPIOD
#define M1_Pin GPIO_PIN_15
#define M1_GPIO_Port GPIOD
#define STEP_Pin GPIO_PIN_6
#define STEP_GPIO_Port GPIOC
#define DIR_Pin GPIO_PIN_7
#define DIR_GPIO_Port GPIOC
#define LPn5_Pin GPIO_PIN_9
#define LPn5_GPIO_Port GPIOA
#define INT5_Pin GPIO_PIN_10
#define INT5_GPIO_Port GPIOA
#define TRIG2_Pin GPIO_PIN_12
#define TRIG2_GPIO_Port GPIOC
#define LPn1_Pin GPIO_PIN_0
#define LPn1_GPIO_Port GPIOE
#define INT1_Pin GPIO_PIN_1
#define INT1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
