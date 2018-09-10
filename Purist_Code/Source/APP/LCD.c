
#include    <ucos_ii.h>

#include    <string.h>

#include "stm32f10x.h"

#include "stm32_eval.h"

#include "LCD.h"

#include "font.h"

#define LCD_Pin_RS      GPIO_Pin_0
#define LCD_PORT_RS     GPIOC
#define LCD_CLK_RS      RCC_APB2Periph_GPIOC

#define LCD_Pin_WR      GPIO_Pin_1
#define LCD_PORT_WR     GPIOC
#define LCD_CLK_WR      RCC_APB2Periph_GPIOC

#define LCD_Pin_CS      GPIO_Pin_3
#define LCD_PORT_CS     GPIOC
#define LCD_CLK_CS      RCC_APB2Periph_GPIOC

#define LCD_Pin_RD      GPIO_Pin_2
#define LCD_PORT_RD     GPIOC
#define LCD_CLK_RD      RCC_APB2Periph_GPIOC

#define LCD_Pin_RESET   GPIO_Pin_4
#define LCD_PORT_RESET  GPIOC
#define LCD_CLK_RESET   RCC_APB2Periph_GPIOC

#define LCD_Pin_BL      GPIO_Pin_5
#define LCD_PORT_BL     GPIOC
#define LCD_CLK_BL      RCC_APB2Periph_GPIOC

#define LCD_DELAY_NUM 1

#define LCD_DELAY(tm) do {int i= 0; while (i++ < tm);}while(0);


#define Set_nCs()            do {GPIO_HIGH(LCD_PORT_CS, LCD_Pin_CS);LCD_DELAY(LCD_DELAY_NUM);}while(0);
#define Clr_nCs()            do {GPIO_LOW(LCD_PORT_CS, LCD_Pin_CS);LCD_DELAY(LCD_DELAY_NUM);}while(0);
 

#define Set_nWr()            do {GPIO_HIGH(LCD_PORT_WR, LCD_Pin_WR);LCD_DELAY(LCD_DELAY_NUM);}while(0);
#define Clr_nWr()            do {GPIO_LOW(LCD_PORT_WR, LCD_Pin_WR);LCD_DELAY(LCD_DELAY_NUM);}while(0);

#define Set_Rs()             do {GPIO_HIGH(LCD_PORT_RS, LCD_Pin_RS);LCD_DELAY(LCD_DELAY_NUM);}while(0);
#define Clr_Rs()             do {GPIO_LOW(LCD_PORT_RS, LCD_Pin_RS);LCD_DELAY(LCD_DELAY_NUM);}while(0);

#define Set_nRd()            do {GPIO_HIGH(LCD_PORT_RD, LCD_Pin_RD);LCD_DELAY(LCD_DELAY_NUM);}while(0);
#define Clr_nRd()            do {GPIO_LOW(LCD_PORT_RD, LCD_Pin_RD);LCD_DELAY(LCD_DELAY_NUM);}while(0);

#define Set_nRst()           do {GPIO_HIGH(LCD_PORT_RESET, LCD_Pin_RESET);LCD_DELAY(LCD_DELAY_NUM);}while(0);
#define Clr_nRst()           do {GPIO_LOW(LCD_PORT_RESET, LCD_Pin_RESET);LCD_DELAY(LCD_DELAY_NUM);}while(0);

#define Set_nBL()            do {GPIO_HIGH(LCD_PORT_BL, LCD_Pin_BL);LCD_DELAY(LCD_DELAY_NUM);}while(0);
#define Clr_nBL()            do {GPIO_LOW(LCD_PORT_BL, LCD_Pin_BL);LCD_DELAY(LCD_DELAY_NUM);}while(0);
#define Get_nBL()            GPIO_STATE_LOW(LCD_PORT_BL, LCD_Pin_BL)


#define LCD_Write(LCD_DATA)  do {GPIO_Write(GPIOB, LCD_DATA);LCD_DELAY(LCD_DELAY_NUM);}while(0);
#define LCD_Read()           GPIO_ReadInputData(GPIOB);

#define LCD_Status(LCD_DATA)  (GPIOB->ODR)

//#define ILI9325


#ifndef  Bus_8    //条件编译-16位数据模式   
void LCD_Writ_Bus(uint16_t data)   //并行数据写入函数
{
    LCD_Write(data);    
    Clr_nWr();
    Set_nWr(); 
}
#else           //条件编译-8位数据模式 
void LCD_Writ_Bus(uint16_t data)   //并行数据写入函数
{   
    uint16_t tmp = LCD_Status();

    tmp &= 0xff00;  
    tmp |= (data >>8) & 0xff;
    Clr_nWr();
    Set_nWr();
    tmp &= 0xff00;  
    tmp |= (data >> 0) & 0xff;
    Clr_nWr();
    Set_nWr();
}
#endif

 void LCD_WR_DATA(uint16_t data)
{
    Set_Rs();
    LCD_Writ_Bus(data);
}     
void LCD_WR_REG(uint16_t data)   
{   
    Clr_Rs();
    LCD_Writ_Bus(data);
}
 void LCD_WR_REG_DATA(int reg,int da)
{
    LCD_WR_REG(reg);
    LCD_WR_DATA(da);
}
 
