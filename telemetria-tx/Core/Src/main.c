/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "spi.h"
#include "subghz.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "radio_driver.h"
#include "stm32wlxx_nucleo_radio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MCP2515_CS_PIN GPIO_PIN_2
#define MCP2515_CS_PORT GPIOB
#define MCP2515_INT_PIN GPIO_PIN_7
#define MCP2515_INT_PORT GPIOB

#define MCP_READ 0x03
#define MCP_READSTATUS 0xA0
#define MCP_CANINTF 0x2C
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile bool tx_done, tx_timeout;
volatile uint8_t canMessageReceived = 0;
uint8_t lora_buff[STD_BUFFER_SIZE];
uint32_t tx_counter;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

int __io_putchar(int ch);
int _write(int file, char *ptr, int len);
void RadioOnDioIrq(RadioIrqMasks_t radioIrq);
void radioInit(void);
void MCP2515_CS_LOW(void);
void MCP2515_CS_HIGH(void);
void MCP2515_WriteRegister(uint8_t reg, uint8_t value);
uint8_t MCP2515_ReadRegister(uint8_t reg);
uint8_t MCP2515_ReadStatus(void);
void MCP2515_Reset(void);
uint8_t MCP2515_Init(void);
uint8_t MCP2515_CheckReceive(void);
void MCP2515_ReadMessage(uint32_t *id, uint8_t *len, uint8_t *data);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void MCP2515_CS_LOW(void) {
  HAL_GPIO_WritePin(MCP2515_CS_PORT, MCP2515_CS_PIN, GPIO_PIN_RESET);
}

void MCP2515_CS_HIGH(void) {
  HAL_GPIO_WritePin(MCP2515_CS_PORT, MCP2515_CS_PIN, GPIO_PIN_SET);
}

void MCP2515_WriteRegister(uint8_t reg, uint8_t value) {
  uint8_t data[3] = {0x02, reg, value};
  MCP2515_CS_LOW();
  HAL_SPI_Transmit(&hspi1, data, 3, 100);
  MCP2515_CS_HIGH();
}

uint8_t MCP2515_ReadRegister(uint8_t reg) {
  uint8_t cmd[2] = {MCP_READ, reg};
  uint8_t value = 0;
  MCP2515_CS_LOW();
  HAL_SPI_Transmit(&hspi1, cmd, 2, 100);
  HAL_SPI_Receive(&hspi1, &value, 1, 100);
  MCP2515_CS_HIGH();
  return value;
}

uint8_t MCP2515_ReadStatus(void) {
  uint8_t cmd = MCP_READSTATUS;
  uint8_t status = 0;
  MCP2515_CS_LOW();
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
  HAL_SPI_Receive(&hspi1, &status, 1, 100);
  MCP2515_CS_HIGH();
  return status;
}

void MCP2515_Reset(void) {
  uint8_t cmd = 0xC0;
  MCP2515_CS_LOW();
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
  MCP2515_CS_HIGH();
  HAL_Delay(10);
}

uint8_t MCP2515_Init(void) {
  BSP_LED_On(LED_BLUE);

  MCP2515_Reset();
  HAL_Delay(100);

  MCP2515_WriteRegister(0x0F, 0x80);
  HAL_Delay(10);

  uint8_t mode = MCP2515_ReadRegister(0x0E);

  if ((mode & 0xE0) != 0x80) {
    BSP_LED_Off(LED_BLUE);
    BSP_LED_On(LED_RED);
    return 0;
  }

  MCP2515_WriteRegister(0x2A, 0x00);
  MCP2515_WriteRegister(0x29, 0x90);
  MCP2515_WriteRegister(0x28, 0x02);

  MCP2515_WriteRegister(0x60, 0x60);
  MCP2515_WriteRegister(0x2B, 0x01);

  MCP2515_WriteRegister(0x0F, 0x00);
  HAL_Delay(10);

  mode = MCP2515_ReadRegister(0x0E);

  if ((mode & 0xE0) != 0x00) {
    BSP_LED_Off(LED_BLUE);
    BSP_LED_On(LED_RED);
    return 0;
  }

  BSP_LED_Off(LED_BLUE);
  BSP_LED_On(LED_GREEN);
  HAL_Delay(500);
  BSP_LED_Off(LED_GREEN);

  return 1;
}

uint8_t MCP2515_CheckReceive(void) {
  uint8_t status = MCP2515_ReadStatus();
  if (status & 0x01) {
    uint8_t intf = MCP2515_ReadRegister(MCP_CANINTF);
    return (intf & 0x01);
  }
  return 0;
}

