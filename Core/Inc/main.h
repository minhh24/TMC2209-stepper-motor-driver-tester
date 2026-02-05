/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#define led_Pin GPIO_PIN_3
#define led_GPIO_Port GPIOA
#define button_Pin GPIO_PIN_10
#define button_GPIO_Port GPIOB
#define GPIO_output_TMC_DIR_Pin GPIO_PIN_14
#define GPIO_output_TMC_DIR_GPIO_Port GPIOB
#define TIM1_CH3N_step_tmc_Pin GPIO_PIN_15
#define TIM1_CH3N_step_tmc_GPIO_Port GPIOB
#define USART1_TX_tmc_Pin GPIO_PIN_9
#define USART1_TX_tmc_GPIO_Port GPIOA
#define USART1_RX_tmc_Pin GPIO_PIN_10
#define USART1_RX_tmc_GPIO_Port GPIOA
#define GPIO_output_TMC_MS2_Pin GPIO_PIN_11
#define GPIO_output_TMC_MS2_GPIO_Port GPIOA
#define GPIO_output_TMC_MS1_Pin GPIO_PIN_12
#define GPIO_output_TMC_MS1_GPIO_Port GPIOA
#define GPIO_output_TMC_EN_Pin GPIO_PIN_15
#define GPIO_output_TMC_EN_GPIO_Port GPIOA
#define btn_encoder_Pin GPIO_PIN_5
#define btn_encoder_GPIO_Port GPIOB
#define btn_encoder_EXTI_IRQn EXTI9_5_IRQn
#define TIM4_CH1_encoder_Pin GPIO_PIN_6
#define TIM4_CH1_encoder_GPIO_Port GPIOB
#define TIM4_CH2_encoder_Pin GPIO_PIN_7
#define TIM4_CH2_encoder_GPIO_Port GPIOB
#define lcd_Pin GPIO_PIN_8
#define lcd_GPIO_Port GPIOB
#define lcdB9_Pin GPIO_PIN_9
#define lcdB9_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