void LCD_AddressSet(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{ 
    if (x1 >= LCD_W)  x1 = LCD_W -1;
    if (y1 >= LCD_H)  y1 = LCD_H -1;

    if (x2 >= LCD_W)  x2 = LCD_W -1;
    if (y2 >= LCD_H)  y2 = LCD_H -1;
#if defined(ILI9325)
#ifdef LCD_HORIZON_DISP_MODE
    LCD_WR_REG_DATA(0x0020,y1);              //设置X坐标位置
    LCD_WR_REG_DATA(0x0021,LCD_W-1-x1);      //设置Y坐标位置
    LCD_WR_REG_DATA(0x0050,y1);              //开始X
    //LCD_WR_REG_DATA(0x0052,LCD_W-1-x1);      //开始Y
    LCD_WR_REG_DATA(0x0052,LCD_W-1-x2);      //开始Y
    LCD_WR_REG_DATA(0x0051,y2);              //结束X
    //LCD_WR_REG_DATA(0x0053,LCD_W-1-x2);      //结束Y
    LCD_WR_REG_DATA(0x0053,LCD_W-1-x1);      //结束Y

#else
    LCD_WR_REG_DATA(0x0020,x1);              //设置X坐标位置
    LCD_WR_REG_DATA(0x0021,y1);              //设置Y坐标位置
    LCD_WR_REG_DATA(0x0050,x1);              //开始X
    LCD_WR_REG_DATA(0x0052,y1);              //开始Y
    LCD_WR_REG_DATA(0x0051,x2);              //结束X
    LCD_WR_REG_DATA(0x0053,y2);              //结束Y
#endif  
    LCD_WR_REG(0x0022);                      // ylf ->Write Data to GRAM         
#else
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(x1>>8);
    LCD_WR_DATA(x1);
    LCD_WR_DATA(x2>>8);
    LCD_WR_DATA(x2);
  
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(y1>>8);
    LCD_WR_DATA(y1);
    LCD_WR_DATA(y2>>8);
    LCD_WR_DATA(y2);	
    LCD_WR_REG(0x2c);	
#endif
}


void LCD_SetCursor(u16 x,u16 y)
{
#if defined(ILI9325)
#ifdef LCD_HORIZON_DISP_MODE
    LCD_WR_REG_DATA(0x0020,y);    //设置X坐标位置
    LCD_WR_REG_DATA(0x0021,LCD_W-1-x);   //设置Y坐标位置
#else
    LCD_WR_REG_DATA(0x0020,x);    //设置X坐标位置
    LCD_WR_REG_DATA(0x0021,y);   //设置Y坐标位置
#endif    
    LCD_WR_REG(0x0022);          // ylf ->Write Data to GRAM               
#else
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(x>>8);
    LCD_WR_DATA(x);
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(y>>8);
    LCD_WR_DATA(y);

#endif
}


void LCD_Init(void)
{

    // gpio init
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(LCD_CLK_RS | RCC_APB2Periph_GPIOB |
                           LCD_CLK_WR | LCD_CLK_RD |LCD_CLK_RESET|
                           LCD_CLK_CS |LCD_CLK_BL, ENABLE);
    
    // DB15--0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //LCD_Pin_WR
    GPIO_InitStructure.GPIO_Pin = LCD_Pin_WR;
    GPIO_Init(LCD_PORT_WR, &GPIO_InitStructure);
    
    //LCD_Pin_CS
    GPIO_InitStructure.GPIO_Pin = LCD_Pin_CS;
    GPIO_Init(LCD_PORT_CS, &GPIO_InitStructure);
    
    //LCD_Pin_RS
    GPIO_InitStructure.GPIO_Pin = LCD_Pin_RS;
    GPIO_Init(LCD_PORT_RS, &GPIO_InitStructure);
    
    //LCD_Pin_RD
    GPIO_InitStructure.GPIO_Pin = LCD_Pin_RD;
    GPIO_Init(LCD_PORT_RD, &GPIO_InitStructure);

    //LCD_Pin_RD
    GPIO_InitStructure.GPIO_Pin = LCD_Pin_RESET;
    GPIO_Init(LCD_PORT_RESET, &GPIO_InitStructure);

    //LCD_Pin_RD
    GPIO_InitStructure.GPIO_Pin = LCD_Pin_BL;
    GPIO_Init(LCD_PORT_BL, &GPIO_InitStructure);
    
    //LCD_CS =1;
    //if(LCD_CS==0)
    //{
    //   LCD_WR_REG_DATA(0,0);
    //   LCD_ShowString(0,0," ");
    //   LCD_ShowNum(0,0,0,0);
    //   LCD_Show2Num(0,0,0,0);
    //   LCD_DrawPoint_big(0,0);
    //   LCD_DrawRectangle(0,0,0,0);
    //   Draw_Circle(0,0,0);
    // }

#if defined(ILI9325)

    Set_nRd();
    Set_nWr();
    Clr_nRst();
    OSTimeDlyHMSM(0,0,0,20);    //此处保证要有5ms以上才能保证复位稳定,推荐20ms
    Set_nRst(); 
    OSTimeDlyHMSM(0,0,0,20);    //此处保证要有5ms以上才能保证复位稳定,推荐20ms      
    Clr_nCs();                   //打开片选使能

    LCD_WR_REG_DATA(0x0001,0x0100); 
    LCD_WR_REG_DATA(0x0002,0x0700); 
#ifdef LCD_HORIZON_DISP_MODE
    LCD_WR_REG_DATA(0x0003,0x1018); 
#else
    LCD_WR_REG_DATA(0x0003,0x1030); 
#endif
    LCD_WR_REG_DATA(0x0004,0x0000); 
    LCD_WR_REG_DATA(0x0008,0x0207);  
    LCD_WR_REG_DATA(0x0009,0x0000);
    LCD_WR_REG_DATA(0x000A,0x0000); 
    LCD_WR_REG_DATA(0x000C,0x0000); 
    LCD_WR_REG_DATA(0x000D,0x0000);
    LCD_WR_REG_DATA(0x000F,0x0000);
   //power on sequence VGHVGL
    LCD_WR_REG_DATA(0x0010,0x0000);   
    LCD_WR_REG_DATA(0x0011,0x0007);  
    LCD_WR_REG_DATA(0x0012,0x0000);  
    LCD_WR_REG_DATA(0x0013,0x0000); 
   //vgh 
    LCD_WR_REG_DATA(0x0010,0x1290);   
    LCD_WR_REG_DATA(0x0011,0x0227);
    //delayms(100);
    //vregiout 
    LCD_WR_REG_DATA(0x0012,0x001d); //0x001b
    //delayms(100); 
    //vom amplitude
    LCD_WR_REG_DATA(0x0013,0x1500);
    //delayms(100); 
    //vom H
    LCD_WR_REG_DATA(0x0029,0x0018); 
    LCD_WR_REG_DATA(0x002B,0x000D); 
   
   //gamma
    LCD_WR_REG_DATA(0x0030,0x0004);
    LCD_WR_REG_DATA(0x0031,0x0307);
    LCD_WR_REG_DATA(0x0032,0x0002);// 0006
    LCD_WR_REG_DATA(0x0035,0x0206);
    LCD_WR_REG_DATA(0x0036,0x0408);
    LCD_WR_REG_DATA(0x0037,0x0507); 
    LCD_WR_REG_DATA(0x0038,0x0204);//0200
    LCD_WR_REG_DATA(0x0039,0x0707); 
    LCD_WR_REG_DATA(0x003C,0x0405);// 0504
    LCD_WR_REG_DATA(0x003D,0x0F02); 
    //ram
    LCD_WR_REG_DATA(0x0050,0x0000); 
    LCD_WR_REG_DATA(0x0051,0x00EF);
    LCD_WR_REG_DATA(0x0052,0x0000); 
    LCD_WR_REG_DATA(0x0053,0x013F);  
    LCD_WR_REG_DATA(0x0060,0xA700); 
    LCD_WR_REG_DATA(0x0061,0x0001); 
    LCD_WR_REG_DATA(0x006A,0x0000); 
    //
    LCD_WR_REG_DATA(0x0080,0x0000); 
    LCD_WR_REG_DATA(0x0081,0x0000); 
    LCD_WR_REG_DATA(0x0082,0x0000); 
    LCD_WR_REG_DATA(0x0083,0x0000); 
    LCD_WR_REG_DATA(0x0084,0x0000); 
    LCD_WR_REG_DATA(0x0085,0x0000); 
    //
    LCD_WR_REG_DATA(0x0090,0x0010); 
    LCD_WR_REG_DATA(0x0092,0x0600); 
    LCD_WR_REG_DATA(0x0093,0x0003); 
    LCD_WR_REG_DATA(0x0095,0x0110); 
    LCD_WR_REG_DATA(0x0097,0x0000); 
    LCD_WR_REG_DATA(0x0098,0x0000);
    LCD_WR_REG_DATA(0x0007,0x0133);

    Set_nCs();					 //disable cs
    
    Clr_nBL();
    
    OSTimeDlyHMSM(0,0,0,20);   // ylf: Delay Some TIME ??
	
#else

    Set_nCs();  
    Set_nRd();
    Set_nWr();
    Clr_nRst();
    OSTimeDlyHMSM(0,0,0,20);    //此处保证要有5ms以上才能保证复位稳定,推荐20ms
    Set_nRst(); 
    OSTimeDlyHMSM(0,0,0,20);    //此处保证要有5ms以上才能保证复位稳定,推荐20ms      
    Clr_nCs();  

    LCD_WR_REG(0xcf);  // ????b
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xc1);
    LCD_WR_DATA(0x30);
    
    LCD_WR_REG(0xed);   // ??????
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x12);
    LCD_WR_DATA(0x81);
    
    LCD_WR_REG(0xcb);  //????a
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x2c);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x02);
    
    LCD_WR_REG(0xea);  //??????b
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    
    LCD_WR_REG(0xe8);   //???????
    LCD_WR_DATA(0x85);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x79);
    
    LCD_WR_REG(0xC0); //Power control   ????1
    LCD_WR_DATA(0x23); //VRH[5:0]   4.6v
    
    LCD_WR_REG(0xC1); //Power control    ????2
    LCD_WR_DATA(0x11); //SAP[2:0];BT[3:0] 
    
    LCD_WR_REG(0xC2);   
    LCD_WR_DATA(0x11);
    
    LCD_WR_REG(0xC5); //VCM control  VCOM??1
    LCD_WR_DATA(0x3d);
    LCD_WR_DATA(0x30);
    
    LCD_WR_REG(0xc7);   //VCOM??2
    LCD_WR_DATA(0xaa);
    
    LCD_WR_REG(0x3A);  //COLMOD ??????
    LCD_WR_DATA(0x55);
    
    LCD_WR_REG(0x36); // Memory Access Control ???????
    LCD_WR_DATA(0x08);
    
    LCD_WR_REG(0xB1); // Frame Rate Control   ?????
    LCD_WR_DATA(0x00); 
    LCD_WR_DATA(0x11);
    
    LCD_WR_REG(0xB6); // Display Function Control ??????
    LCD_WR_DATA(0x0a);
    LCD_WR_DATA(0xa2);
    
    LCD_WR_REG(0xF2); // 3Gamma Function Disable   ??3G
    LCD_WR_DATA(0x00);
    
    LCD_WR_REG(0xF7);  //????
    LCD_WR_DATA(0x20);
    
    LCD_WR_REG(0xF1);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x30);
    
    LCD_WR_REG(0x26); //Gamma curve selected
    LCD_WR_DATA(0x01);
    
    LCD_WR_REG(0xE0); //Set Gamma
    LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x3f);
    LCD_WR_DATA(0x2f);
    LCD_WR_DATA(0x0c);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x0a);
    LCD_WR_DATA(0x53);
    LCD_WR_DATA(0xd5);
    LCD_WR_DATA(0x40);
    LCD_WR_DATA(0x0a);
    LCD_WR_DATA(0x13);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x00);
    
    LCD_WR_REG(0xE1); //Set Gamma
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x2c);
    LCD_WR_DATA(0xa2);
    LCD_WR_DATA(0x3f);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x0e);
    LCD_WR_DATA(0x0c);
    LCD_WR_DATA(0x37);
    LCD_WR_DATA(0x3c);
    LCD_WR_DATA(0x0F);

