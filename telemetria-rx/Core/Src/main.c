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
#include "subghz.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE BEGIN PV */

enum PrintMode {PM_INT, PM_UINT, PM_FLOAT};
volatile bool tx_done, tx_timeout, rx_done, rx_error, rx_timeout;
uint8_t lora_buff[STD_BUFFER_SIZE], rx_size, print_mode;
float float_result[8];
int32_t int_result[8], id_buff;
uint32_t uint_result[8];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

int __io_putchar(int ch);
int _write(int file, char *ptr, int len);
void RadioOnDioIrq(RadioIrqMasks_t radioIrq);
void radioInit(void);
uint32_t combine_u8_to_u32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
static inline uint16_t u16(uint8_t high, uint8_t low);
static inline int16_t s16(uint8_t high, uint8_t low);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  // Enable GPIO Clocks
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // DEBUG_SUBGHZSPI_{NSSOUT, SCKOUT, MSIOOUT, MOSIOUT} pins
  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DEBUG_SUBGHZSPI;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // DEBUG_RF_{HSE32RDY, NRESET} pins
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
  GPIO_InitStruct.Alternate = GPIO_AF13_DEBUG_RF;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // DEBUG_RF_{SMPSRDY, LDORDY} pins
  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // RF_BUSY pin
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Alternate = GPIO_AF6_RF_BUSY;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // RF_{IRQ0, IRQ1, IRQ2} pins
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_8;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SUBGHZ_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  radioInit();

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW2, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW3, BUTTON_MODE_EXTI);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  SUBGRF_SetDioIrqParams(IRQ_TX_DONE | IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR,
                         IRQ_TX_DONE | IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR,
                         IRQ_RADIO_NONE,
                         IRQ_RADIO_NONE);

  printf("\rTELEMETRIA NUCLEOWL RX - FÓRMULA TESLA UFMG 2025\n\r");

  while (1)
  {

	rx_done = rx_timeout = rx_error = false;
	SUBGRF_SetSwitch(RFO_LP, RFSWITCH_RX);
	SUBGRF_SetRx(3000 << 6);
	while (!rx_done && !rx_timeout && !rx_error);

	if (rx_done)
	{
		  SUBGRF_GetPayload(lora_buff, &rx_size, 0xFF);

		  uint16_t received_password = ((uint16_t)lora_buff[1] << 8) | lora_buff[0];

		  //printf("0x%X vs. 0x%X\r\n", received_password, PACKET_PASSWORD);

		  if (received_password == PACKET_PASSWORD) {
			  id_buff = u16(lora_buff[2], lora_buff[3]);

			  //uint16_t id_low = ((uint16_t)lora_buff[3] << 8) | lora_buff[2];
			  //uint16_t id_high = ((uint16_t)lora_buff[5] << 8) | lora_buff[4];
			  //id_buff = ((uint32_t)id_high << 16) | id_low;

			  switch (id_buff) {
				case 75:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]);
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]);
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]);
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]);
				  print_mode = PM_FLOAT;
				  break;

				case 76:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]); // SPEED_AVG
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]); // STEERING_WHEEL
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]); // THROTTLE
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]); // BRAKE
				  print_mode = PM_FLOAT;
				  break;

				case 77:
				  memset(uint_result, 0, sizeof(uint_result));
				  uint_result[0] = (uint32_t) u16(lora_buff[4], lora_buff[5]); // MODE
				  uint_result[1] = (uint32_t) u16(lora_buff[6], lora_buff[7]); // TORQUER_GAIN
				  uint_result[2] = (uint32_t) u16(lora_buff[8], lora_buff[9]); // DISTANCE_P_ODOM
				   // DISTANCE_T_ODOM
				  print_mode = PM_UINT;
				  break;

				case 78:
				  memset(uint_result, 0, sizeof(uint_result));
				  uint_result[0] = (uint32_t) u16(lora_buff[4], lora_buff[5]); // CONTROL_EVENT_FLAG_1
				  uint_result[1] = (uint32_t) u16(lora_buff[6], lora_buff[7]); // CONTROL_EVENT_FLAG_2
				  uint_result[2] = (uint32_t) u16(lora_buff[8], lora_buff[9]); // REF_TORQUE_R_MOTOR
				  uint_result[3] = (uint32_t) u16(lora_buff[10], lora_buff[11]); // REF_TORQUE_L_MOTOR
				  print_mode = PM_UINT;
				  break;

				case 79:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]); // SPEED_LF
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]); // SPEED_LR
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]); // SPEED_RL
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]); // SPEED_RR
				  print_mode = PM_FLOAT;
				  break;

				case 80:
				  memset(int_result,  0, sizeof(int_result));
				  int_result[0] = (int32_t) s16(lora_buff[4], lora_buff[5]); // ID_PANEL_DEBUG_1
				  int_result[1] = (int32_t) s16(lora_buff[6], lora_buff[7]); // ID_PANEL_DEBUG_2
				  int_result[2] = (int32_t) s16(lora_buff[8], lora_buff[9]); // ID_PANEL_DEBUG_3
				  int_result[3] = (int32_t) s16(lora_buff[10], lora_buff[11]); // ID_PANEL_DEBUG_4
				  print_mode = PM_INT;
				  break;

				case 81:
				  memset(uint_result, 0, sizeof(uint_result));
				  uint_result[0] = (uint32_t) u16(lora_buff[4], lora_buff[5]); // ID_REGEN_BRAKE_STATE
				  print_mode = PM_UINT;
				  break;

				case 85:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) s16(lora_buff[4], lora_buff[5]); // SPEED_L_MOTOR
				  float_result[1] = (float) s16(lora_buff[6], lora_buff[7]); // TORQUE_L_MOTOR
				  float_result[2] = (float) s16(lora_buff[8], lora_buff[9]); // POWER_L_MOTOR
				  float_result[3] = (float) s16(lora_buff[10], lora_buff[11]); // CURRENT_L_MOTOR
				  print_mode = PM_FLOAT;
				  break;

				case 86:
				  memset(int_result,  0, sizeof(int_result));
				  int_result[0] = (int32_t) s16(lora_buff[4], lora_buff[5]); // ENERGY_L_MOTOR
				  int_result[1] = (int32_t) s16(lora_buff[6], lora_buff[7]); // OVERLOAD_L_MOTOR
				  int_result[2] = (int32_t) s16(lora_buff[8], lora_buff[9]); // TEMPERATURE1_L
				  int_result[3] = (int32_t) s16(lora_buff[10], lora_buff[11]); // TEMPERATURE2_L
				  print_mode = PM_INT;
				  break;

				case 87:
				  memset(uint_result, 0, sizeof(uint_result));
				  uint_result[0] = (uint32_t) u16(lora_buff[4], lora_buff[5]); // ID_LOST_MSG_L_MOTOR
				  uint_result[1] = (uint32_t) u16(lora_buff[6], lora_buff[7]); // ID_BUS_OFF_L_MOTOR
				  uint_result[2] = (uint32_t) u16(lora_buff[8], lora_buff[9]); // ID_CAN_STATE_L_MOTOR
				  print_mode = PM_UINT;
				  break;

				case 88:
				  memset(uint_result, 0, sizeof(uint_result));
				  uint_result[0] = (uint32_t) u16(lora_buff[4], lora_buff[5]); // ID_INV_STATE_L_MOTOR
				  uint_result[1] = (uint32_t) u16(lora_buff[6], lora_buff[7]); // ID_FAILURE_L_MOTOR
				  uint_result[2] = (uint32_t) u16(lora_buff[8], lora_buff[9]); // ID_ALARM_L_MOTOR
				  print_mode = PM_UINT;
				  break;

				case 95:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) s16(lora_buff[4], lora_buff[5]); // ID_SPEED_R_MOTOR
				  float_result[1] = (float) s16(lora_buff[6], lora_buff[7]); // ID_TORQUE_R_MOTOR
				  float_result[2] = (float) s16(lora_buff[8], lora_buff[9]); // ID_POWER_R_MOTOR
				  float_result[3] = (float) s16(lora_buff[10], lora_buff[11]); // ID_CURRENT_R_MOTOR
				  print_mode = PM_FLOAT;
				  break;

				case 96:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]); // ID_ENERGY_R_MOTOR
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]); // ID_OVERLOAD_R_MOTOR
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]); // ID_TEMPERATURE1_R
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]); // ID_TEMPERATURE2_R
				  print_mode = PM_FLOAT;
				  break;

				case 97:
				  memset(uint_result, 0, sizeof(uint_result));
				  uint_result[0] = (uint32_t) u16(lora_buff[4], lora_buff[5]); // ID_LOST_MSG_R_MOTOR
				  uint_result[1] = (uint32_t) u16(lora_buff[6], lora_buff[7]); // ID_BUS_OFF_R_MOTOR
				  uint_result[2] = (uint32_t) u16(lora_buff[8], lora_buff[9]); // ID_CAN_STATE_R_MOTOR
				  print_mode = PM_UINT;
				  break;

				case 98:
				  memset(uint_result, 0, sizeof(uint_result));
				  uint_result[0] = (uint32_t) u16(lora_buff[4], lora_buff[5]); // ID_INV_STATE_R_MOTOR
				  uint_result[1] = (uint32_t) u16(lora_buff[6], lora_buff[7]); // ID_FAILURE_R_MOTOR
				  uint_result[2] = (uint32_t) u16(lora_buff[8], lora_buff[9]); // ID_ALARM_R_MOTOR
				  print_mode = PM_UINT;
				  break;

				case 259:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) s16(lora_buff[4], lora_buff[5]); // AcelX
				  float_result[1] = (float) s16(lora_buff[6], lora_buff[7]); // AcelY
				  float_result[2] = (float) s16(lora_buff[8], lora_buff[9]); // AcelZ
				  print_mode = PM_FLOAT;
				  break;

				case 260:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) s16(lora_buff[4], lora_buff[5]); // GyroX
				  float_result[1] = (float) s16(lora_buff[6], lora_buff[7]); // GyroY
				  float_result[2] = (float) s16(lora_buff[8], lora_buff[9]); // GyroZ
				  print_mode = PM_FLOAT;
				  break;

				case 261:
				  memset(int_result,  0, sizeof(int_result));
				  int_result[0] = (int32_t) s16(lora_buff[4], lora_buff[5]); // Temp
				  print_mode = PM_INT;
				  break;

				case 361:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]); // ID_PANEL_DEBUG_1
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]); // ID_PANEL_DEBUG_2
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]); // ID_PANEL_DEBUG_3
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]); // ID_PANEL_DEBUG_4
				  print_mode = PM_FLOAT;
				  break;

				case 300: // STACK 1
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]) / 10000.0f; // CELL_1
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]) / 10000.0f; // CELL_2
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]) / 10000.0f; // CELL_3
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]) / 10000.0f; // CELL_4
				  print_mode = PM_FLOAT;
				  break;

				case 301: // STACK 2
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]) / 10000.0f; // CELL_1
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]) / 10000.0f; // CELL_2
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]) / 10000.0f; // CELL_3
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]) / 10000.0f; // CELL_4
				  print_mode = PM_FLOAT;
				  break;

				case 302: // STACK 3
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]) / 10000.0f; // CELL_1
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]) / 10000.0f; // CELL_2
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]) / 10000.0f; // CELL_3
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]) / 10000.0f; // CELL_4
				  print_mode = PM_FLOAT;
				  break;

				case 303: // STACK 4
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]) / 10000.0f; // CELL_1
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]) / 10000.0f; // CELL_2
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]) / 10000.0f; // CELL_3
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]) / 10000.0f; // CELL_4
				  print_mode = PM_FLOAT;
				  break;

				case 304: // STACK 5
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]) / 10000.0f; // CELL_1
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]) / 10000.0f; // CELL_2
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]) / 10000.0f; // CELL_3
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]) / 10000.0f; // CELL_4
				  print_mode = PM_FLOAT;
				  break;

				case 305: // STACK 6
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]) / 10000.0f; // CELL_1
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]) / 10000.0f; // CELL_2
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]) / 10000.0f; // CELL_3
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]) / 10000.0f; // CELL_4
				  print_mode = PM_FLOAT;
				  break;

				case 306: // ACCUMULATOR PARAMS
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]) / 10000.0f; // MIN_VOLTAGE
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]) / 10000.0f; // MAX_VOLTAGE
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]) / 10.0f;   // TOTAL_VOLTAGE
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]);         // SHUNT CURRENT
				  print_mode = PM_FLOAT;
				  break;

				case 307: // BMS PARAMS
				  memset(int_result,  0, sizeof(int_result));
				  int_result[0] = (int32_t) s16(lora_buff[4], lora_buff[5]); // BMS_MODE
				  int_result[1] = (int32_t) s16(lora_buff[6], lora_buff[7]); // BMS_ERROR
				  int_result[2] = (int32_t) s16(lora_buff[8], lora_buff[9]); // AIR_P
				  int_result[3] = (int32_t) s16(lora_buff[10], lora_buff[11]); // AIR_N
				  print_mode = PM_INT;
				  break;

				case 308: // ACCUMULATOR PARAMS
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]); // MIN_VOLTAGE
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]); // MAX_VOLTAGE
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]);   // TOTAL_VOLTAGE
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]) / 10000.0f;         // SHUNT CURRENT
				  print_mode = PM_FLOAT;
				  break;

				case 311: // ACCUMULATOR PARAMS
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]); // MIN_VOLTAGE
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]); // MAX_VOLTAGE
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]);   // TOTAL_VOLTAGE
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]) / 10000.0f;         // SHUNT CURRENT
				  print_mode = PM_FLOAT;
				  break;

				case 312: // ACCUMULATOR PARAMS
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]); // MIN_VOLTAGE
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]); // MAX_VOLTAGE
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]);   // TOTAL_VOLTAGE
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]) / 10000.0f;         // SHUNT CURRENT
				  print_mode = PM_FLOAT;
				  break;

				default:
				  memset(float_result, 0, sizeof(float_result));
				  float_result[0] = (float) u16(lora_buff[4], lora_buff[5]); // ID_ENERGY_R_MOTOR
				  float_result[1] = (float) u16(lora_buff[6], lora_buff[7]); // ID_OVERLOAD_R_MOTOR
				  float_result[2] = (float) u16(lora_buff[8], lora_buff[9]); // ID_TEMPERATURE1_R
				  float_result[3] = (float) u16(lora_buff[10], lora_buff[11]); // ID_TEMPERATURE2_R
				  print_mode = PM_FLOAT;
				  break;
				}


				printf("%ld,", id_buff);

				if (print_mode == PM_UINT) {
				  for (uint8_t i = 0; i < 8; i++) {
					printf("%d", uint_result[i]);
					if (i < 7) printf(",");
				  }
				} else if (print_mode == PM_INT) {
				  for (uint8_t i = 0; i < 8; i++) {
					printf("%d", int_result[i]);
					if (i < 7) printf(",");
				  }
				} else { // PM_FLOAT
				  for (uint8_t i = 0; i < 8; i++) {
					printf("%.2f", float_result[i]);
					if (i < 7) printf(",");
				  }
				}
				printf("\n"); // adicionar /r para debug local

		  BSP_LED_Off(LED_RED);
		  BSP_LED_Off(LED_BLUE);
		  BSP_LED_Toggle(LED_GREEN);
		} else {
		  BSP_LED_Off(LED_RED);
		  BSP_LED_Off(LED_GREEN);
		  BSP_LED_Toggle(LED_BLUE);
		}
	} else {
	  BSP_LED_Off(LED_GREEN);
	  BSP_LED_Off(LED_BLUE);
	  BSP_LED_Toggle(LED_RED);
	}

    //HAL_Delay(SLEEP_TIME);

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

