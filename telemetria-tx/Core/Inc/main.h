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
#include "stm32wlxx_hal.h"

#include "stm32wlxx_nucleo.h"

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
#define RCC_OSC32_IN_Pin GPIO_PIN_14
#define RCC_OSC32_IN_GPIO_Port GPIOC
#define RCC_OSC32_OUT_Pin GPIO_PIN_15
#define RCC_OSC32_OUT_GPIO_Port GPIOC
#define FE_CTRL3_Pin GPIO_PIN_3
#define FE_CTRL3_GPIO_Port GPIOC
#define FE_CTRL2_Pin GPIO_PIN_5
#define FE_CTRL2_GPIO_Port GPIOC
#define FE_CTRL1_Pin GPIO_PIN_4
#define FE_CTRL1_GPIO_Port GPIOC
#define T_VCP_RX_Pin GPIO_PIN_3
#define T_VCP_RX_GPIO_Port GPIOA
#define T_VCP_RXA2_Pin GPIO_PIN_2
#define T_VCP_RXA2_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

#define RF_FREQUENCY                                915000000 /* Hz */
#define TX_OUTPUT_POWER                             14        /* dBm */
#define LORA_BANDWIDTH                              0         /* Hz */
#define LORA_SPREADING_FACTOR                       7
#define LORA_CODINGRATE                             1
#define LORA_PREAMBLE_LENGTH                        8         /* Igual para Tx e Rx */
#define LORA_SYMBOL_TIMEOUT                         5         /* Symbols */
#define LORA_SYNC_WORD 0xABDC                                 /* ID da rede */
#define STD_BUFFER_SIZE 12

#define TX_INTERVAL_MS 500

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
