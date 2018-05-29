#ifndef _APP_H_
#define _APP_H_

#include <os.h>
#include "includes.h"

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
int init_air_clean();

//清洗流程主任务
void AirCleanTask(void *p_arg);         //自动清洗
void ManualAirCleanTask(void *p_arg);   //手动清洗

//遥控器控制任务
void KeyControlTask(void * p_arg);

//显示屏控制任务
void ScreenRefreshTask(void * p_arg);

//检测"关闭风扇"任务
void CloseFanTask(void * p_arg);

//定时"上传数据"任务
void ScheduledTask(void * p_arg);

//检测"切换清洗"任务
void CycleCheckTask(void * p_arg);

//检测"溢出液位"任务
void DetectOverflowTask(void * p_arg);

//网络数据处理任务
void WifiNetHandleTask(void * p_arg);


extern char global_fan_one_clear_flag;        //风扇1故障清除标识
extern char global_fan_two_clear_flag;        //风扇2故障清除标识

#endif  //_APP_H_
