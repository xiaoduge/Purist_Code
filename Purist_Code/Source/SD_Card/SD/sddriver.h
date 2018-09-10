/*
================================================================================
sddriver.c
Author : LiYong
Date   : 2008-12-4 09:14
Description: This file contains some SD card operations, And they are based on
sdcmd.c. User can call the functions in sdcmd.c and compose his functions. But
User must offer some SPI functions in this head file, They are:
     void SPI_Initialize( void );  //SPI initialized as low speed
     void SPI_CS_Assert( void );   //Pull low the CS line by software
     void SPI_CS_Deassert( void ); //Release the CS line, Pull high
     unsigned char SPI_SendByte( unsigned char dt );//SPI send byte
     unsigned char SPI_RecByte( void );//SPI receive byte
     void SPI_SetLowSpeed( void );//Set SPI low speed, Less than 400kbps
     void SPI_SetHighSpeed( void );//Set SPI high speed, Max:25Mbps
================================================================================
*/
#ifndef 	__sddriver_h
#define 	__sddriver_h

#include "sddriver_hal.h"

/* SD����Ϣ�ṹ�嶨�� */ 
/* the information structure variable of SD Card*/
typedef struct SD_STRUCT
{	
	unsigned long block_num;				    /* ���п������ */
	unsigned long block_len;				    /* ���Ŀ鳤��(��λ:�ֽ�) */
	unsigned long erase_unit;				    /* һ�οɲ����Ŀ���� */
	
	unsigned int timeout_read;				/* ���鳬ʱʱ��(��λ: 8 SPI clock) */
	unsigned int timeout_write;			  /* д�鳬ʱʱ��(��λ: 8 SPI clock) */
	unsigned int timeout_erase;			  /* ���鳬ʱʱ��(��λ: 8 SPI clock) */	

    unsigned int ccs;
}SD_INFO;

extern SD_INFO sds;				  /* SD����Ϣ�ṹ����� */ 
/*
================================================================================
The following are some error codes for SD operations
================================================================================
*/		
/* ������ error code */
#define   SD_NO_ERR			     	    0x00			// ����ִ�гɹ�
#define   SD_ERR_NO_CARD		 	    0x01			// SD��û����ȫ���뵽������
#define   SD_ERR_USER_PARAM      	0x02			// �û�ʹ��API����ʱ����ڲ����д���
#define   SD_ERR_CARD_PARAM		 	  0x03			// ���в����д����뱾ģ�鲻���ݣ�
#define	  SD_ERR_VOL_NOTSUSP      0x04			// ����֧��3.3V����
#define   SD_ERR_OVER_CARDRANGE		0x05			// ����������������Χ

/* SD������ܷ��صĴ����� */
#define   SD_ERR_CMD_RESPTYPE	 	  0x10			// �������ʹ���
#define   SD_ERR_CMD_TIMEOUT     	0x11			// SD������Ӧ��ʱ
#define   SD_ERR_CMD_RESP		 	    0x12			// SD������Ӧ����
			
/* ������������ */
#define   SD_ERR_DATA_CRC16      	0x20			// ������CRC16У�鲻ͨ��
#define   SD_ERR_DATA_START_TOK		0x21			// ���������ʱ�����ݿ�ʼ���Ʋ���ȷ
#define	  SD_ERR_DATA_RESP		 	  0x22			// д�������ʱ��SD��������Ӧ���Ʋ���ȷ

/* �ȴ������� */
#define   SD_ERR_TIMEOUT_WAIT    	0x30			// д�������ʱ��������ʱ����
#define   SD_ERR_TIMEOUT_READ    	0x31			// ��������ʱ����
#define	  SD_ERR_TIMEOUT_WRITE	 	0x32			// д������ʱ����
#define   SD_ERR_TIMEOUT_ERASE   	0x33			// ����������ʱ����
#define	  SD_ERR_TIMEOUT_WAITIDLE 0x34			// ��ʼ��SD��ʱ���ȴ�SD���������״̬��ʱ����

/* д�������ܷ��صĴ����� */
#define	  SD_ERR_WRITE_BLK			  0x40			// д�����ݴ���
#define	  SD_ERR_WRITE_BLKNUMS    0x41			// д���ʱ����Ҫд��Ŀ�����ȷд��Ŀ�����һ��
#define   SD_ERR_WRITE_PROTECT		0x42			// ����ǵ�д�������ش���д����λ��

/*
================================================================================
The following are some macro definations for SD card
================================================================================
*/
	    	    	    
/* Mask off the bits in the OCR corresponding to voltage range 3.2V to
 * 3.4V, OCR bits 20 and 21 */
#define MSK_OCR_33			 	    0xC0				//1100 0000B

/* �����ڳ�ʼ���׶�,�ȴ�SD���˳�����״̬�����Դ��� */
/* Number of tries to wait for the card to go idle during initialization */
#define SD_IDLE_WAIT_MAX     	1000

/* SD�����ʱʱ��(��λ 8clock)*/
/* timeout of command */
#define SD_CMD_TIMEOUT   	 	  100
	
/* 100ms �൱��SPIʱ����(��λ unit: 8 clocks) */
/* 100ms correspond to SPI clock(unit: 8 clocks)*/
#define READ_TIMEOUT_100MS    100 * SPI_CLOCK / 1000 / 8

/* 250ms �൱��SPIʱ����(��λ unit: 8 clocks) */
/* 250ms correspond to SPI clock(unit: 8 clocks)*/
#define WRITE_TIMEOUT_250MS		250 * SPI_CLOCK / 1000 / 8    


