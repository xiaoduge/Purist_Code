/******************************************************************************************************
** Descriptions:		sd �����������: �����ļ�
********************************************************************************************************/


/* �������ݴ���ʱ�Ƿ�ʹ��CRC */
#define SD_CRC_EN		   		0

#define SPI_CLOCK				5529600		/* ����ͨ��ʱ,SPIʱ��Ƶ�� frequency (Hz) */

#define SD_BLOCKSIZE 			512			/* SD����ĳ��� */

#define SD_BLOCKSIZE_NBITS		9  

/* ���溯��������,����û�����Ҫ,����Ϊ 0 �ü�ָ������ */

#define SD_ReadMultiBlock_EN    0			/* �Ƿ�ʹ�ܶ���麯�� */

#define SD_WriteMultiBlock_EN   0			/* �Ƿ�ʹ��д��麯�� */

#define SD_EraseBlock_EN		0			/* �Ƿ�ʹ�ܲ������� */

#define SD_ProgramCSD_EN   		0			/* �Ƿ�ʹ��дCSD�Ĵ������� */
	
#define SD_ReadCID_EN	   		1			/* �Ƿ�ʹ�ܶ�CID�Ĵ������� */
	
#define	SD_ReadSD_Status_EN		0			/* �Ƿ�ʹ�ܶ�SD_Status�Ĵ������� */

#define	SD_ReadSCR_EN			0			/* �Ƿ�ʹ�ܶ�SCR�Ĵ������� */

