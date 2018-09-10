#ifndef _COPYRIGHT_H_
#define _COPYRIGHT_H_

enum CP_RESOURCE_TYPE
{
    CP_RESOURCE_GENERAL = 0,

    CP_RESOURCE_RTC ,

    CP_RESOURCE_CAN,

    CP_RESOURCE_GPIO,

    CP_RESOURCE_TMR,

    CP_RESOURCE_NUM,
};

#ifdef CP_DEMO

int CopyRight_Increase(int type);
int CopyRight_return_type(int *ret_value,const char* fmt,...);
#define COPYRIGHT_CHECK_RETURN(type,fmt, args...) \
do {int ret_value;\
    int ret = CopyRight_Increase(type);\
    int type = CopyRight_return_type(&ret_value,fmt,##  args);\
    if (ret)\
    {\
        if (type)return ret_value;\
    }\
}while(0)
#define COPYRIGHT_CHECK_VOID(type) \
do {int ret = CopyRight_Increase(type);\
    if (ret)\
    {\
        return;\
    }\
}while(0)
#else
#define COPYRIGHT_CHECK_VOID(type)
#define COPYRIGHT_CHECK_RETURN(type,fmt, args...)
#endif

#endif
