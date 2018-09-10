/*****************************************************************************************************
** Descriptions:		SD Command set 
********************************************************************************************************/
#ifndef 	__sdcmd_h
#define 	__sdcmd_h
/*
 *******************************************************	
		Commands and resonse	
 *******************************************************
*/

/* define command's response */
#define R1 		1
#define R1B 	2
#define R2 		3
#define R3 		4
#define R7 		5

/* R1 and upper byte of R2 error code */
#define MSK_IDLE          		  0x01
#define MSK_ERASE_RST     		  0x02
#define MSK_ILL_CMD       		  0x04
#define MSK_CRC_ERR       		  0x08
#define MSK_ERASE_SEQ_ERR 		  0x10
#define MSK_ADDR_ERR      		  0x20
#define MSK_PARAM_ERR     		  0x40

/* lower byte of R2 error code */
#define MSK_TOK_ERROR             0x01
#define MSK_TOK_CC_ERROR          0x02
#define MSK_TOK_ECC_FAILED        0x04
#define MSK_TOK_CC_OUTOFRANGE     0x08
#define MSK_TOK_CC_LOCKED         0x10

/* Data Tokens */
#define SD_TOK_READ_STARTBLOCK    0xFE
#define SD_TOK_WRITE_STARTBLOCK   0xFE
#define SD_TOK_READ_STARTBLOCK_M  0xFE
#define SD_TOK_WRITE_STARTBLOCK_M 0xFC
#define SD_TOK_STOP_MULTI         0xFD

/* Data Response Tokens */
#define SD_RESP_DATA_MSK		  0x0F		      //data response
#define SD_RESP_DATA_ACCETPTED	  0x05		  //data is accept
#define SD_RESP_DATA_REJECT_CRC	  0x0B      //CRC error
#define SD_RESP_DATA_REJECT_WRITE   0x0D		//write error

/*Wait Type */
#define SD_WAIT_READ			  0x00		//wait for read
#define SD_WAIT_WRITE		    0x01		//wait for write
#define SD_WAIT_ERASE		    0x02		//wait for erase


#define SD_READREG_TIMEOUT		  8


/*
*********************************************

     SD card SPI command set

*********************************************
*/

/******************************** Basic command set **************************/
/* Reset cards to idle state */
#define CMD0 0
#define CMD0_R R1

/* Read the OCR (MMC mode, do not use for SD cards) */
#define CMD1 1
#define CMD1_R R1

/* Card sends the COND IF */
#define CMD8 8
#define CMD8_R R7

/* Card sends the CSD */
#define CMD9 9
#define CMD9_R R1

/* Card sends CID */
#define CMD10 10
#define CMD10_R R1

/* Stop a multiple block (stream) read/write operation */
#define CMD12 12
#define CMD12_R R1B

/* Get the addressed card's status register */
#define CMD13 13
#define CMD13_R R2

/***************************** Block read commands **************************/

/* Set the block length */
#define CMD16 16
#define CMD16_R R1

/* Read a single block */
#define CMD17 17
#define CMD17_R R1

/* Read multiple blocks until a CMD12 */
#define CMD18 18
#define CMD18_R R1

/*****************************  Block write commands *************************/
/* Write a block of the size selected with CMD16 */
#define CMD24 24
#define CMD24_R R1

/* Multiple block write until a CMD12 */
#define CMD25 25
#define CMD25_R R1

/* Program the programmable bits of the CSD */
#define CMD27 27
#define CMD27_R R1

/***************************** Write protection *****************************/
/* Set the write protection bit of the addressed group */
#define CMD28 28
#define CMD28_R R1B

/* Clear the write protection bit of the addressed group */
#define CMD29 29
#define CMD29_R R1B

/* Ask the card for the status of the write protection bits */
#define CMD30 30
#define CMD30_R R1

/***************************** Erase commands *******************************/
/* Set the address of the first write block to be erased */
#define CMD32 32
#define CMD32_R R1