/* @brief Redireciona um ponteiro oara vetor de caracteres
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

  //SUBGRF_SetSyncWord((uint8_t*)LORA_SYNC_WORD);
}

/**
  * @brief  Receive data trough SUBGHZSPI peripheral
  * @param  radioIrq  interrupt pending status information
  * @retval None
  */
void RadioOnDioIrq(RadioIrqMasks_t irq)
{
  if(irq == IRQ_TX_DONE)        tx_done    = true;
  else if(irq == IRQ_RX_TX_TIMEOUT) { tx_timeout = true; rx_timeout = true; }
  else if (irq == IRQ_RX_DONE) rx_done = true;
  else if (irq == IRQ_CRC_ERROR) rx_error = true;
}

/* Combina 4 primeiros numeros do pacote em um unico de 32 bits */
uint32_t combine_u8_to_u32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
    uint32_t value = 0;
    value |= ((uint32_t)b0);
    value |= ((uint32_t)b1 << 8);
    value |= ((uint32_t)b2 << 16);
    value |= ((uint32_t)b3 << 24);
    return value;
}

/* converte dois numeros uint8_t em um uint16_t */
static inline uint16_t u16(uint8_t high, uint8_t low) {
    return ((uint16_t)low << 8) | high;
}

/* converte dois numeros uint8_t em um int16_t */
static inline int16_t s16(uint8_t high, uint8_t low) {
    return (int16_t)(((uint16_t)low << 8) | high);
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
