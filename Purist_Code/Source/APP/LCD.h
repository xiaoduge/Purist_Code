#ifndef __LCD_H
#define __LCD_H		

/**
  ******************************************************************************
  * @file    LCD.h 
  * @author  STM32 Team
  * @version V1.0.0
  * @date    10/10/2014
  * @brief   Interface for LCD Driver access.
  ******************************************************************************
  *          DO NOT MODIFY ANY OF THE FOLLOWING CODE EXCEPT UNDER PERMISSION OF ITS PROPRIETOR
  *
  * Should you have any questions regarding your right to use this Software,
  * contact Shanghai ZhenNeng Corporation at www.tranun.com.
  *
  * @copy
  *
  *
  * <h2><center>&copy; COPYRIGHT 2012 Shanghai ZhenNeng Corporation</center></h2>
  */ 

#define LCD_HORIZON_DISP_MODE

#ifdef LCD_HORIZON_DISP_MODE
#define LCD_W 320
#define LCD_H 240
#else
#define LCD_W 240
#define LCD_H 320
#endif

void LCD_Init(void); 
void LCD_Clear(u16 Color);
void LCD_AddressSet(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
void LCD_WR_DATA(u16 da);
void LCD_WR_REG(u16 da);

void LCD_DrawPoint(u16 x,u16 y,u16 color);//����
void LCD_DrawPoint_big(u16 x,u16 y,u16 color);//��һ�����
u16  LCD_ReadPoint(u16 x,u16 y); //����
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);		   
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);
void LCD_PutString8x16(u16 x,u16 y,u8 *string,u16 charColor,u16 bkColor);
void LCD_PutChar8x16(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor);
void LCD_GetStringGB8x16TextSize(u8 *s, int* sx, int* sy); 
void LCD_PutString12x24(u16 x,u16 y,char *string,u16 charColor,u16 bkColor);
void LCD_PutChar12x24(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor);
void PutStringGB3232(u16 x, u16 y, u8 *s, u16 fColor, u16 bColor);
void LCD_GetStringGB32x32TextSize(u8 *s, int* sx, int* sy); 
void LCD_PutString16x32(u16 x,u16 y,char *string,u16 charColor,u16 bkColor);
void LCD_PutStringGB24x24(u16 x, u16 y, u8 *s, u16 fColor, u16 bColor);
void LCD_GetStringGB24x24TextSize(u8 *s, int* sx, int* sy); 
void PutStringGB4848(u16 x, u16 y, u8 *s, u16 fColor, u16 bColor);
void LCD_GetStringGB48x48TextSize(u8 *s, int* sx, int* sy) ;

void PutGB2424(u16 x, u16  y, u8 c[2], u16 fColor,u16 bColor);
void LCD_EnableBackLight(int bEnable);
int LCD_GetBackLightState(void);
void LCD_ClearRect(u16 x1,u16 y1,u16 x2,u16 y2,u16 Color);

//������ɫ
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //��ɫ
#define BRRED 			 0XFC07 //�غ�ɫ
#define GRAY  			 0X8430 //��ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	 0X841F //ǳ��ɫ
#define LGRAY 			 0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE        0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE           0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)

typedef  void (*FONT_DrawText)(u16 x, u16 y, u8 *s, u16 fColor, u16 bColor); 
typedef  void  (*FONT_GetTextSize)(u8 *s,int *x,int *y); 

typedef  void  (*FONT_PutChar)(u16 x, u16 y, u8 c, u16 fColor, u16 bColor); 

typedef struct
{
    uint8_t sizeX;
    uint8_t sizeY;
    FONT_PutChar  DrawChar;
    FONT_DrawText DrawText;
    FONT_GetTextSize GetTextSize;
    
}FONT_STRU;

					  		 
#endif  
	 
	 



