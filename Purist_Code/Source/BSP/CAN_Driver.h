#ifndef _CAN_DRIVER_H_
#define _CAN_DRIVER_H_
void STM_EVAL_CANInit(CAN_InitTypeDef* CAN_InitStruct);
uint8_t CANSendMsgNoWait(uint16_t Identifier, uint8_t* Msg, uint8_t MsgSize );

#endif
