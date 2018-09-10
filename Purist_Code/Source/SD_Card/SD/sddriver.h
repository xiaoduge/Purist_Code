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

/* SD卡信息结构体定义 */ 
/* the information structure variable of SD Card*/
typedef struct SD_STRUCT
{	
	unsigned long block_num;				    /* 卡中块的数量 */
	unsigned long block_len;				    /* 卡的块长度(单位:字节) */
	unsigned long erase_unit;				    /* 一次可擦除的块个数 */
	
	unsigned int timeout_read;				/* 读块超时时间(单位: 8 SPI clock) */
	unsigned int timeout_write;			  /* 写块超时时间(单位: 8 SPI clock) */
	unsigned int timeout_erase;			  /* 擦块超时时间(单位: 8 SPI clock) */	

    unsigned int ccs;
}SD_INFO;

extern SD_INFO sds;				  /* SD卡信息结构体变量 */ 
/*
================================================================================
The following are some error codes for SD operations
================================================================================
*/		
/* 错误码 error code */
#define   SD_NO_ERR			     	    0x00			// 函数执行成功
#define   SD_ERR_NO_CARD		 	    0x01			// SD卡没有完全插入到卡座中
#define   SD_ERR_USER_PARAM      	0x02			// 用户使用API函数时，入口参数有错误
#define   SD_ERR_CARD_PARAM		 	  0x03			// 卡中参数有错误（与本模块不兼容）
#define	  SD_ERR_VOL_NOTSUSP      0x04			// 卡不支持3.3V供电
#define   SD_ERR_OVER_CARDRANGE		0x05			// 操作超出卡容量范围

/* SD命令可能返回的错误码 */
#define   SD_ERR_CMD_RESPTYPE	 	  0x10			// 命令类型错误
#define   SD_ERR_CMD_TIMEOUT     	0x11			// SD命令响应超时
#define   SD_ERR_CMD_RESP		 	    0x12			// SD命令响应错误
			
/* 数据流错误码 */
#define   SD_ERR_DATA_CRC16      	0x20			// 数据流CRC16校验不通过
#define   SD_ERR_DATA_START_TOK		0x21			// 读单块或多块时，数据开始令牌不正确
#define	  SD_ERR_DATA_RESP		 	  0x22			// 写单块或多块时，SD卡数据响应令牌不正确

/* 等待错误码 */
#define   SD_ERR_TIMEOUT_WAIT    	0x30			// 写或擦操作时，发生超时错误
#define   SD_ERR_TIMEOUT_READ    	0x31			// 读操作超时错误
#define	  SD_ERR_TIMEOUT_WRITE	 	0x32			// 写操作超时错误
#define   SD_ERR_TIMEOUT_ERASE   	0x33			// 擦除操作超时错误
#define	  SD_ERR_TIMEOUT_WAITIDLE 0x34			// 初始化SD卡时，等待SD卡进入空闲状态超时错误

/* 写操作可能返回的错误码 */
#define	  SD_ERR_WRITE_BLK			  0x40			// 写块数据错误
#define	  SD_ERR_WRITE_BLKNUMS    0x41			// 写多块时，想要写入的块与正确写入的块数不一致
#define   SD_ERR_WRITE_PROTECT		0x42			// 卡外壳的写保护开关打在写保护位置

/*
================================================================================
The following are some macro definations for SD card
================================================================================
*/
	    	    	    
/* Mask off the bits in the OCR corresponding to voltage range 3.2V to
 * 3.4V, OCR bits 20 and 21 */
#define MSK_OCR_33			 	    0xC0				//1100 0000B

/* 定义在初始化阶段,等待SD卡退出空闲状态的重试次数 */
/* Number of tries to wait for the card to go idle during initialization */
#define SD_IDLE_WAIT_MAX     	1000

/* SD卡命令超时时间(单位 8clock)*/
/* timeout of command */
#define SD_CMD_TIMEOUT   	 	  100
	
/* 100ms 相当的SPI时钟数(单位 unit: 8 clocks) */
/* 100ms correspond to SPI clock(unit: 8 clocks)*/
#define READ_TIMEOUT_100MS    100 * SPI_CLOCK / 1000 / 8

/* 250ms 相当的SPI时钟数(单位 unit: 8 clocks) */
/* 250ms correspond to SPI clock(unit: 8 clocks)*/
#define WRITE_TIMEOUT_250MS		250 * SPI_CLOCK / 1000 / 8    


/* CSD中一些域的字节位置(高字节在前) */
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


//CSD中一些域的掩码
#define TAAC_MSK			   0x07		//TACC 域掩码
#define NSAC_MSK			   0x78		//NSAC 域掩码

#define READ_BL_LEN_MSK	 0x0F		//READ_BL_LEN 的掩码

#define C_SIZE_MSK1			 0x03		//C_SIZE 高2位掩码
#define C_SIZE_MSK3			 0xC0		//C_SIZE 低2位掩码

#define C_SIZE_MULT_MSK1 0x03		//C_SIZE_MULT 的高2位掩码
#define C_SIZE_MULT_MSK2 0x80		//C_SIZE_MULT 的低2位掩码

#define R2WFACTOR_MSK		 0x1C		//R2WFACTOR 掩码

#define SECTOR_SIZE_MSK1 0x3F		//SECTOR_SIZE 的高5位
#define SECTOR_SIZE_MSK2 0x80		//SECTOR_SIZE 的低2位

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
** 函数说明: 该命令不断重复发送到SD卡，直到响应R1的Bit0(Idle)位为0，表示SD卡内部初始化处理完成。
		     当响应的Idle位为0时，SD卡就完全进入SPI模式了。当然重复发送命令CMD1是有次数限制的，
		     最大次数为宏定义SD_IDLE_WAIT_MAX.
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







