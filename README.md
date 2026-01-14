# Telemetria LoRa STM32WL

Sistema de telemetria baseado em comunicação LoRa para transmissão de dados, utilizando placas NUCLEO-WL55JC1 da STMicroelectronics. O código é compatível com microcontroladores da família STM-WL, porém é possível que necessite de eventuais ajustes na pinagem e outros parâmetros a depender da versão.

---

## Requisitos

- STM32CubeIDE >= 1.13.0
- Placa NUCLEO-WL55JC1
- Cabo USB tipo C
- Módulo MCP2515 (interface CAN)

---

## Configuração do Ambiente

### 1. Instalação dos Embedded Software Packages

1. Acesse: Help > Manage Embedded Software Packages
2. Selecione STM32WL Series
3. Instale a versão V1.3.1 ou superior
4. Reinicie o STM32CubeIDE

Os pacotes são instalados normalmente em:
    ~/STM32Cube/Repository/

---

### 2. Criação do Projeto

1. File > New > STM32 Project
2. Board Selector: NUCLEO-WL55JC1
3. Configurações:
   - Language: C
   - Binary Type: Executable
   - Project Type: STM32Cube
   - Multi-CPU: desabilitado
4. Em Board Project Options, habilite apenas os LEDs LD1, LD2 e LD3

---

### 3. Configuração dos Periféricos (.ioc)

#### SUBGHZ (LoRa)

- Connectivity > SUBGHZ
  - Mode: Activated
- NVIC
  - SUBGHZ Radio Interrupt habilitado

#### USART2 (Debug)

- Connectivity > USART2
  - Mode: Asynchronous

#### SPI1 (MCP2515)

- Connectivity > SPI1
  - Mode: Full-Duplex Master
  - Hardware NSS: Disable

Parâmetros:
- Frame: Motorola
- Data Size: 8 bits
- First Bit: MSB
- Prescaler: 32 (ou 16)
- CPOL: Low
- CPHA: 1 Edge
- CRC: Disabled
- NSS: Software

#### GPIO MCP2515

- CS
  - Pino: PB2
  - Output Push-Pull
  - Nível: High
  - Label: MCP2515_CS

- INT
  - Pino: PB7
  - EXTI Falling Edge
  - Pull-up
  - Label: MCP2515_INT

- NVIC
  - Habilitar EXTI line[9:5]

---

### 4. Geração de Código

- Project Manager > Code Generator
  - Generate peripheral initialization as .c/.h files
- Project > Generate Code

---

### 5. Drivers BSP

Importar de:
    ~/STM32Cube/Repository/STM32Cube_FW_WL_V1.3.1/Drivers/BSP/STM32WLxx_Nucleo/

Ações:
- Importar arquivos .c e .h
- Renomear:
      stm32wlxx_nucleo_conf_template.h -> stm32wlxx_nucleo_conf.h
- Adicionar a pasta ao include path (Debug e Release)

---

### 6. Utilities

Criar pasta Utils e importar:
    Utilities/conf/utilities_conf_template.h
    Utilities/misc/stm32_mem.c
    Utilities/misc/stm32_mem.h

Renomear:
    utilities_conf_template.h -> utilities_conf.h

Adicionar ao include path:
    Utilities/conf
    Utilities/misc

---

### 7. Drivers RF (LoRa)

Criar pasta Drivers/Radio e importar:

    Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.c
    Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.h
    Middlewares/Third_Party/SubGHz_Phy/Conf/radio_conf_template.h
    Projects/NUCLEO-WL55JC/Applications/SubGHz_Phy/SubGHz_Phy_PingPong/SubGHz_Phy/Target/radio_board_if.c
    Projects/NUCLEO-WL55JC/Applications/SubGHz_Phy/SubGHz_Phy_PingPong/SubGHz_Phy/Target/radio_board_if.h

Renomear:
    radio_conf_template.h -> radio_conf.h

Editar arquivos:

Em radio_conf.h:
    // #include "mw_log_conf.h"
    // #include "utilities_def.h"
    // #include "sys_debug.h"

Em radio_driver.c:
    // #include "mw_log_conf.h"

Copiar platform.h de:
    Projects/NUCLEO-WL55JC/Applications/SubGHz_Phy/SubGHz_Phy_PingPong/Common/Inc/

Para:
    Core/Inc/

Em platform.h, comentar:
    // #include "stm32wlxx_ll_gpio.h"

Adicionar Drivers/Radio ao include path.

---

### 8. Importação do Código do Projeto

- Copiar de <b>telemetria-tx</b> ou <b>telemetria-rx</b>:
  - Core/Inc
  - Core/Src
  - Core/Startup
- Sobrescrever os arquivos existentes

---

## Conexões MCP2515

    MCP2515      NUCLEO-WL55JC1
    VCC      ->  3.3V / 5V
    GND      ->  GND
    SCK      ->  PA5
    MISO     ->  PA6
    MOSI     ->  PA7
    CS       ->  PB2
    INT      ->  PB7

- GND comum obrigatório
- Resistores de 120 ohm nas extremidades do barramento CAN

---

## Parâmetros de Configuração

### LoRa

    #define RF_FREQUENCY          868000000U
    #define TX_OUTPUT_POWER       14
    #define LORA_BANDWIDTH        0
    #define LORA_SPREADING_FACTOR 7
    #define LORA_CODINGRATE       1
    #define LORA_PREAMBLE_LENGTH  8

### MCP2515 – 8 MHz / 500 kbps

    #define MCP_CNF1  0x00
    #define MCP_CNF2  0x90
    #define MCP_CNF3  0x02

Para cristal de 16 MHz:

    #define MCP_CNF1  0x01
    #define MCP_CNF2  0xB1
    #define MCP_CNF3  0x05

---

## Autores

- Lucas Oliveira Rodrigues / Equipe Fórmula Tesla UFMG

Temporada 2025
