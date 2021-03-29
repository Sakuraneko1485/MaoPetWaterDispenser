/**********************************Copyright (c)**********************************
**                       版权所有 (C), 2015-2020, 涂鸦科技
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    wifi.h
 * @author  涂鸦综合协议开发组
 * @version v2.5.6
 * @date    2020.12.16
 * @brief   用户无需关心该文件实现内容
 */

/****************************** 免责声明 ！！！ *******************************
由于MCU类型和编译环境多种多样，所以此代码仅供参考，用户请自行把控最终代码质量，
涂鸦不对MCU功能结果负责。
******************************************************************************/

#ifndef __WIFI_H_
#define __WIFI_H_

//所需程序库牵引
//#include "stm32f1xx.h"
#include "stdio.h"
#include "string.h"
#include "protocol.h"
#include "system.h"
#include "mcu_api.h"
#include <SoftwareSerial.h>
#include <EEPROM.h>

//MCU功能引脚定义
#define BUTTON_PIN  PB6    // 配网按键的管脚定义
#define LED_PIN     PB4    // LED指示灯的管脚定义
#define NTC_PIN     PA1    // NTC测水温的管脚定义
#define SB_PIN      PA3    // 水泵控制的管脚定义
#define SW_PIN      PA5    // 水位检测的管脚定义
#define WS2812_PIN  PA7    // 幻彩指示灯的管脚定义
#define UV_PIN      PB1    // UV杀菌灯的管脚定义
#define WB_PIN      PB13   // 微波传感器的管脚定义
#define HOT_PIN     PB15   // 加热器的管脚定义

//EEPROM地址注册
extern int E_TempSet;
extern int E_SBSpeed;

//联网参数注册
extern char Device_Switch,UV_Switch,Warm_Switch;                //设备开关，UV杀菌开关，恒温开关
extern enum WorkMode Work_Mode;                                 //设备模式
extern enum WaterLevel water_level;                            //剩余水量档位
extern long Temp_Set,temp_current,Water_Control;                              //水温设置，当前水温

//=============================================================================
//定义常量
//如果编译发生错误: #40: expected an identifier  DISABLE = 0, 类似这样的错误提示，可以包含头文件 #include "stm32f1xx.h" 来解决
//=============================================================================
#ifndef TRUE
#define      TRUE                1
#endif

#ifndef FALSE
#define         FALSE            0
#endif

#ifndef NULL
#define         NULL             ((void *) 0)
#endif

#ifndef SUCCESS
#define         SUCCESS          1
#endif

#ifndef ERROR
#define         ERROR            0
#endif

#ifndef INVALID
#define         INVALID          0xFF
#endif

#ifndef ENABLE
#define         ENABLE           1
#endif

#ifndef DISABLE
#define         DISABLE          0
#endif
//=============================================================================
//dp数据点类型
//=============================================================================
#define         DP_TYPE_RAW                     0x00        //RAW 类型
#define         DP_TYPE_BOOL                    0x01        //bool 类型
#define         DP_TYPE_VALUE                   0x02        //value 类型
#define         DP_TYPE_STRING                  0x03        //string 类型
#define         DP_TYPE_ENUM                    0x04        //enum 类型
#define         DP_TYPE_BITMAP                  0x05        //fault 类型

//=============================================================================
//WIFI工作状态
//=============================================================================
#define         SMART_CONFIG_STATE              0x00
#define         AP_STATE                        0x01
#define         WIFI_NOT_CONNECTED              0x02
#define         WIFI_CONNECTED                  0x03
#define         WIFI_CONN_CLOUD                 0x04
#define         WIFI_LOW_POWER                  0x05
#define         SMART_AND_AP_STATE              0x06
#define         WIFI_SATE_UNKNOW                0xff
//=============================================================================
//wifi配网的方式
//=============================================================================
#define         SMART_CONFIG                    0x0  
#define         AP_CONFIG                       0x1   

//=============================================================================
//wifi复位状态
//=============================================================================
#define         RESET_WIFI_ERROR                0
#define         RESET_WIFI_SUCCESS              1

//=============================================================================
//wifi配置复位状态
//=============================================================================
#define         SET_WIFICONFIG_ERROR            0
#define         SET_WIFICONFIG_SUCCESS          1

//=============================================================================
//MCU固件升级状态
//=============================================================================
#define         FIRM_STATE_UN_SUPPORT           0x00                            //不支持 MCU 升级
#define         FIRM_STATE_WIFI_UN_READY        0x01                            //模块未就绪
#define         FIRM_STATE_GET_ERROR            0x02                            //云端升级信息查询失败
#define         FIRM_STATE_NO                   0x03                            //无需升级（云端无更新版本）
#define         FIRM_STATE_START                0x04                            //需升级，等待模块发起升级操作

//=============================================================================
//WIFI和mcu的工作方式 
//=============================================================================
#define         UNION_WORK                      0x0                             //mcu模块与wifi配合处理
#define         WIFI_ALONE                      0x1                             //wifi模块自处理

//=============================================================================
//系统工作模式
//=============================================================================
#define         NORMAL_MODE                     0x00                            //正常工作状态
#define         FACTORY_MODE                    0x01                            //工厂模式	
#define         UPDATE_MODE                     0x02                            //升级模式	 

//=============================================================================
//配网方式选择
//=============================================================================
#define         CONFIG_MODE_DEFAULT             "0"                             //默认配网方式
#define         CONFIG_MODE_LOWPOWER            "1"                             //低功耗配网方式
#define         CONFIG_MODE_SPECIAL             "2"                             //特殊配网方式  




//=============================================================================
//下发命令
//=============================================================================
typedef struct {
  unsigned char dp_id;                              //dp序号
  unsigned char dp_type;                            //dp类型
} DOWNLOAD_CMD_S;

#endif
