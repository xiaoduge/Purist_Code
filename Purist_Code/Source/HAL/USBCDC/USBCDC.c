//USBCDC for STM32
#include "stm32f10x.h"

#include "USBCDC.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_conf.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_istr.h"

extern LINE_CODING linecoding;

void USBCDC_init(void)
{
    Set_GPIO();
    Set_USBClock(); 
    USB_Interrupts_Config();
    USB_Init();
}

void USBCDC_connect(void)
{
    PowerOn(); 
} 

void USBCDC_disconnect(void)
{
    PowerOff();
}

bool USBCDC_ready(void)
{
    if(bDeviceState == CONFIGURED) return TRUE;
    else return FALSE;
}

/*synchronize between USB host and device*/
void USBCDC_sync(void)
{
	if(bDeviceState != CONFIGURED) return;
	//When USB is disconnected, ignore the sync operation
	while(USART_Rx_ptr_in != USART_Rx_ptr_out);
	//wait for transmit complete
}

/*******************************************************************************
* Function Name  : USBCDC_senddata.
* Description    : USB CDC virtual COM port send data
* Input           :  data ,data to be send to usb port
                        length ,data buffer length
* Output         : None.
* Return          : None.
*******************************************************************************/
void USBCDC_senddata(uint8_t *data,uint16_t length)
{
    uint8_t mask = (linecoding.datatype == 7) ? 0x7F : 0xff;
    
    while(length)
    {
        USART_Rx_Buffer[USART_Rx_ptr_in] = *data & mask;
        data++;
        length--;
        USART_Rx_ptr_in++;
        
        if(USART_Rx_ptr_in == USART_RX_DATA_SIZE) 
        {
            USART_Rx_ptr_in = 0;
        }
    }
}

/*******************************************************************************
* Function Name  : USBCDC_recvdata.
* Description       : retrive data received from USB port
* Input               :  None
* Output             : pdata ,data buffer for holding data.
                           limit , maximun data to retrive
* Return          :  actual data retrived.
*******************************************************************************/
uint16_t USBCDC_recvdata(uint8_t *pdata,uint16_t limit) 
{
	uint16_t length;

	if(USART_Tx_ptr_in == USART_Tx_ptr_out) 
    {   
        return 0;
    }
	if(USART_Tx_ptr_in > USART_Tx_ptr_out)
    {   
		length = USART_Tx_ptr_in - USART_Tx_ptr_out;
    }
  	else
    { 
  	 	length = USART_Tx_ptr_in + USART_TX_DATA_SIZE - USART_Tx_ptr_out;
    }

	if(length > limit)
    {   
		length = limit;
    }

    limit = length;
    
	while(length)
	{
		*pdata = USART_Tx_Buffer[USART_Tx_ptr_out];
		USART_Tx_ptr_out++;
		if(USART_Tx_ptr_out >= USART_TX_DATA_SIZE) 
            USART_Tx_ptr_out = 0;
		pdata++;
		length--;
	}

	return limit;
}