/* Set the address of the last write block to be erased */
#define CMD33 33
#define CMD33_R R1

/* Erase the selected write blocks */
#define CMD38 38
#define CMD38_R R1B

/***************************** Lock Card commands ***************************/
/* Set/reset the password or lock/unlock the card */
#define CMD42 42
#define CMD42_R	R1B
/* Commands from 42 to 54, not defined here */

/***************************** Application-specific commands ****************/
/* Flag that the next command is application-specific */
#define CMD55 55
#define CMD55_R R1

/* General purpose I/O for application-specific commands */
#define CMD56 56
#define CMD56_R R1

/* Read the OCR (SPI mode only) */
#define CMD58 58
#define CMD58_R R3

/* CRC Turn CRC on or off */
#define CMD59 59
#define CMD59_R R1

/***************************** Application-specific commands ***************/
/* Get the SD card's status */
#define ACMD13 13
#define ACMD13_R R2

/* Get the number of written write blocks (Minus errors ) */
#define ACMD22 22
#define ACMD22_R R1

/* Set the number of write blocks to be pre-erased before writing */
#define ACMD23 23
#define ACMD23_R R1

/* Get the card's OCR (SD mode) */
#define ACMD41 41
#define ACMD41_R R1

/* Connect or disconnect the 50kOhm internal pull-up on CD/DAT[3] */
#define ACMD42 42
#define ACMD42_R R1

/* Get the SD configuration register */
#define ACMD51 51
#define ACMD51_R R1



uint8 SD_SendCmd(uint8 cmd, uint8 *param, uint8 resptype, uint8 *resp);  
void SD_PackParam(uint8 *parameter, uint32 value);						  
uint8 SD_BlockCommand(uint8 cmd, uint8 resptype, uint32 parameter);		  

	
uint8 SD_ResetSD(void);											 

uint8 SD_ReadCSD(uint8 csdlen, uint8 *recbuf);					// 		read CSD register
uint8 SD_ReadCID(uint8 cidlen, uint8 *recbuf);					// 		read CID register
uint8 SD_StopTransmission(void);								//stop transmission	

uint8 SD_ReadCard_Status(uint8 len, uint8 *buffer);				//  read Card Status register
uint8 SD_SetBlockLen(uint32 length);							// 

uint8 SD_ReadSingleBlock(uint32 blockaddr);						//  
uint8 SD_ReadMultipleBlock(uint32 blockaddr);					//  

uint8 SD_WriteSingleBlock(uint32 blockaddr);					// 
uint8 SD_WriteMultipleBlock(uint32 blockaddr);					//  


uint8 SD_ProgramCSD(uint8 len, uint8 *buff);					//  	write CSD register 

uint8 SD_EraseStartBlock(uint32 startblock);					// erase start address
uint8 SD_EraseEndBlock(uint32 endblock);						// erase end address
uint8 SD_EraseSelectedBlock(void);								// erase the block


uint8 SD_ReadOCR(uint8 ocrlen,uint8 *recbuf);					// 		read OCR register
uint8 SD_EnableCRC(uint8 bEnable);								//  	enable CRC


//spetial command function
uint8 SD_ReadSD_Status(uint8 sdslen, uint8  *recbuf);			// read SD_Status 
uint8 SD_GetNumWRBlcoks(uint32  *blocknum);						//  
uint8 SD_ReadSCR(uint8 scrlen, uint8 *recbuf);					// read SCR register


//data stream functions
uint8 SD_ReadRegister(uint32 len, uint8 *recbuf);						//  
uint8 SD_ReadBlockData(uint32 len, uint8 *recbuf);						//  
uint8 SD_WriteBlockData(uint8 bmulti, uint32 len, uint8 *sendbuf);		//  

//Other functions
void SD_StopMultiToken(void);									//stop 
uint8 SD_WaitBusy(uint8 waittype);						//busy
void SD_SPIDelay(uint8 value);								//Create some clocks


#endif













