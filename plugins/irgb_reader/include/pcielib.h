#ifndef PCIELIB_H
#define PCIELIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch36x_lib.h"
#include <math.h>

#define MAX_PCIE_NUM  5

typedef enum{
    MODE_GPS   = 0,
    MODE_BD,
}GNSS_MODE;

typedef struct
{
    GNSS_MODE        mode;          //GNSS模式，0表示GPS，1表示北斗
    unsigned int     nVisual;       //可见卫星数量
    unsigned int     nUsed;         //可用卫星数量
    int              nLongi;        //经度(*105取整)，正数表示东经，负数表示西经
    int              nLati;         //纬度(*105取整)，正数表示北纬，负数表示南纬
    int              nHigh;         //海拔（取整，单位为米）
}GNSS_Msg;

typedef struct
{
    unsigned int     Ns;             //纳秒
    unsigned char    Zone;           //时区
    unsigned char    Sec;            //秒
    unsigned char    Min;            //分
    unsigned char    Hour;           //时
    unsigned char    Day;            //日
    unsigned char    Month;          //月
    unsigned short   Year;           //年
    unsigned char    LeapSec;        //闰秒
}Time_Msg;

typedef struct
{    //e.g. 192.168.1.11
    unsigned char    field1;          //IP字段1，如：192
    unsigned char    field2;          //IP字段2，如：168
    unsigned char    field3;          //IP字段3，如：1
    unsigned char    field4;          //IP字段4，如：11
}ip_stru;


typedef struct
{
    ip_stru    ip_addr;          //IP地址
    ip_stru    ip_mask;          //子网掩码
    ip_stru    ip_gate;          //网关
}IP_Msg;

typedef struct
{
    unsigned char    profile;        //0:三层单播，1：三层组播
    unsigned char    domain;         //0~255
    char             slave_int;      //0、-1、-2、-3、-4、-5、-6、-7；
    char             master_int;     // 预留
    ip_stru          serv_ip;      	 //单播模式服务器IP
    short            send_delay;  	 //发送延时：-32768~32767
    short            rec_delay;  	 //接收延时：-32768~32767
}PTPin_Msg;

typedef struct
{
    unsigned char    profile;        //0:三层单播，1：三层组播
    unsigned char    domain;         //0~255
    char             master_int;     //0、-1、-2、-3、-4、-5、-6、-7；
}PTPout_Msg;


typedef struct
{
    unsigned char    syncState;          //1:自由运行，2：保持，3：快捕，4：锁定 0:未知状态
   int               syncAccu;         		   //同步精度，反映板卡与参考的同步状态，值越接近0效果越好
   unsigned char    ref_sta[3];         // ref_sta[0~2]依次表示GNSS、B码、PTP三种参考的状态，值为1表示有效，值为0表示无效；
   unsigned char    curRef;            //系统当前所用参考类型，1:GNSS，2：B码，3：PTP;
   unsigned char    anteState;         //当前GNSS天线状态检测，0：未知状态，1:正常连接，2：天线短路，3：天线断开
   unsigned char    flag;              //1:ok 0:failed
}BoardState_Msg;


typedef struct
{
    int   	Year;            //年
    int   	Month;          //月
    int    	Day;            //日
    int    	Hour;           //时
    int    	Min;            //分
    int    	Sec;            //秒
    int    	us;             //微秒
}TIME,*PTIME;
/**************************************************************************************
Function      : Get_DllVersion
Description   :获取固件版本信息
Input         :   char *ver表示dll的版本信息
Return        :
Others        :
**************************************************************************************/
void Get_DllVersion (char *ver);



/**************************************************************************************
Function      : Open_Device
Description   :打开指定设备
Input         : index要打开的设备编号，第一个设备为0，第二个为1...
Return        :0成功 -1失败
Others        :
**************************************************************************************/
int Open_Device (int index);


/**************************************************************************************
Function      : Close_Device
Description   :打开指定设备
Input         : index要关闭的设备编号，第一个设备为0，第二个为1...
Return        :0成功 -1失败
Others        :
**************************************************************************************/
int Close_Device (int index);


/**************************************************************************************
Function      : Update_SystemTime
Description   :更新操作系统时间
Input         : index设备编号，第一个设备为0，第二个为1...
Return        :0成功 -1失败 -2:未锁定
Others        :
**************************************************************************************/
int Update_SystemTime (int index);


/**************************************************************************************
Function      : SetIntRoutine
Description   :设置回调函数
Input         : index设备编号，第一个设备为0，第二个为1...
Return        :
Others        :
**************************************************************************************/
void SetIntRoutine(int index, void *isr_handler );//指定中断服务程序,为NULL则取消中断服务,否则在中断时调用该程序


