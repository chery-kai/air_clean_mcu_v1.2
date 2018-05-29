/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTI
  
  AL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"


//wifi
//uint8_t ucTcpClosedFlag = 0;


//Clock
//char TimeDisplay=0;
//extern uint32_t TimeDisplay;


/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    printf("HardFault!!!");
#ifdef SOFTRESET
    Soft_Reset();
#endif
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}


/**
  * @brief  This function handles RTC interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)       //使用外置RTC获取时间, 内置RTC不使用
{
    /*
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();  
    OSIntNestingCtr++;
    CPU_CRITICAL_EXIT();
    */

    /*
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
    {
        // Clear the RTC Second interrupt
        RTC_ClearITPendingBit(RTC_IT_SEC);
        TimeDisplay = 1;
        // Wait until last write operation on RTC registers has finished
        RTC_WaitForLastTask();
    }
    */

    /* 在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换 */
    //OSIntExit();
}


/**
  * @串口3中断, 用于处理 WIFI模块
  * @param  None
  * @retval None
  */
void USART3_IRQHandler(void)
{
    uint8_t ucCh,i;
    uint8_t Clear = Clear;

    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();  
    OSIntNestingCtr++;
    CPU_CRITICAL_EXIT();

    if(USART_GetITStatus(WIFI_USARTx,USART_IT_RXNE) != RESET)
    {
        rxBuffer[rxCounter++] = WIFI_USARTx->DR;
    }else if(USART_GetITStatus(WIFI_USARTx,USART_IT_IDLE) != RESET)
    {
        Clear = WIFI_USARTx->SR;
        Clear = WIFI_USARTx->DR;
        idleFlag = 1;
    }

    OSIntExit();
}


/**
  * @串口3中断, 用于处理 NH3传感器模块
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();  
    OSIntNestingCtr++;
    CPU_CRITICAL_EXIT();

    bsp_USART2_IRQHandler();

    /* 在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换 */
    OSIntExit();
}


__IO uint16_t IC2Value = 0;
__IO uint16_t IC1Value = 0;
__IO float DutyCycle = 0;
__IO float Frequency = 0;

/*
 * 如果是第一个上升沿中断, 计数器会被复位, 锁存到CCR1寄存器的值是0, CCR2寄存器的值也是0
 * 无法计算频率和占空比. 当第二次上升沿到来的时候, CCR1和CCR2捕获到的才是有效值. 其中
 * CCR1对应的是周期, CCR2对应的是占空比
 */
void ADVANCE_TIM_IRQHandler(void)
{
    /* 清除中断标志位 */
    TIM_ClearITPendingBit(ADVANCE_TIM, TIM_IT_CC1);

    /* 获取输入捕获值 */
    IC1Value = TIM_GetCapture1(ADVANCE_TIM);
    IC2Value = TIM_GetCapture2(ADVANCE_TIM);

    // 注意: 捕获寄存器CCR1和CCR2的值在计算占空比和频率的时候必须加1
    if (IC1Value != 0)
    {
        //printf("[ADVANCE_TIM_IRQHandler] IC1Value =  %d, IC2Value = %d\r\n", IC1Value, IC2Value);
        /* 占空比计算 */
        //DutyCycle = (float)((IC2Value+1) * 100) / (IC1Value+1);
        //printf("[ADVANCE_TIM_IRQHandler] 占空比 %0.2f\r\n", DutyCycle);
        
        DISABLE_ADVANCE_TIM();
        global_fan_one_clear_flag = 0;
        
        /* 频率计算 */
        //Frequency = (72000000/(ADVANCE_TIM_PSC+1))/(float)(IC1Value+1);
        //printf("占空比 %0.2f  %%   频率 %0.2f  Hz\r\n",DutyCycle,Frequency);
    }
    else
    {
        DutyCycle = 0;
        Frequency = 0;
    }
}

/*
风扇故障捕获, 采用高级定时器1 和 高级定时器8
只要捕获到方波, 则认为风扇是没有故障的
这里并没有准确抓取到PWM波的频率 
*/

void ADVANCE2_TIM_IRQHandler(void)
{
    /* 清除中断标志位 */
    TIM_ClearITPendingBit(ADVANCE2_TIM, TIM_IT_CC1);

    /* 获取输入捕获值 */
    IC1Value = TIM_GetCapture1(ADVANCE2_TIM);
    IC2Value = TIM_GetCapture2(ADVANCE2_TIM);

    // 注意: 捕获寄存器CCR1和CCR2的值在计算占空比和频率的时候必须加1
    if (IC1Value != 0)
    {
        //printf("[ADVANCE2_TIM_IRQHandler] IC1Value =  %d, IC2Value = %d\r\n", IC1Value, IC2Value);
        /* 占空比计算 */
        //DutyCycle = (float)((IC2Value+1) * 100) / (IC1Value+1);
        //printf("[ADVANCE2_TIM_IRQHandler] 占空比 %0.2f\r\n", DutyCycle);
        
        DISABLE_ADVANCE2_TIM();
        global_fan_two_clear_flag = 0;
        
        /* 频率计算 */
        //Frequency = (72000000/(ADVANCE_TIM_PSC+1))/(float)(IC1Value+1);
        //printf("占空比 %0.2f  %%   频率 %0.2f  Hz\r\n",DutyCycle,Frequency);
    }
    else
    {
        DutyCycle = 0;
        Frequency = 0;
    }
}


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
