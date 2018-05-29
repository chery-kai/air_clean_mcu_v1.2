/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                           (c) Copyright 2009-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Micrium uC-Eval-STM32F107
*                                         Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : JJL
*                 EHS
*                 DC
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__


/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/

#define  APP_CFG_SERIAL_EN                          DEF_DISABLED//DEF_ENABLED


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                    2u

//AIRCLEAN 优先级
#define  AIR_CLEAN_TASK_PRIO                    12u
#define  KEY_CONTROL_TASK_PRIO                  15u
#define  SCREEN_REFRESH_TASK_PRIO               11u

#define  CLOSE_FAN_TASK_PRIO                    13u
#define  SCHEDULED_TASK_PRIO                    20u
#define  CYCLE_CHECK_TASK_PRIO                  30u
#define  DETECT_OVERFLOW_TASK_PRIO              19u

#define  WIFI_NET_HANDLE_TASK_PRIO              28u
//这里优先级设置可能会导致程序报错: HardFault!!! 
//原因待查！！！


/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_TASK_START_STK_SIZE                    128*30u

//AIRCLEAN 定义任务堆栈大小
#define  AIR_CLEAN_TASK_STK_SIZE                    512*3u
#define  KEY_CONTROL_TASK_STK_SIZE                  512*3u
#define  SCREEN_REFRESH_TASK_STK_SIZE               512u

#define  CLOSE_FAN_TASK_STK_SIZE                    64u
#define  SCHEDULED_TASK_STK_SIZE                    128u
#define  CYCLE_CHECK_TASK_STK_SIZE                  512*3u
#define  DETECT_OVERFLOW_TASK_STK_SIZE              128u

#define  WIFI_NET_HANDLE_TASK_STK_SIZE              512u


/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#define  APP_TRACE_LEVEL                            TRACE_LEVEL_INFO
#define  APP_TRACE                                  printf

#define  APP_TRACE_INFO(x)            ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DEBUG(x)           ((APP_TRACE_LEVEL >= TRACE_LEVEL_DEBUG) ? (void)(APP_TRACE x) : (void)0)

//***************************PART1:ON/OFF define*******************************

#define OS_DEBUG_ON                 0
#define OS_DEBUG_ARRAY_ON           0
#define OS_DEBUG_FUNC_ON            0
// Log define
#define OS_INFO(fmt,arg...)           printf("<<-OS-INFO->> "fmt"\n",##arg)
#define OS_ERROR(fmt,arg...)          printf("<<-OS-ERROR->> "fmt"\n",##arg)

#define OS_DEBUG(fmt,arg...)          do{\
                                         if(OS_DEBUG_ON)\
                                         printf("<<-OS-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                       }while(0)
#define OS_DEBUG_ARRAY(array, num)    do{\
                                         int32_t i;\
                                         uint8_t* a = array;\
                                         if(OS_DEBUG_ARRAY_ON)\
                                         {\
                                            printf("<<-OS-DEBUG-ARRAY->>\n");\
                                            for (i = 0; i < (num); i++)\
                                            {\
                                                printf("%02x   ", (a)[i]);\
                                                if ((i + 1 ) %10 == 0)\
                                                {\
                                                    printf("\n");\
                                                }\
                                            }\
                                            printf("\n");\
                                        }\
                                       }while(0)
#define OS_DEBUG_FUNC()               do{\
                                         if(OS_DEBUG_FUNC_ON)\
                                         printf("<<-OS-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
                                       }while(0)

#endif