#ifdef LCD_HORIZON_DISP_MODE
    LCD_WR_REG(0x36);     //set the model of scanning
    LCD_WR_DATA((1<<5)|(0<<6)|(1<<7)|(1<<3)); //左横屏
    //LCD_WR_DATA((1<<5)|(1<<6));右横屏幕
    //LCD_WR_DATA(0x08); 竖屏幕
    LCD_WR_REG(0x2B);     //set the page address 横屏幕设置
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xEF);
    LCD_WR_REG(0x2A);    //set the column address
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x3F);
#endif		
    LCD_WR_REG(0x11); //Exit Sleep
    OSTimeDlyHMSM(0,0,0,200);//???????80ms????
    LCD_WR_REG(0x29); //display on

    Set_nCs();        //disable cs
    Clr_nBL();

#endif


}

void LCD_Clear(u16 Color)
{
    u16 i,j;
    
    Clr_nCs();                   //enable CS

    LCD_AddressSet(0,0,LCD_W-1,LCD_H-1);
    
    for(i = 0;i< LCD_W; i++)
    {
        for (j = 0; j < LCD_H; j++)
        {
            LCD_WR_DATA(Color);
        }
   
    }
    Set_nCs(); 
}

void LCD_ClearRect(u16 x1,u16 y1,u16 x2,u16 y2,u16 Color)
{
    u16 i,j;

    Clr_nCs();                   //enable CS

    LCD_AddressSet(x1,y1,x2,y2);
    
    for(i = x1;i<= x2; i++)
    {
        for (j = y1; j <= y2; j++)
        {
            LCD_WR_DATA(Color);
        }
   
    }
    Set_nCs(); 
}