void MCP2515_ReadMessage(uint32_t *id, uint8_t *len, uint8_t *data) {
  uint8_t sidh = MCP2515_ReadRegister(0x61);
  uint8_t sidl = MCP2515_ReadRegister(0x62);
  uint8_t dlc = MCP2515_ReadRegister(0x65);

  *id = (sidh << 3) | (sidl >> 5);
  *len = dlc & 0x0F;

  if (*len > 8) {
    *len = 8;
  }

  for (int i = 0; i < *len; i++) {
    data[i] = MCP2515_ReadRegister(0x66 + i);
  }

  MCP2515_WriteRegister(MCP_CANINTF, 0x00);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == MCP2515_INT_PIN) {
    canMessageReceived = 1;
  }
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

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DEBUG_SUBGHZSPI;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
  GPIO_InitStruct.Alternate = GPIO_AF13_DEBUG_RF;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Alternate = GPIO_AF6_RF_BUSY;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_8;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SUBGHZ_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  radioInit();

  tx_counter = 0;

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  SUBGRF_SetDioIrqParams(IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                         IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                         IRQ_RADIO_NONE,
                         IRQ_RADIO_NONE);

  printf("\rTELEMETRIA NUCLEOWL TX - FÓRMULA TESLA UFMG 2025\n\r");

  if (!MCP2515_Init()) {
    printf("FALHA: MCP2515 nao inicializou!\n\r");
    while(1) {
      BSP_LED_Toggle(LED_RED);
      HAL_Delay(200);
    }
  }

  printf("MCP2515 inicializado. Aguardando mensagens CAN...\n\r");

  while (1)
  {

    if (canMessageReceived || MCP2515_CheckReceive()) {
      canMessageReceived = 0;

      uint32_t can_id;
      uint8_t can_len;
      uint8_t can_data[8];

      MCP2515_ReadMessage(&can_id, &can_len, can_data);

      if (can_id > 0 && can_len > 0) {

        lora_buff[0] = (uint8_t)(can_id & 0xFF);
        lora_buff[1] = (uint8_t)((can_id >> 8) & 0xFF);
        lora_buff[2] = (uint8_t)((can_id >> 16) & 0xFF);
        lora_buff[3] = (uint8_t)((can_id >> 24) & 0xFF);

        for (int i = 0; i < 8; i++) {
          if (i < can_len) {
            lora_buff[4 + i] = can_data[i];
          } else {
            lora_buff[4 + i] = 0;
          }
        }

        tx_done = tx_timeout = false;
        SUBGRF_SetSwitch(RFO_LP, RFSWITCH_TX);
        SUBGRF_SendPayload(lora_buff, STD_BUFFER_SIZE, 3000 << 6);
        while (!tx_done && !tx_timeout);

        if (tx_done)
        {
          BSP_LED_Off(LED_RED);
          BSP_LED_Toggle(LED_GREEN);
          printf("\r\n\nTX #%lu - CAN ID: 0x%03lX (%lu)\n\r",
                 tx_counter++, can_id, can_id, can_len);
          printf("Data: ");
          int n = sizeof(lora_buff)/sizeof(lora_buff[0]);
          for(int i = 0; i < n; i++){
        	  printf("%d", lora_buff[i]);
        	  if(i < n - 1){
        		  printf(",");
        	  }
          }
        } else {
          BSP_LED_Off(LED_GREEN);
          BSP_LED_Toggle(LED_RED);
          printf("ERRO: Timeout ao transmitir LoRa\n\r");
        }
      }
    }

    //HAL_Delay(10);

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* @brief Redireciona um caracter para escrita no monitor serial, via UART2
 * @param ch Caracter
 * @retval Caracter escrito
 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/* @brief Redireciona um ponteiro para vetor de caracteres
 * @param file Stream de destino
 * @param ptr Ponteiro para o vetor de caracteres
 * @param len Comprimento do vetor de caracteres
 * @retval Tamanho do vetor de caracteres redirecionado
 */
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/**
  * @brief  Initialize the Sub-GHz radio and dependent hardware.
  * @retval None
  */
void radioInit(void)
{
  SUBGRF_Init(RadioOnDioIrq);

  SUBGRF_SetBufferBaseAddress(0x00, 0x00);

  SUBGRF_SetRfFrequency(RF_FREQUENCY);
  SUBGRF_SetRfTxPower(TX_OUTPUT_POWER);

  SUBGRF_SetPacketType(PACKET_TYPE_LORA);

  ModulationParams_t modParams = {
    .PacketType = PACKET_TYPE_LORA,
    .Params.LoRa = {
      .Bandwidth           = LORA_BW_500,
      .CodingRate          = LORA_CR_4_5,
      .SpreadingFactor     = LORA_SF7,
      .LowDatarateOptimize = 0
    }
  };
  SUBGRF_SetModulationParams(&modParams);

  PacketParams_t pktParams = {
    .PacketType = PACKET_TYPE_LORA,
    .Params.LoRa = {
      .CrcMode        = LORA_CRC_ON,
      .HeaderType     = LORA_PACKET_VARIABLE_LENGTH,
      .InvertIQ       = LORA_IQ_NORMAL,
      .PayloadLength  = STD_BUFFER_SIZE,
      .PreambleLength = LORA_PREAMBLE_LENGTH
    }
  };
  SUBGRF_SetPacketParams(&pktParams);

  SUBGRF_SetSyncWord((uint8_t*)LORA_SYNC_WORD);
}

/**
  * @brief  Receive data trough SUBGHZSPI peripheral
  * @param  radioIrq  interrupt pending status information
  * @retval None
  */
void RadioOnDioIrq(RadioIrqMasks_t irq)
{
  if(irq == IRQ_TX_DONE)            tx_done    = true;
  else if(irq == IRQ_RX_TX_TIMEOUT) tx_timeout = true;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
