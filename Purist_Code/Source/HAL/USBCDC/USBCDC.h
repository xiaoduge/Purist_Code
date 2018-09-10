//USBCDC for STM32
#ifndef _USBCDC_H__
#define _USBCDC_H__

void USBCDC_init(void); //��ʼ��USBCDC���⴮�� 

bool USBCDC_ready(void);//��ȡ����״̬,TRUE��ʾ����ͨѶ 

void USBCDC_connect(void);//��USB����
void USBCDC_disconnect(void);//�Ͽ�USB����

void USBCDC_senddata(uint8_t *pdata,uint16_t length);//������
uint16_t USBCDC_recvdata(uint8_t *pdata, uint16_t limit);//������,����ʵ�ʽ��յ������ݳ���
//limit��pdata��������
void USBCDC_sync(void);//�ȴ�������� 

void __USBCDC_ISR(void);//USB�жϷ���ʱ����
//��usb_istr.c��ʵ��

#endif