#if 0
void showhanzi(unsigned int x,unsigned int y,unsigned char index)   
{  
    unsigned char i,j;
    unsigned char *temp=hanzi;    
    LCD_AddressSet(x,y,x+31,y+31); //设置区域      
    temp+=index*128;    
    for(j=0;j<128;j++)
    {
        for(i=0;i<8;i++)
        {            
            if((*temp&(1<<i))!=0)
            {
                LCD_WR_DATA(POINT_COLOR);
            } 
            else
            {
                LCD_WR_DATA(BACK_COLOR);
            }   
        }
        temp++;
     }
}
#endif
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
    LCD_AddressSet(x,y,x,y);//设置光标位置 
    LCD_WR_DATA(color);         
}    

void LCD_DrawPoint_big(u16 x,u16 y,u16 color)
{
    LCD_Fill(x-1,y-1,x+1,y+1,color);
} 


void LCD_SetPoint(u16 x,u16 y,u16 color)
{
  //Clr_nCs;
  if (x >= LCD_W)  x = LCD_W -1;
  if (y >= LCD_H)  y = LCD_H -1;
  
#ifdef LCD_HORIZON_DISP_MODE
  LCD_AddressSet(x,y,x,y);//设置光标位置 
#else
  LCD_SetCursor(x,y);
#endif
  LCD_WR_DATA(color);  
  
  //Set_nCs;
}    

void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
    u16 i,j; 

    Clr_nCs();                  
    LCD_AddressSet(xsta,ysta,xend,yend);      
    for(i=ysta;i<=yend;i++)
    {                                                           
        for(j=xsta;j<=xend;j++)
        {      
            LCD_WR_DATA(color);
        }
    }   
    Set_nCs();                  
    
}  

void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
    u16 t; 
    int xerr=0,yerr=0,delta_x,delta_y,distance; 
    int incx,incy,uRow,uCol; 
    Clr_nCs();                  

    delta_x=x2-x1; //计算坐标增量 
    delta_y=y2-y1; 
    uRow=x1; 
    uCol=y1; 
    if(delta_x>0)incx=1; //设置单步方向 
    else if(delta_x==0)incx=0;//垂直线 
    else {incx=-1;delta_x=-delta_x;} 
    if(delta_y>0)incy=1; 
    else if(delta_y==0)incy=0;//水平线 
    else{incy=-1;delta_y=-delta_y;} 
    if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
    else distance=delta_y; 
    for(t=0;t<=distance+1;t++ )//画线输出 
    {  
        LCD_DrawPoint(uRow,uCol,color);//画点 
        xerr+=delta_x ; 
        yerr+=delta_y ; 
        if(xerr>distance) 
        { 
            xerr-=distance; 
            uRow+=incx; 
        } 
        if(yerr>distance) 
        { 
            yerr-=distance; 
            uCol+=incy; 
        } 
    }  
    
    Set_nCs();                  
    
}  

