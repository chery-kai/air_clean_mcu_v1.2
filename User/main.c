#include "includes.h"


#define MAIN_DEBUG_ON                   0
#define MAIN_DEBUG(fmt,arg...)          do{\
                                            if(MAIN_DEBUG_ON)\
                                            printf("<<MAIN-DEBUG>> [%d]"fmt"\n",__LINE__, ##arg);\
                                        }while(0)


/*
*********************************************************************************************************
*                                                 TCB
*********************************************************************************************************
*/
//定义任务控制块
static  OS_TCB  AppTaskStartTCB;    

//AIRCLEAN 定义任务控制块
static  OS_TCB  AirCleanTaskTCB;            //
static  OS_TCB  KeyControlTaskTCB;          //
static  OS_TCB  ScreenRefreshTaskTCB;       //

static  OS_TCB  CloseFanTaskTCB;            //
static  OS_TCB  ScheduledTaskTCB;
static  OS_TCB  CycleCheckTaskTCB;          //
static  OS_TCB  DetectOverflowTaskTCB;      //

static  OS_TCB  WifiNetHandleTaskTCB;       //


/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/
//定义任务堆栈
static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];

//AIRCLEAN 定义任务堆栈
static  CPU_STK  AirCleanTaskStk[AIR_CLEAN_TASK_STK_SIZE];
static  CPU_STK  KeyControlTaskStk[KEY_CONTROL_TASK_STK_SIZE];
static  CPU_STK  ScreenRefreshTaskStk[SCREEN_REFRESH_TASK_STK_SIZE];

static  CPU_STK  CloseFanTaskStk[CLOSE_FAN_TASK_STK_SIZE];
static  CPU_STK  ScheduledTaskStk[SCHEDULED_TASK_STK_SIZE];
static  CPU_STK  CycleCheckTaskStk[CYCLE_CHECK_TASK_STK_SIZE];
static  CPU_STK  DetectOverflowStk[DETECT_OVERFLOW_TASK_STK_SIZE];

static  CPU_STK  WifiNetHandleStk[WIFI_NET_HANDLE_TASK_STK_SIZE];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void   AppTaskStart(void *p_arg);
static  void   AppTaskCreate(void);

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
/*
*********************************************************************************************************
*   函 数 名: main
*   功能说明: 整个工程入口执行函数
*   形    参：无
*   返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
    OS_ERR  err;

    /* 初始化"uC/OS-III"内核 */ 
    OSInit(&err);

    /*创建任务*/
    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                // 任务控制块指针   
                 (CPU_CHAR   *)"App Task Start",                // 任务名称
                 (OS_TASK_PTR ) AppTaskStart,                   // 任务代码指针
                 (void       *) 0,                              // 传递给任务的参数parg
                 (OS_PRIO     ) APP_TASK_START_PRIO,            // 任务优先级
                 (CPU_STK    *)&AppTaskStartStk[0],             // 任务堆栈基地址
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10,   // 堆栈剩余警戒线
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,        // 堆栈大小
                 (OS_MSG_QTY  ) 5u,                             // 可接收的最大消息队列数
                 (OS_TICK     ) 0u,                             // 时间片轮转时间
                 (void       *) 0,                              // 任务控制块扩展信息
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | 
                                OS_OPT_TASK_STK_CLR),           // 任务选项
                 (OS_ERR     *)&err);                           // 返回值
    
    /* 启动多任务系统，控制权交给uC/OS-III */
    OSStart(&err);
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
static void AppTaskStart(void *p_arg)
{
    OS_ERR err;
    (void)p_arg;

    //板级初始化
    BSP_Init();                         //Initialize BSP functions
    MAIN_DEBUG("[AppTaskStart] BSP_Init do success... \r\n");
    
    CPU_Init();
    MAIN_DEBUG("[AppTaskStart] CPU_Init do success... \r\n");
    
    BSP_Tick_Init();
    MAIN_DEBUG("[AppTaskStart] BSP_Tick_Init do success... \r\n");
    
    //Mem_Init();                       //Initialize Memory Management Module
    //若执行Mem_Init，这里初始化会导致程序执行不下去，只能执行到此处, 原因待查！！！
    //MAIN_DEBUG("[AppTaskStart] Mem_Init do success...\n\r");
    
#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);       //Compute CPU capacity with no task running
    MAIN_DEBUG("[AppTaskStart] OSStatTaskCPUUsageInit func return err = %d \r\n",err);
#endif
    
    init_air_clean();                   //全局变量 air_clean_cfg 初始化
    
    MAIN_DEBUG("[AppTaskStart] Creating Application Tasks Start... \r\n");
    AppTaskCreate();                                //Create Application Tasks
    MAIN_DEBUG("[AppTaskStart] Creating Application Tasks Finish... \r\n");
    
    //通电情况下, 蜂鸣器需发声一次
    macBEEP_ON();
    OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err);
    macBEEP_OFF();
    
    //Delete task
    OSTaskDel(&AppTaskStartTCB,&err);
}