/**************************************************************************************
Function      : Init_System
Description   :系统初始化配置
Input         :
Return        : 返回值为配置结果:
                0表示配置成功，-1表示配置失败。
Others        : 设备操作相关的初始化，参数相关的初始化
**************************************************************************************/
int Init_System (void);


/**************************************************************************************
Function      : Get_Sys_Status
Description   :获取系统状态
Input         : int *pSyncAccuacy：表示同步精度的指针。
                int *sysStatus 1表示自由运行，2表示保持，3表示快捕，4表示锁定，0表示未知状态。
Return        : 返回值为系统状态:
                0表示成功，-1表示失败。
Others        : 入参指针包含传回的同步精度值，为有符号整型数值。
**************************************************************************************/
int Get_Sys_Status(int index,int *sysStatus,int *pSyncAccuacy);


/**************************************************************************************
Function      : Get_Ref_Type
Description   :获取当前参考源类型
Input         : int *type：表示当前参考类型，0:未知 1表示GNSS，2表示B码，3表示PTP。
                int *isOk  表示所用参考源状态 1表示参考有效， 0表示参考无效。
Return        : 0表示成功，-1表示失败。
Others        : 入参指针包含传回的参考类型
**************************************************************************************/
int Get_Ref_Type (int index,int *type,int *isOk);


/**************************************************************************************
Function      : Get_RefStatus
Description   :获取参考源工作状态
Input         : int type：1表示GNSS，2表示B码，3表示PTP。
                int *status 1表示参考有效， 0表示参考无效。
Return        : 0表示成功，-1表示失败。
Others        :
**************************************************************************************/
int Get_Ref_Status (int index,int type,int *status);


/**************************************************************************************
Function      : Get_Antenna_Status
Description   :获取天线工作状态及天线延迟
Input         : int * pOffset：表示天线延迟的指针。
                int *status 天线状态: 1表示正常，2表示短路，3表示开路，0表示未知状态。
Return        : 0表示成功，-1表示失败。
Others        : 入参指针包含传回的天线延迟值，为有符号整型数值。
**************************************************************************************/
int Get_Antenna_Status(int index,int *pOffset,int *status);


/**************************************************************************************
Function      : Set_Antenna_Offset
Description   :设置天线延迟
Input         : int nOffset：表示待设置的天线延迟值。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        :天线延迟值为有符号整型数值。
**************************************************************************************/
int Set_Antenna_Offset (int index,int nOffset);


/**************************************************************************************
Function      : Get_GNSS_Status
Description   :获取GNSS状态信息
Input         : GNSS_Msg *msg：表示状态的指针。
Return        : 返回值为查询结果:
                0表示查询成功，-1表示查询失败。
Others        : 入参指针包含传回的状态信息，具体参见结构体GNSS_Msg定义。
**************************************************************************************/


int Get_GNSS_Status(int index,GNSS_Msg *msg);


/**************************************************************************************
Function      : Get_GNSS_Valid
Description   :查询GNSS是否有效
Input         :
Return        : 返回值为查询结果:
                0表示无信号/无效，1表示有信号/有效。
Others        :
**************************************************************************************/
int Get_GNSS_Valid(int index);

/**************************************************************************************
Function      : Set_GNSS_Mode
Description   :设置GNSS模式
Input         : char mode：表示待设置的模式值，0表示GPS，1表示北斗。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        :
**************************************************************************************/
int Set_GNSS_Mode (int index,GNSS_MODE mode);


/**************************************************************************************
Function      : Get_Sys_Time
Description   :获取系统时间信息
Input         : Time_Msg *msg：表示时间信息的指针。
Return        : 返回值为查询结果:
                0表示查询成功，-1表示查询失败。
Others        : 入参指针包含传回的时间信息，具体参见结构体Time_Msg定义。
**************************************************************************************/
int Get_Sys_Time (int index,Time_Msg *msg);


/**************************************************************************************
Function      : Set_Time_Zone
Description   :设置系统时区
Input         : char zone：表示待设置的时区值，范围-12~12，步进为1。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        :
**************************************************************************************/
int Set_Time_Zone (int index,char zone);

/**************************************************************************************
Function      : Get_Time_Zone
Description   :获取系统时区
Input         : char *zone：表示获取到的时区值，范围-12~12，步进为1。
Return        : 返回操作状态值:
                0表示成功，-1表示失败。
Others        :
**************************************************************************************/
int Get_Time_Zone (int index,char *zone);


/**************************************************************************************
Function      : Set_Leap_Second
Description   :设置闰秒时间
Input         : unsigned char：表示待设置的闰秒值。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        : 此处为GPS时间的闰秒。
**************************************************************************************/
int Set_Leap_Second (int index,unsigned char leap);