//画矩形
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
    LCD_DrawLine(x1,y1,x2,y1,color);
    LCD_DrawLine(x1,y1,x1,y2,color);
    LCD_DrawLine(x1,y2,x2,y2,color);
    LCD_DrawLine(x2,y1,x2,y2,color);
}

void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
    int a,b;
    int di;
    a=0;b=r;      
    di=3-(r<<1);             //判断下个点位置的标志
    while(a<=b)
    {
        LCD_DrawPoint(x0-b,y0-a,color);             //3           
        LCD_DrawPoint(x0+b,y0-a,color);             //0           
        LCD_DrawPoint(x0-a,y0+b,color);             //1       
        LCD_DrawPoint(x0-b,y0-a,color);             //7           
        LCD_DrawPoint(x0-a,y0-b,color);             //2             
        LCD_DrawPoint(x0+b,y0+a,color);             //4               
        LCD_DrawPoint(x0+a,y0-b,color);             //5
        LCD_DrawPoint(x0+a,y0+b,color);             //6 
        LCD_DrawPoint(x0-b,y0+a,color);             
        a++;
        //使用Bresenham算法画圆     
        if(di<0)di +=4*a+6;   
        else
        {
            di+=10+4*(a-b);   
            b--;
        } 
        LCD_DrawPoint(x0+a,y0+b,color);
    }
} 
#if 0

u16 BACK_COLOR = WHITE, POINT_COLOR = BLACK;   //背景色，画笔色

extern  u16 BACK_COLOR, POINT_COLOR;   //背景色，画笔色

//在指定位置显示一个字符
//num:要显示的字符:" "--->"~"
//mode:叠加方式(1)还是非叠加方式(0)
//在指定位置显示一个字符
//num:要显示的字符:" "--->"~"
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 mode)
{
    u8 temp;
    u8 pos,t;
    u16 x0=x;
    u16 colortemp = POINT_COLOR;      
    if(x>LCD_W-16||y>LCD_H-16)return;   

    Clr_nCs();                  
    
    //设置窗口         
    num=num-' ';//得到偏移后的值
    LCD_AddressSet(x,y,x+8-1,y+16-1);      //设置光标位置 
    if(!mode) //非叠加方式
    {
        for(pos=0;pos<16;pos++)
        { 
            temp = asc2_1608[(u16)num*16+pos];       //调用1608字体
            for(t=0;t<8;t++)
            {                 
                if(temp&0x01)POINT_COLOR=colortemp;
                else POINT_COLOR=BACK_COLOR;
                LCD_WR_DATA(POINT_COLOR);   
                temp>>=1; 
                x++;
            }
            x=x0;
            y++;
        }   
    }else//叠加方式
    {
        for(pos=0;pos<16;pos++)
        {
            temp = asc2_1608[(u16)num*16+pos];       //调用1608字体
            for(t=0;t<8;t++)
            {                 
                if(temp&0x01)LCD_DrawPoint(x+t,y+pos,POINT_COLOR);//画一个点     
                temp>>=1; 
            }
        }
    }
    POINT_COLOR=colortemp;    
    Set_nCs();                  
    
}  
//显示2个数字
//x,y :起点坐标  
//len :数字的位数
//color:颜色
//num:数值(0~4294967295);   
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len)
{           
    u8 t,temp;
    u8 enshow=0;
    
    num=(u16)num;
    for(t=0;t<len;t++)
    {
        temp=(num/mypow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                LCD_ShowChar(x+8*t,y,' ',0);
                continue;
            }else enshow=1; 
             
        }
        LCD_ShowChar(x+8*t,y,temp+48,0); 
    }
    
} 
//显示2个数字
//x,y:起点坐标
//num:数值(0~99);    
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len)
{           
    u8 t,temp;                         
    for(t=0;t<len;t++)
    {
        temp=(num/mypow(10,len-t-1))%10;
        LCD_ShowChar(x+8*t,y,temp+'0',0); 
    }
} 

//显示字符串
//x,y:起点坐标  
//*p:字符串起始地址
//用16字体
void LCD_ShowString(u16 x,u16 y,const u8 *p)
{       


    while(*p != '\0')
    {       
        if(x > LCD_W-16) // move to next line
        {
            x = 0; 
            y += 16;
        }
        
        if(y > LCD_H-16)
        {
            y = x = 0;
            LCD_Clear(RED);
        }
        
        LCD_ShowChar(x,y,*p,0);
        x+=8;
        p++;
    }  
    
}
#endif

