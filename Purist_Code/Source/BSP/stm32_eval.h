/**
  ******************************************************************************
  * @file    stm32_eval.h
  * @author  MCD Application Team
  * @version V4.2.0
  * @date    04/16/2010
  * @brief   Header file for stm32_eval.c module.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_EVAL_H
#define __STM32_EVAL_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/

/** @addtogroup Utilities
  * @{
  */ 
  
/** @addtogroup STM32_EVAL
  * @{
  */ 

/** @defgroup STM32_EVAL_Abstraction_Layer
  * @{
  */
  
/** @defgroup STM32_EVAL_HARDWARE_RESOURCES
  * @{
  */

/**
@code  
 The table below gives an overview of the hardware resources supported by each 
 STM32 EVAL board.
     - LCD: TFT Color LCD (Parallel (FSMC) and Serial (SPI))
     - IOE: IO Expander on I2C
     - sFLASH: serial SPI FLASH (M25Pxxx)
     - sEE: serial I2C EEPROM (M24C08, M24C32, M24C64)
     - TSENSOR: Temperature Sensor (LM75)
     - SD: SD Card memory (SPI and SDIO (SD Card MODE)) 
  =================================================================================================================+
    STM32 EVAL     | LED | Buttons  | Com Ports |    LCD    | IOE  | sFLASH | sEE | TSENSOR | SD (SPI) | SD(SDIO)  |
  =================================================================================================================+
   STM3210B-EVAL   |  4  |    8     |     2     | YES (SPI) | NO   |  YES   | NO  |   YES   |    YES   |    NO     |
  -----------------------------------------------------------------------------------------------------------------+
   STM3210E-EVAL   |  4  |    8     |     2     | YES (FSMC)| NO   |  YES   | NO  |   YES   |    NO    |    YES    |
  -----------------------------------------------------------------------------------------------------------------+
   STM3210C-EVAL   |  4  |    3     |     1     | YES (SPI) | YES  |  NO    | YES |   NO    |    YES   |    NO     |
  -----------------------------------------------------------------------------------------------------------------+
   STM32100B-EVAL  |  4  |    8     |     2     | YES (SPI) | NO   |  YES   | NO  |   YES   |    YES   |    NO     |
  =================================================================================================================+
@endcode
*/

/**
  * @}
  */
  
/** @defgroup STM32_EVAL_Exported_Types
  * @{
  */
typedef enum 
{
  LED_RUN = 0, // pd2 YLF: FOR RUN
  LED_NUM,
} Led_TypeDef;


typedef enum 
{
  DI_SENSOR1 = 0,  //PX0
  DI_SENSOR2 ,     //PX1
  DI_SENSOR3 ,     //PX2
  DI_SENSOR4 ,     //PX3
  DI_SENSOR5 ,     //PX4
  DI_SENSOR6 ,     //PX5
  DI_SENSOR7 ,     //PX6
  DI_SENSOR8 ,     //PX7
  DI_SENSOR9 ,     //PX8
  DI_SENSOR10 ,    //PX9
  DI_SENSOR11 ,    //PX10
  DI_SENSOR12 ,    //PX11
  DI_SENSOR13 ,    //PX12
  DI_SENSOR14 ,    //PX13
  DI_SENSOR15 ,    //PX14
  DI_SENSOR16 ,    //PX15
  DI_SENSOR_NUM,
} DI_SENSOR_TypeDef;


/**
  * @}
  */ 

#define GPIO_HIGH(a,b) 		a->BSRR = b
#define GPIO_LOW(a,b)		a->BRR = b
#define GPIO_TOGGLE(a,b) 	a->ODR ^= b 

#define GPIO_STATE_HIGH(a,b) ((a->IDR & b) != Bit_RESET)
#define GPIO_STATE_LOW(a,b) (!((a->IDR & b) != Bit_RESET))

  
/** @defgroup STM32_EVAL_Exported_Constants
  * @{
  */ 


 #include "stm32f10x.h"
 #include "stm3210c_eval/stm3210c_eval.h"

 #include "gpio.h"

 #include "voslog.h"

/** @defgroup STM32_EVAL_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM32_EVAL_Exported_Functions
  * @{
  */ 
/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif


#endif /* __STM32_EVAL_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */   

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