/* CSD��һЩ����ֽ�λ��(���ֽ���ǰ) */
#define TAAC_POS 			    1			//TACC
#define NSAC_POS			    2			//NSAC

#define READ_BL_LEN_POS		5			//READ_BL_LEN

#define C_SIZE_POS1			  6			//C_SIZE upper  2-bit
#define C_SIZE_POS2			  7			//C_SIZE middle 8-bit
#define C_SIZE_POS3			  8			//C_SIZE lower	2-bit

#define C_SIZE_MULT_POS1	9			//C_SIZE_MULT upper 2-bit
#define C_SIZE_MULT_POS2	10			//C_SIZE_MULT lower 1-bit	

#define SECTOR_SIZE_POS1	10			//SECTOR_SIZE upper 5-bit
#define SECTOR_SIZE_POS2	11			//SECTOR_SIZE lower 2-bit

#define R2WFACTOR_POS 		12			//R2WFACTOR_POS


//CSD��һЩ�������
#define TAAC_MSK			   0x07		//TACC ������
#define NSAC_MSK			   0x78		//NSAC ������

#define READ_BL_LEN_MSK	 0x0F		//READ_BL_LEN ������

#define C_SIZE_MSK1			 0x03		//C_SIZE ��2λ����
#define C_SIZE_MSK3			 0xC0		//C_SIZE ��2λ����

#define C_SIZE_MULT_MSK1 0x03		//C_SIZE_MULT �ĸ�2λ����
#define C_SIZE_MULT_MSK2 0x80		//C_SIZE_MULT �ĵ�2λ����

#define R2WFACTOR_MSK		 0x1C		//R2WFACTOR ����

#define SECTOR_SIZE_MSK1 0x3F		//SECTOR_SIZE �ĸ�5λ
#define SECTOR_SIZE_MSK2 0x80		//SECTOR_SIZE �ĵ�2λ

/*
================================================================================
Function list :
unsigned char SD_Initialize( void );
void SD_ReadSector( unsigned long SectorNum, unsigned char* buffer );    
unsigned char SD_GetCardInfo( void );
void SD_CalTimeout(unsigned char *csdbuf);
unsigned char SD_ActiveInit(void);
void SD_WriteSector( unsigned long SectorNum, unsigned char* buffer );
================================================================================
*/
/*
================================================================================
Funtion : SD_Initialize( )
Description: Initialize the SD card
Input   : None
Output  : None
Note    : None        
================================================================================
*/
unsigned char SD_Initialize( void );
/*
================================================================================
Funtion : SD_ReadSector( )
Description: Read a single block from SD card
Input   : -SectorNum, Number of the sector
          -buffer, Buffer to store the data
Output  : None
Note    : None        
================================================================================
*/
unsigned char SD_ReadSector( unsigned long SectorNum, unsigned char * buffer );


/*
================================================================================
Funtion : SD_ReadMultiSector( )
Description: Read a single block from SD card
Input   : -SectorNum, Number of the sector
          -buffer, Buffer to store the data
Output  : None
Note    : None        
================================================================================
*/
unsigned char SD_ReadMultiSector( unsigned long SectorNum, unsigned char * buffer ,unsigned char count);

/*
================================================================================
Funtion : SD_GetCardInfo( )
Description: Get the informations of the SD card
Input   : None
Output  : SD error code, 0: no error
Note    : None        
================================================================================
*/
unsigned char SD_GetCardInfo( void ) ;
/*******************************************************************************************************************
** Name:	  INT8U SD_CalTimeout()
** Function: get the information of SD card
** Input:    INT8U cardtype: card type	
		         INT8U *csdbuf : CSD register content
** Output:	  0:  right		>0:  error code
*******************************************************************************************************************/
void SD_CalTimeout(unsigned char *csdbuf);
/*******************************************************************************************************************
** Name:	  INT8U SD_CalTimeout()
** Function: get the information of SD card
** Input:    INT8U *cardtype: card type receive buff
** Output:	  0:  right		>0:  error code
** ����˵��: ��������ظ����͵�SD����ֱ����ӦR1��Bit0(Idle)λΪ0����ʾSD���ڲ���ʼ��������ɡ�
		     ����Ӧ��IdleλΪ0ʱ��SD������ȫ����SPIģʽ�ˡ���Ȼ�ظ���������CMD1���д������Ƶģ�
		     ������Ϊ�궨��SD_IDLE_WAIT_MAX.
*******************************************************************************************************************/
unsigned char SD_ActiveInit(void);


uint8 SD_SendIFCond(void);

/*
================================================================================
Funtion : SD_WriteSector( )
Description: Read a single block from SD card
Input   : -SectorNum, Number of the sector
          -buffer, Buffer to store the data
Output  : None
Note    : None        
================================================================================
*/
unsigned char SD_WriteSector( unsigned long SectorNum, unsigned char * buffer );

/*
================================================================================
Funtion : SD_WriteSector( )
Description: Read a single block from SD card
Input   : -SectorNum, Number of the sector
          -buffer, Buffer to store the data
Output  : None
Note    : None        
================================================================================
*/
unsigned char SD_WriteMultiSector( unsigned long SectorNum, unsigned char * buffer ,unsigned char count);


//#define DEBUG_SD

#ifdef DEBUG_SD

#define DEBUG_SD_INFO(fmt,arg...) printf(fmt,## arg) 
#else
#define DEBUG_SD_INFO(fmt,arg...)
#endif

/*
================================================================================
============================End of file=========================================        
================================================================================
*/
#endif







