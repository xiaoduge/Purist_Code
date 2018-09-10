#ifndef _AD7799_H_
#define _AD7799_H_

/**
  ******************************************************************************
  * @file    ad7799.h 
  * @author  STM32 Team
  * @version V1.0.0
  * @date    10/10/2014
  * @brief   Interface for AD7799 Driver access.
  ******************************************************************************
  *          DO NOT MODIFY ANY OF THE FOLLOWING CODE EXCEPT UNDER PERMISSION OF ITS PROPRIETOR
  * @copy
  *
  *
  * <h2><center>&copy; COPYRIGHT 2012 Shanghai ZhenNeng Corporation</center></h2>
  */ 

uint8_t AD7799_Init(uint8_t Gain);
void AD7799_Start(uint8_t CovChx,uint8_t CovGain,uint8_t CovRate,uint8_t CovMode);
uint32_t AD7799_Read(void);  

#endif