/**************************************************************************************
Function      : Get_Leap_Second
Description   :获取闰秒时间
Input         : unsigned char*：表示获取到的闰秒值。
Return        : 返回操作状态值:
                0表示成功，-1表示失败。
Others        :
**************************************************************************************/
int Get_Leap_Second (int index,unsigned char *leap);


/**************************************************************************************
Function      : Get_Local_IP
Description   :获取板卡IP信息
Input         : IP_Msg *msg：表示板卡IP信息的指针。
Return        : 返回值为查询结果:
                0表示查询成功，-1表示查询失败。
Others        : 入参指针包含传回的IP信息，具体参见结构体IP_Msg定义。
**************************************************************************************/
int Get_Local_IP (int index,IP_Msg *msg);


/**************************************************************************************
Function      : Set_Local_IP
Description   :设置板卡IP信息
Input         : IP_Msg msg：表示待设置的板卡IP值。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        : 入参指针包含传回的IP信息，具体参见结构体IP_Msg定义。
**************************************************************************************/
int Set_Local_IP (int index,IP_Msg msg);


/**************************************************************************************
Function      : Get_TOD_IRIG_Out_Type
Description   :查询板卡B码和TOD当前输出类型
Input         :  int *type  0表示输出B码，1表示输出TOD。
Return        : 返回值为查询结果:
                0表示成功，-1表示失败。
Others        :
**************************************************************************************/
int Get_TOD_IRIG_Out_Type (int index,int *type);


/**************************************************************************************
Function      : Set_TOD_IRIG_Out_Type
Description   :设置信号输出类型
Input         : int type：表示待设置的输出类型值，0表示输出B码，1表示输出TOD。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        : TOD和B码同时只能输出其中一种信号。
**************************************************************************************/
int Set_TOD_IRIG_Out_Type (int index,int type);


/**************************************************************************************
Function      : Get_ IRIG_Out_Mode
Description   :查询板卡B码输出类型
Input         :int *mode 0表示输出标准B码，1表示输出军用B码。
Return        : 返回值为查询结果:
                0表示成功，-1表示失败。
Others        :
**************************************************************************************/
int Get_IRIG_Out_Mode (int index,int *mode);


/**************************************************************************************
Function      : Set_ IRIG_Out_Mode
Description   :设置B码输出类型
Input         : int type：表示待设置的输出类型值，0表示标准B码，1表示军用B码。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        : 标准B码与军用B码同时只能输出其中一种信号。
**************************************************************************************/
int Set_IRIG_Out_Mode (int index,int mode);


/**************************************************************************************
Function      : Get_ PTP_Work_Mode
Description   :查询板卡PTP模块工作模式
Input         :int *mode 1表示输入模式（slave），0表示输出模式（master）。
Return        : 返回值为查询结果:
                 0表示成功，-1表示失败。
Others        :
**************************************************************************************/
int Get_PTP_Work_Mode (int index,int *mode);


/**************************************************************************************
Function      : Set_ PTP_Work_Mode
Description   :设置板卡PTP模块工作模式
Input         : int mode：表示待设置的工作模式，0表示输入(slave)，1表示输出
(master)。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        : PTP输入与输出同时只能选择其中一种模式。
**************************************************************************************/
int Set_PTP_Work_Mode (int index,int mode);

/**************************************************************************************
Function      : Get_PTPin_Cfg
Description   :获取板卡PTP输入配置信息
Input         : PTPin_Msg *msg：表示板卡PTP输入配置的指针。
Return        : 返回值为查询结果:
                0表示查询成功，-1表示查询失败。
Others        : 入参指针包含传回的配置信息，具体参见结构体PTPin_Msg定义。
**************************************************************************************/
int Get_PTPin_Cfg (int index,PTPin_Msg *msg);


/**************************************************************************************
Function      : Set_ PTPin_Cfg
Description   :设置板卡PTP输入配置信息
Input         : PTPin_Msg msg：表示待设置的PTP输入配置信息。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        : 入参指针包含传回的配置信息，具体参见结构体PTPin_Msg定义。
**************************************************************************************/
int Set_PTPin_Cfg (int index,PTPin_Msg msg);


/**************************************************************************************
Function      : Get_PTPout_Cfg
Description   :获取板卡PTP输出配置信息
Input         : PTPout_Msg *msg：表示板卡PTP输入配置的指针。
Return        : 返回值为查询结果:
                0表示查询成功，-1表示查询失败。
Others        : 入参指针包含传回的配置信息，具体参见结构体PTPout_Msg定义。
**************************************************************************************/
int Get_PTPout_Cfg (int index,PTPout_Msg *msg);