/*
*********************************************************************************************************
*   函 数 名: AppTaskCreate
*   功能说明: 创建应用任务
*   形    参：p_arg 是在创建该任务时传递的形参
*   返 回 值: 无
*********************************************************************************************************
*/
static  void  AppTaskCreate(void)
{
    OS_ERR      err;
    
    //Create the air clean task
    #if MANUAL_EDITION  //手动版创建 ManualAirCleanTask 线程
    OSTaskCreate((OS_TCB     *)&AirCleanTaskTCB,
                 (CPU_CHAR   *)"Manual Air Clean Task",
                 (OS_TASK_PTR ) ManualAirCleanTask,
                 (void       *) 0,
                 (OS_PRIO     ) AIR_CLEAN_TASK_PRIO,
                 (CPU_STK    *)&AirCleanTaskStk[0],
                 (CPU_STK_SIZE) AIR_CLEAN_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) AIR_CLEAN_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    MAIN_DEBUG("[AppTaskCreate] OSTaskCreate ManualAirCleanTask return err = %d \r\n", err);
    #else               //自动版创建 AirCleanTask 线程
    OSTaskCreate((OS_TCB     *)&AirCleanTaskTCB,
                 (CPU_CHAR   *)"Air Clean Task",
                 (OS_TASK_PTR ) AirCleanTask,
                 (void       *) 0,
                 (OS_PRIO     ) AIR_CLEAN_TASK_PRIO,
                 (CPU_STK    *)&AirCleanTaskStk[0],
                 (CPU_STK_SIZE) AIR_CLEAN_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) AIR_CLEAN_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    MAIN_DEBUG("[AppTaskCreate] OSTaskCreate AirCleanTask return err = %d \r\n", err);
    #endif
    
    //Create the screen refresh task
    OSTaskCreate((OS_TCB     *)&ScreenRefreshTaskTCB,
                 (CPU_CHAR   *)"Screen Refresh Task",
                 (OS_TASK_PTR ) ScreenRefreshTask,
                 (void       *) 0,
                 (OS_PRIO     ) SCREEN_REFRESH_TASK_PRIO,
                 (CPU_STK    *)&ScreenRefreshTaskStk[0],
                 (CPU_STK_SIZE) SCREEN_REFRESH_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) SCREEN_REFRESH_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    MAIN_DEBUG("[AppTaskCreate] OSTaskCreate ScreenRefreshTask return err = %d \r\n", err);
    
    //Create the key control task
    OSTaskCreate((OS_TCB     *)&KeyControlTaskTCB,
                 (CPU_CHAR   *)"Key Control Task",
                 (OS_TASK_PTR ) KeyControlTask,
                 (void       *) 0,
                 (OS_PRIO     ) KEY_CONTROL_TASK_PRIO,
                 (CPU_STK    *)&KeyControlTaskStk[0],
                 (CPU_STK_SIZE) KEY_CONTROL_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) KEY_CONTROL_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    MAIN_DEBUG("[AppTaskCreate] OSTaskCreate KeyControlTask return err = %d \r\n", err);
    
    
    OSTaskCreate((OS_TCB     *)&CloseFanTaskTCB,
                 (CPU_CHAR   *)"Close Fan Task",
                 (OS_TASK_PTR ) CloseFanTask,
                 (void       *) 0,
                 (OS_PRIO     ) CLOSE_FAN_TASK_PRIO,
                 (CPU_STK    *)&CloseFanTaskStk[0],
                 (CPU_STK_SIZE) CLOSE_FAN_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) CLOSE_FAN_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    MAIN_DEBUG("[AppTaskCreate] OSTaskCreate CloseFanTask return err = %d \r\n", err);
    
    OSTaskCreate((OS_TCB     *)&ScheduledTaskTCB,
                 (CPU_CHAR   *)"Scheduled Task",
                 (OS_TASK_PTR ) ScheduledTask,
                 (void       *) 0,
                 (OS_PRIO     ) SCHEDULED_TASK_PRIO,
                 (CPU_STK    *)&ScheduledTaskStk[0],
                 (CPU_STK_SIZE) SCHEDULED_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) SCHEDULED_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,//5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    MAIN_DEBUG("[AppTaskCreate] OSTaskCreate ScheduledTask return err = %d \r\n", err);
    
    OSTaskCreate((OS_TCB     *)&CycleCheckTaskTCB,
                 (CPU_CHAR   *)"Cycle Check Task",
                 (OS_TASK_PTR ) CycleCheckTask,
                 (void       *) 0,
                 (OS_PRIO     ) CYCLE_CHECK_TASK_PRIO,
                 (CPU_STK    *)&CycleCheckTaskStk[0],
                 (CPU_STK_SIZE) CYCLE_CHECK_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) CYCLE_CHECK_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,//5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    MAIN_DEBUG("[AppTaskCreate] OSTaskCreate CycleCheckTask return err = %d \r\n", err);
    
    OSTaskCreate((OS_TCB     *)&DetectOverflowTaskTCB,
                 (CPU_CHAR   *)"Detect Overflow Task",
                 (OS_TASK_PTR ) DetectOverflowTask,
                 (void       *) 0,
                 (OS_PRIO     ) DETECT_OVERFLOW_TASK_PRIO,
                 (CPU_STK    *)&DetectOverflowStk[0],
                 (CPU_STK_SIZE) DETECT_OVERFLOW_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) DETECT_OVERFLOW_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,//5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    MAIN_DEBUG("[AppTaskCreate] OSTaskCreate DetectOverflowTask return err = %d \r\n", err);
    
    #if NETWORK_VERSION
    OSTaskCreate((OS_TCB     *)&WifiNetHandleTaskTCB,
                 (CPU_CHAR   *)"Wifi Net Handle Task",
                 (OS_TASK_PTR ) WifiNetHandleTask,
                 (void       *) 0,
                 (OS_PRIO     ) WIFI_NET_HANDLE_TASK_PRIO,
                 (CPU_STK    *)&WifiNetHandleStk[0],
                 (CPU_STK_SIZE) WIFI_NET_HANDLE_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) WIFI_NET_HANDLE_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,//5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    MAIN_DEBUG("[AppTaskCreate] OSTaskCreate WifiNetHandleTask return err = %d \r\n", err);
    #endif
}




