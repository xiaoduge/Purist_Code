#ifndef _CM_INTERFACE_H_
#define _CM_INTERFACE_H_


typedef enum   //app ЦёБо
{
    RF_CMD_ACCESS = 0,
    RF_CMD_CONFIG ,
    RF_CMD_NTW  ,
    RF_CMD_QTW	,
    RF_CMD_CIRCULAION,
    RF_CMD_DEPRESS	,
    RF_CMD_RESET_CM ,
    RF_CMD_CM ,
    RF_CMD_CONDUCTIVITY ,
    RF_CMD_HISTORY ,
    RF_CMD_USER_HIS ,
    RF_CMD_STATUS ,
    RF_CMD_KEY ,
    RF_CMD_RESTORE_FACTORY_CFG ,
    RF_CMD_NUM,
}RF_REMOTE_CMD_ENUM;

typedef enum
{
    CM_TYPE_PACK   = 0X0,
    CM_TYPE_FILTER ,
    CM_TYPE_UV ,
	CM_TYPE_NUM,
}CM_TYPE_ENUM;

#endif