/**************************************************************************************
Function      : Set_PTPout_Cfg
Description   :设置板卡PTP输出配置信息
Input         : PTPout_Msg msg：表示待设置的PTP输入配置信息。
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        : 入参指针包含传回的配置信息，具体参见结构体PTPout_Msg定义。
**************************************************************************************/
int Set_PTPout_Cfg (int index,PTPout_Msg msg);


/**************************************************************************************
Function      : Set_TimeSource
Description   :设置板卡的时间来源
Input         : ref 0:外部(GNSS/B码/PTP) 1:选择“内部(PC)”
Return        : 返回值为设置结果:
                0表示设置成功，-1表示设置失败。
Others        :
**************************************************************************************/
int Set_TimeSource (int index,unsigned char ref);

/**************************************************************************************
Function      : Get_TimeSource
Description   :获取板卡的时间来源
Input         : *ref 0:外部(GNSS/B码/PTP) 1:选择“内部(PC)”
Return        : 返回值为操作结果:
                0表示成功，-1表示失败。
Others        :
**************************************************************************************/
int Get_TimeSource (int index,unsigned char *ref);

/**************************************************************************************
Function      : Set_BoardCard_DateTime
Description   :设置板卡时间，只有在为主板卡时有效
Input         : Time_Msg msg：表示时间信息的结构体。
Return        : 返回结果:
                0表示查询成功，-1表示查询失败。
Others        : 入参指针包含传回的时间信息，具体参见结构体Time_Msg定义。
**************************************************************************************/
int Set_BoardCard_DateTime (int index,Time_Msg msg);


/**************************************************************************************
Function      : Get_Interrupt_DateTime
Description   : 当中断发生后，通过该接口获取板卡上的时间
Input         : Time_Msg msg：表示时间信息的结构体。
Return        : 返回结果:
                0表示查询成功，-1表示查询失败。
Others        : 入参指针包含传回的时间信息，具体参见结构体Time_Msg定义。
**************************************************************************************/
//int Get_Interrupt_DateTime (int index,Time_Msg *msg);

/**************************************************************************************
Function      : Get_Time_Sec
Description   :获取系统时间信息-UTC整数秒值
Input         :
Return        : 返回值为查询结果，单位为‘s’:
                返回值0表示查询失败。
Others        :
**************************************************************************************/
unsigned int Get_Time_Sec (int index);

/**************************************************************************************
Function      : Get_Time_MicroSec
Description   :获取系统时间信息-当前秒下值
Input         :
Return        : 返回值为查询结果，单位为‘us’:
                返回值0xFFFFFFFF表示查询失败。
Others        :
**************************************************************************************/
unsigned int Get_Time_MicroSec (int index);

/**************************************************************************************
Function      : Get_Time_NanoSec
Description   :获取系统时间信息-当前秒下值
Input         :
Return        : 返回值为查询结果，单位为‘ns’:
                返回值0xFFFFFFFF表示查询失败。
Others        :
**************************************************************************************/
unsigned int Get_Time_NanoSec (int index);

/**************************************************************************************
Function      : Get_Board_Normal_State
Description   :获取板卡常规状态信息
Input         :
Return        : 返回值为查询结果:
                具体参见结构体BoardState_Msg定义
Others        :
**************************************************************************************/
BoardState_Msg Get_Board_Normal_State (int index);


/**************************************************************************************
Function      : Get_Sys_Time_MilliSec
Description   :获取系统时间信息-秒上值换算为毫秒，再加上当前秒下毫秒值
Input         :
Return        : 返回值为查询结果，无符号64位数值，单位为‘ms’:
                返回值0xFFFFFFFFFFFFFFFF表示查询失败。
Others        :
**************************************************************************************/
unsigned long long Get_Sys_Time_MilliSec (int index);

/**************************************************************************************
Function      : Get_Sys_Time_MicroSec
Description   :获取系统时间信息-秒上值换算为微秒，再加上当前秒下微秒值
Input         :
Return        : 返回值为查询结果，无符号64位数值，单位为‘us’:
                返回值0xFFFFFFFFFFFFFFFF表示查询失败。
Others        :
**************************************************************************************/
unsigned long long Get_Sys_Time_MicroSec (int index);


/**************************************************************************************
Function      : Get_Sys_Time_Sec_And_Ns
Description   :获取系统时间信息-秒上值换算为微秒，再加上当前秒下微秒值
Input         :
Return        : 返回值为查询结果，无符号64位数值，单位为‘us’:
                返回值<0 失败。
Others        :
**************************************************************************************/
int Get_Sys_Time_Sec_And_Ns (int index,unsigned long *sec,unsigned long *ns);


int Get_Sys_LockStatus(int index,unsigned char *lockstatus);


#ifdef __cplusplus
}
#endif

#endif // PCIELIB_H