void LCD_PutChar8x16(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
{
    u16 i=0;
    u16 j=0;
    
    u8 tmp_char=0;

    Clr_nCs();                  
  
    for (i = 0; i < 16; i++) // YLF: 16 ROWS
    {
        //tmp_char = asc2_1608[((c-0x20)*16)+i];
        tmp_char = ascii_8x16[((c-0x20)*16)+i];        
        for (j = 0;j < 8;j++)
        {
            if ( (tmp_char >> 7-j) & 0x01 == 0x01)
            {
              LCD_SetPoint(x+j,y+i,charColor); // 字符颜色
            }
            else
            {
              LCD_SetPoint(x+j,y+i,bkColor); // 背景颜色
            }
        }
    }
    
    Set_nCs();                  
}


void LCD_PutString8x16(u16 x,u16 y,u8 *string,u16 charColor,u16 bkColor)
{
    while(*string != '\0')
    {       
        if(x > LCD_W-8) // move to next line
        {
            //x = 0; 
            //y += 16;
            
            break;
        }
        
        if(y > LCD_H-16)
        {
            //y = x = 0;
            //LCD_Clear(RED);
            
            break;
        }
        
        LCD_PutChar8x16(x,y,*string,charColor,bkColor);
        x += 8;
        string++;
    }  
                  
}

/*****************************************************************************************
Function: calculate text size
input : texgt
output : sx  size for x cord
            sy size for y cord
******************************************************************************************/
void LCD_GetStringGB8x16TextSize(u8 *s, int* sx, int* sy) 
{
    int offset = strlen((char *)s);

    *sx = offset*8; 
    *sy = offset*18; 
}


void LCD_PutChar12x24(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
{
    u16 i=0;
    u16 j=0;
    
    u16 tmp_char=0;

    Clr_nCs();                  
  
    for (i = 0; i < 24; i++) // YLF: 16 ROWS
    {
        tmp_char = (ASCII_12x24[(c-0x20)*48 + 2*i] << 8)|ASCII_12x24[(c-0x20)*48 + 2*i+1];        
        for (j = 0;j < 12;j++)
        {
            if ( tmp_char & 0x8000)
            {
              LCD_SetPoint(x+j,y+i,charColor); // 字符颜色
            }
            else
            {
              LCD_SetPoint(x+j,y+i,bkColor); // 背景颜色
            }
            tmp_char <<= 1;
        }
    }
    
    Set_nCs();                  
}

void LCD_PutString12x24(u16 x,u16 y,char *string,u16 charColor,u16 bkColor)
{
    while(*string != '\0')
    {       
        if(x >= LCD_W - 12) // move to next line
        {
            //x  = 0; 
            //y += 32;
            break;
        }
        
        if(y >= LCD_H - 24)
        {
            //y = x = 0;
            //LCD_Clear(RED);
            break;
        }
        
        LCD_PutChar12x24(x,y,*string,charColor,bkColor);
        x += 12;
        string++;
    }  
                  
}

#define pgm_read_byte(p)(*p)

#if defined(ILI9325)
void LCD_PutChar24x48(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
{
    u16 i=0;
    u16 j=0;
    
    u8 m = 0;
  
    const u8 *pdata = &ascii_24x48[(c-0x20)*144];
    
    Clr_nCs();                  
    
    LCD_AddressSet(x,y,x+24-1,y+48-1);  
    
    for(i  =0; i < 144; i++)
    {
        m = pgm_read_byte(&pdata[i]);//提取c字符的第i个字节以,c减去0x20是由于Ascii码库中的0~1f被去掉
        for(j=0;j<8;j++)
        {                       
            if((m&0x80)==0x80) 
            {                                     //判断最高位是否为1
                LCD_WR_DATA(charColor);           //最高位为1，写入字符颜色
            }
            else 
            {
                LCD_WR_DATA(bkColor);           //最高位为0，写入背景颜色
            }
            m<<=1;                                //左移1位，准备写下一位
        }
    }
    Set_nCs();                  
}

#else
/*20170208,放大产水水质字体，修改LCD_PutChar24x48驱动函数*/
void LCD_PutChar24x48(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
{
    u16 i=0;
    u16 j=0;
    
    u8 m = 0;
  
    const u8 *pdata = &ascii_24x48[(c-0x20)*144];
    
    Clr_nCs();                  
    
    //LCD_AddressSet(x,y,x+24-1,y+48-1);  
    
    for(i  =0; i < 48; i++)
    {
        //m = pgm_read_byte(&pdata[i]);//提取c字符的第i个字节以,c减去0x20是由于Ascii码库中的0~1f被去掉
        int col;
    	for (col = 0; col < 3; col ++)
    	{
            m = pgm_read_byte(&pdata[i*3+col]);
            for(j=0;j<8;j++)
            {                       
                if((m&0x80)==0x80) 
                {                                    
                   LCD_SetPoint(x+j+col*8,y+i,charColor);
                }
                else 
                {
                    LCD_SetPoint(x+j+col*8,y+i,bkColor);
                }
                m<<=1;                                //左移1位，准备写下一位
            }
        }
    }
    Set_nCs();                  
}
#endif

void LCD_PutString24x48(u16 x,u16 y,char *string,u16 charColor,u16 bkColor)
{
    while(*string != '\0')
    {       
        if(x >= LCD_W-24) // move to next line
        {
            //x  = 0; 
            //y += 32;
            break;
        }
        
        if(y >= LCD_H-48)
        {
            //y = x = 0;
            //LCD_Clear(RED);
            break;
        }
        
        LCD_PutChar24x48(x,y,*string,charColor,bkColor);
        x += 24;
        string++;
    }  
                  
}



#ifdef FONT16x33
void LCD_PutChar16x33(unsigned short x, unsigned short y, unsigned char c, unsigned int fColor, unsigned int bColor)
{
   unsigned int i,j,m;
   Clr_nCs();                  
   
   LCD_AddressSet(x,y,x+16-1,y+33-1);                    //设置字符显示位置
   for(i=0;i<66;i++)
   {                                               //循环写入16字节，一个字符为16字节
        m = pgm_read_byte(&Font16x33[c*66+i]);      //提取c字符的第i个字节以,c减去0x20是由于Ascii码库中的0~1f被去掉
        for(j=0;j<8;j++) 
        {                        //循环写入8位，一个字节为8位
            if((m&0x80)==0x80) 
            {                  //判断最高位是否为1
                LCD_WR_DATA(fColor);           //最高位为1，写入字符颜色
            }
            else 
            {
                LCD_WR_DATA(bColor);           //最高位为0，写入背景颜色
            }
            m<<=1;                                //左移1位，准备写下一位
        }
   }
   Set_nCs();                  
}

void LCD_PutChar24x48(unsigned short x, unsigned short y, unsigned char c, unsigned int fColor, unsigned int bColor)
{
    unsigned int i,j,m;
    Clr_nCs();                  
    LCD_AddressSet(x,y,x+24-1,y+48-1);                    //设置字符显示位置
    for(i=0;i<144;i++) 
    {                             //循环写入16字节，一个字符为16字节
        m=pgm_read_byte(&Font24x48[c*144+i]);//提取c字符的第i个字节以,c减去0x20是由于Ascii码库中的0~1f被去掉
        for(j=0;j<8;j++)
        {                        //循环写入8位，一个字节为8位
            if((m&0x80)==0x80) 
            {                  //判断最高位是否为1
                LCD_WR_DATA(fColor);           //最高位为1，写入字符颜色
            }
            else 
            {
                LCD_WR_DATA(bColor);           //最高位为0，写入背景颜色
            }
            m<<=1;                                //左移1位，准备写下一位
        }
    }
    
    Set_nCs();                  
}
#endif

/*
void PutGB16x22(unsigned short x, unsigned short  y, unsigned char c[2], unsigned int fColor,unsigned int bColor){
    unsigned int i,j,k;

    LCD_SetPos(y,  y+22-1,x, x+16-1);

for (k=0;k<20;k++) { //11表示自建汉字库中的个数，循环查询内码
  if ((pgm_read_byte(&codeGB_22[k].Index[0])==c[0])&&(pgm_read_byte(&codeGB_22[k].Index[1])==c[1])){
        for(i=0;i<44;i++) {
         unsigned short m=pgm_read_byte(&codeGB_22[k].Msk[i]); 
          for(j=0;j<8;j++) {
            if((m&0x80)==0x80) {
                Write_Data_U16(fColor);
                }
              else {
                Write_Data_U16(bColor);
                }
            m<<=1;
            }
          }
        }
      }
}
*/
#if 0
void PutGB24x48(unsigned short x, unsigned short  y, unsigned char c[2], unsigned int fColor,unsigned int bColor)
{
    unsigned int i,j,k;
    Clr_nCs();                  

    LCD_AddressSet(x, y,x+24-1,y+48-1);

    for (k=0;k<GetcodeGB_49_LENGTH();k++) //11表示自建汉字库中的个数，循环查询内码
    { 
        if ((pgm_read_byte(&codeGB_49[k].Index[0])==c[0])
             &&(pgm_read_byte(&codeGB_49[k].Index[1])==c[1]))
        {
            for(i=0;i<144;i++) 
            {
                 unsigned char m = pgm_read_byte(&codeGB_49[k].Msk[i]); 
                 for(j=0;j<8;j++) 
                 {
                     if((m&0x80)==0x80) 
                     {
                         LCD_WR_DATA(fColor);
                     }
                     else 
                     {
                         LCD_WR_DATA(bColor);
                     }
                     m<<=1;
                 }
            }
            break;
         }
    }
    Set_nCs();                  

}
#endif
//-----------------显示32X32的汉字，fColor为字颜色，bColor为底颜色----------------------------//

#ifdef CODEGB_32

void PutGB3232(u16 x, u16  y, unsigned char c[2], u16 fColor,u16 bColor)
{
    unsigned int i,j,k;

    Clr_nCs();                  
    LCD_AddressSet(x,y,x+32-1,y+32-1);

    for (k = 0;k< GetcodeGB_32_LENGTH();k++) // //15标示自建汉字库中的个数，循环查询内码
    { 
      if ((pgm_read_byte(&codeGB_32[k].Index[0]) == c[0])
          &&(pgm_read_byte(&codeGB_32[k].Index[1]) ==c[1]))
      {  
          for(i = 0; i< 128; i++) 
          {                     
              unsigned char m = pgm_read_byte(&codeGB_32[k].Msk[i]);
              for(j=0;j<8;j++)
              {
                  if((m&0x80)==0x80) 
                  {
                      LCD_WR_DATA(fColor);
                  }
                  else
                  {
                      LCD_WR_DATA(bColor);
                  }
                  m<<=1;
              } 
           }
           break;
        }  
    }
    
    Set_nCs();                  
}

void PutStringGB3232(u16 x, u16 y, u8 *s, u16 fColor, u16 bColor)
{
    unsigned char l=0;
    while(*s) 
    {
        if( *s < 0x80) 
        {
          //LCD_PutChar(y+l*8,x,*s,fColor,bColor);
          s++;l++;
        }
        else
        {
            PutGB3232(x+l*16,y,(unsigned char*)s,fColor,bColor);
            s+=2;
            l+=2;
        }
    }
}

void LCD_GetStringGB32x32TextSize(u8 *s, int* sx, int* sy) 
{
    int offset = strlen((char *)s);

    *sx = offset*16; 
    *sy = offset*32; 
}


#endif

//-----------------显示32X32的汉字，fColor为字颜色，bColor为底颜色----------------------------//
#ifdef CODEGB_48

void PutGB4848(u16 x, u16  y, u8 c[2], u16 fColor,u16 bColor)
{
    unsigned int i,j,k;

    Clr_nCs();                  
    LCD_AddressSet(x,y,x+48-1,y+48-1);

    for (k = 0; k < GetcodeGB_48_LENGTH(); k++) //15标示自建汉字库中的个数，循环查询内码
    { 
        if ((pgm_read_byte(&codeGB_48[k].Index2[0]) == c[0])
           &&(pgm_read_byte(&codeGB_48[k].Index2[1]) == c[1]))
        {    
            for(i=0;i<288;i++) 
            {                     
                unsigned char m =pgm_read_byte(&codeGB_48[k].Msk2[i]);
                for(j=0;j<8;j++) 
                {
                    if((m&0x80)==0x80) 
                    {
                        LCD_WR_DATA(fColor);
                    }
                    else 
                    {
                        LCD_WR_DATA(bColor);
                    }
                    m<<=1;
                } 
            }
            break;
        }  
    }
  
    Set_nCs();                  
}

void PutStringGB4848(u16 x, u16 y, u8 *s, u16 fColor, u16 bColor)
{
    unsigned char l=0;
    while(*s) 
    {
        if( *s < 0x80) 
        {
          LCD_PutChar24x48(x + l*24,y,*s,fColor,bColor);
          s++;l++;
        }
        else
        {
            PutGB4848(x + l*24,y,(unsigned char*)s,fColor,bColor);
            s+=2;l+=2;
         }
    }
}
void LCD_GetStringGB48x48TextSize(u8 *s, int* sx, int* sy) 
{
    int offset = strlen((char *)s);

    *sx = offset*24; 
    *sy = offset*48; 
}


#endif
/***********************************************************************************************************
函数功能: 显示汉字24x24
输入参数：x 横坐标
          y 纵坐标
          c 需要显示的汉字码
          fColor 字符颜色
          bColor 字符背景颜色
***********************************************************************************************************/
void PutGB2424(u16 x, u16  y, u8 c[2], u16 fColor,u16 bColor)
{
    unsigned int i,j,k;

    Clr_nCs();   

    for (k = 0;k<GetcodeGB_24_LENGTH();k++) 
    { 
    	if ((pgm_read_byte(&codeGB_24[k].Index[0]) == c[0])
    	  &&(pgm_read_byte(&codeGB_24[k].Index[1]) == c[1]))
    	{
    	
    		for(i = 0; i < 24; i++) 
    		{	 
    			 int col;
    			 for (col = 0; col < 3; col ++)
    			 {
    				 unsigned char m = pgm_read_byte(&codeGB_24[k].Msk[i*3 + col]); 
    				 
    				 for(j=0;j<8;j++) 
    				 {
    					 if((m&0x80)==0x80) 
    					 {
    						 LCD_SetPoint(x+j+col*8,y+i,fColor); // 字符颜色
    					 }
    					 else
    					 {
    						 LCD_SetPoint(x+j+col*8,y+i,bColor); // 背景颜色
    					 }
    					 m<<=1;
    				 }
    			 }
    		}
    		break;
    	}
    }

    Set_nCs();                  
}

/*****************************************************************************************
函数功能:显示字符串,可以中英文同时显示
输入参数：x 横坐标
          y 纵坐标
         *s 需要显示的字符串
      fColor 字符颜色
          bColor 字符背景颜色
******************************************************************************************/

void LCD_PutStringGB24x24(u16 x, u16 y, u8 *s, u16 fColor, u16 bColor) 
{
    int offset = 0;
    while(*s) 
    {
        if( *s < 0x80) 
        {
            LCD_PutChar12x24(x + offset,y,*s,fColor,bColor);
            offset += 12;
            s++;
        }
        else
        {
            PutGB2424(x + offset,y,(unsigned char*)s,fColor,bColor);
            s += 2;
            offset += 24;
            
        }
    }
}

/*****************************************************************************************
Function: calculate text size
input : texgt
output : sx  size for x cord
            sy size for y cord
******************************************************************************************/
void LCD_GetStringGB24x24TextSize(u8 *s, int* sx, int* sy) 
{
    int offset = strlen((char *)s);

    *sx = offset*12; 
    *sy = offset*24; 
}


//m^n函数
u32 mypow(u8 m,u8 n)
{
    u32 result=1;    
    while(n--)result*=m;    
    return result;
}            


void LCD_EnableBackLight(int bEnable)
{
    if (bEnable)
    {
        Clr_nBL();
    }
    else
    {
        Set_nBL();
    }
}

int LCD_GetBackLightState(void)
{
    return Get_nBL();
}

#if 0
void LCD_PutHZ(u16 x,u16 y,u16 c,u16 charColor,u16 bkColor)
{
    u16 i=0;
    u16 j=0;
    int pos;
    u8 High8bit,Low8bit;

    u16 tmp_char = 0;

    High8bit = (c>>8);
    Low8bit  = (c&0x00FF);

    pos = 32*((High8bit-0xb0)*94+Low8bit-0xa1);

    Clr_nCs();                  
  
    for (i = 0; i < 16; i++) // YLF: 16 ROWS
    {
        tmp_char = (HzLib[pos + 2*i] << 8) | HzLib[pos + 2*i + 1];
        
        for (j = 0;j < 16;j++)
        {
            if ( (tmp_char & (1 << 15)) != 0x00)
            {
              LCD_SetPoint(x+j,y+i,charColor); // 字符颜色
            }
            else
            {
              LCD_SetPoint(x+j,y+i,bkColor); // 背景颜色
            }
            tmp_char <<= 1;
        }
    }
    
    Set_nCs();                  
}

void LCD_PutHZString(u16 x,u16 y,char *string,u16 charColor,u16 bkColor)
{
    while(*string != '\0')
    {       
        if(x > LCD_W-16) // move to next line
        {
            x = 0; 
            y += 16;
        }
        
        if(y > LCD_H-16)
        {
            y = x = 0;
            LCD_Clear(RED);
        }
        
        LCD_PutHZ(x,y,(*string << 8)|(*(string+1)) ,charColor,bkColor);
        x += 16;
        string += 2;
    }  
                  
}
#endif

