//USBCDC for STM32
#ifndef _USBCDC_H__
#define _USBCDC_H__

void USBCDC_init(void); //初始化USBCDC虚拟串口 

bool USBCDC_ready(void);//获取串口状态,TRUE表示可以通讯 

void USBCDC_connect(void);//打开USB连接
void USBCDC_disconnect(void);//断开USB连接

void USBCDC_senddata(uint8_t *pdata,uint16_t length);//发数据
uint16_t USBCDC_recvdata(uint8_t *pdata, uint16_t limit);//收数据,返回实际接收到的数据长度
//limit是pdata区的容量
void USBCDC_sync(void);//等待传送完成 

void __USBCDC_ISR(void);//USB中断发生时调用
//在usb_istr.c中实现

#endif
