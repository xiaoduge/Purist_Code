/******************************************************************************************************
** Descriptions:		sd 卡驱动软件包: 配置文件
********************************************************************************************************/


/* 设置数据传输时是否使用CRC */
#define SD_CRC_EN		   		0

#define SPI_CLOCK				5529600		/* 正常通信时,SPI时钟频率 frequency (Hz) */

#define SD_BLOCKSIZE 			512			/* SD卡块的长度 */

#define SD_BLOCKSIZE_NBITS		9  

/* 下面函数不常用,如果用户不需要,可置为 0 裁剪指定函数 */

#define SD_ReadMultiBlock_EN    0			/* 是否使能读多块函数 */

#define SD_WriteMultiBlock_EN   0			/* 是否使能写多块函数 */

#define SD_EraseBlock_EN		0			/* 是否使能擦卡函数 */

#define SD_ProgramCSD_EN   		0			/* 是否使能写CSD寄存器函数 */
	
#define SD_ReadCID_EN	   		1			/* 是否使能读CID寄存器函数 */
	
#define	SD_ReadSD_Status_EN		0			/* 是否使能读SD_Status寄存器函数 */

#define	SD_ReadSCR_EN			0			/* 是否使能读SCR寄存器函数 */

