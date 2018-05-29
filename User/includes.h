/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           MASTER INCLUDES
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Micrium uC-Eval-STM32F107
*                                         Evaluation Board
*
* Filename      : includes.h
* Version       : V1.00
* Programmer(s) : EHS
*                 DC
*********************************************************************************************************
*/

#ifndef  INCLUDES_PRESENT
#define  INCLUDES_PRESENT

//#ifndef SOFTRESET
//#define SOFTRESET
//#endif

/*
*********************************************************************************************************
*                                         STANDARD LIBRARIES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>


/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>


/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/
#include  "app.h"
#include  "app_cfg.h"
#include  "appfunc.h"
#include  "appwifi.h"

//#include "stm32f10x.h"


#include "bsp.h"

#include "bsp_atomizer.h"
#include "bsp_beep.h"
#include "bsp_debug_usart.h"
#include "bsp_external_rtc.h"       //外置 RTC
#include "bsp_fan.h"

#include "bsp_internal_flash.h"     //内部 Flash
#include "bsp_liquid_sensor.h"


//NH3 模块
#include "myiic.h"
#include "bsp_nh3_sensor.h"

#include "bsp_pump.h"
#include "bsp_remote.h"
#include "bsp_screen.h"
#include "bsp_solenoid.h"
#include "bsp_wifi.h"


/*
*********************************************************************************************************
*                                                 OS
*********************************************************************************************************
*/

#include  <os.h>
#include "os_type.h"
#include "os_cfg_app.h"


/*
*********************************************************************************************************
*                                                 ST
*********************************************************************************************************
*/

#include  <stm32f10x.h>


/*
*********************************************************************************************************
*                                              FUNCTION
*********************************************************************************************************
*/

#define bsp_DelayMS(ms) bsp_DelayUS(1000*ms)
void bsp_DelayUS(uint32_t _ulDelayTime);
void Soft_Reset(void);


/*
*********************************************************************************************************
*                                            INCLUDES END
*********************************************************************************************************
*/


#endif

