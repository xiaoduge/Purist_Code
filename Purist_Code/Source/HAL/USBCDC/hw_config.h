/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : hw_config.h
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : Hardware Configuration & Setup
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "usb_type.h"

#define USART_RX_DATA_SIZE   512
#define USART_TX_DATA_SIZE   512

extern uint8_t  USART_Rx_Buffer [USART_RX_DATA_SIZE]; 
extern uint32_t USART_Rx_ptr_in;
extern uint32_t USART_Rx_ptr_out;

extern uint8_t  USART_Tx_Buffer [USART_TX_DATA_SIZE]; 
extern uint32_t USART_Tx_ptr_in;
extern uint32_t USART_Tx_ptr_out;

/* Exported functions ------------------------------------------------------- */

/* UART Events */
#define HAL_UART_RX_FULL         0x01
#define HAL_UART_RX_ABOUT_FULL   0x02
#define HAL_UART_RX_TIMEOUT      0x04
#define HAL_UART_TX_FULL         0x08
#define HAL_UART_TX_EMPTY        0x10

typedef void (*halUARTCBack_t) (uint8_t port, uint8_t event);

//��ֲʱ��Ҫ�޸�
void Set_GPIO(void);
void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
void Get_SerialNum(void);
void USB_OTG_BSP_uDelay (const uint32_t usec);

//��ֲʱ����Ҫ�޸�
void USB_To_USART_Send_Data(uint8_t* data_buffer, uint8_t Nb_bytes);
void Handle_USBAsynchXfer (void);
void USB_SetUsb2UartCallback (halUARTCBack_t cb);


/* External variables --------------------------------------------------------*/

#endif  /*__HW_CONFIG_H*/
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
