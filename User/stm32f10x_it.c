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
void RTC_IRQHandler(void)       //ʹ������RTC��ȡʱ��, ����RTC��ʹ��
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

    /* ��os_core.c�ļ��ﶨ��,����и������ȼ������������,��ִ��һ�������л� */
    //OSIntExit();
}


/**
  * @����3�ж�, ���ڴ��� WIFIģ��
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
  * @����3�ж�, ���ڴ��� NH3������ģ��
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

    /* ��os_core.c�ļ��ﶨ��,����и������ȼ������������,��ִ��һ�������л� */
    OSIntExit();
}


__IO uint16_t IC2Value = 0;
__IO uint16_t IC1Value = 0;
__IO float DutyCycle = 0;
__IO float Frequency = 0;

/*
 * ����ǵ�һ���������ж�, �������ᱻ��λ, ���浽CCR1�Ĵ�����ֵ��0, CCR2�Ĵ�����ֵҲ��0
 * �޷�����Ƶ�ʺ�ռ�ձ�. ���ڶ��������ص�����ʱ��, CCR1��CCR2���񵽵Ĳ�����Чֵ. ����
 * CCR1��Ӧ��������, CCR2��Ӧ����ռ�ձ�
 */
void ADVANCE_TIM_IRQHandler(void)
{
    /* ����жϱ�־λ */
    TIM_ClearITPendingBit(ADVANCE_TIM, TIM_IT_CC1);

    /* ��ȡ���벶��ֵ */
    IC1Value = TIM_GetCapture1(ADVANCE_TIM);
    IC2Value = TIM_GetCapture2(ADVANCE_TIM);

    // ע��: ����Ĵ���CCR1��CCR2��ֵ�ڼ���ռ�ձȺ�Ƶ�ʵ�ʱ������1
    if (IC1Value != 0)
    {
        //printf("[ADVANCE_TIM_IRQHandler] IC1Value =  %d, IC2Value = %d\r\n", IC1Value, IC2Value);
        /* ռ�ձȼ��� */
        //DutyCycle = (float)((IC2Value+1) * 100) / (IC1Value+1);
        //printf("[ADVANCE_TIM_IRQHandler] ռ�ձ� %0.2f\r\n", DutyCycle);
        
        DISABLE_ADVANCE_TIM();
        global_fan_one_clear_flag = 0;
        
        /* Ƶ�ʼ��� */
        //Frequency = (72000000/(ADVANCE_TIM_PSC+1))/(float)(IC1Value+1);
        //printf("ռ�ձ� %0.2f  %%   Ƶ�� %0.2f  Hz\r\n",DutyCycle,Frequency);
    }
    else
    {
        DutyCycle = 0;
        Frequency = 0;
    }
}

/*
���ȹ��ϲ���, ���ø߼���ʱ��1 �� �߼���ʱ��8
ֻҪ���񵽷���, ����Ϊ������û�й��ϵ�
���ﲢû��׼ȷץȡ��PWM����Ƶ�� 
*/

void ADVANCE2_TIM_IRQHandler(void)
{
    /* ����жϱ�־λ */
    TIM_ClearITPendingBit(ADVANCE2_TIM, TIM_IT_CC1);

    /* ��ȡ���벶��ֵ */
    IC1Value = TIM_GetCapture1(ADVANCE2_TIM);
    IC2Value = TIM_GetCapture2(ADVANCE2_TIM);

    // ע��: ����Ĵ���CCR1��CCR2��ֵ�ڼ���ռ�ձȺ�Ƶ�ʵ�ʱ������1
    if (IC1Value != 0)
    {
        //printf("[ADVANCE2_TIM_IRQHandler] IC1Value =  %d, IC2Value = %d\r\n", IC1Value, IC2Value);
        /* ռ�ձȼ��� */
        //DutyCycle = (float)((IC2Value+1) * 100) / (IC1Value+1);
        //printf("[ADVANCE2_TIM_IRQHandler] ռ�ձ� %0.2f\r\n", DutyCycle);
        
        DISABLE_ADVANCE2_TIM();
        global_fan_two_clear_flag = 0;
        
        /* Ƶ�ʼ��� */
        //Frequency = (72000000/(ADVANCE_TIM_PSC+1))/(float)(IC1Value+1);
        //printf("ռ�ձ� %0.2f  %%   Ƶ�� %0.2f  Hz\r\n",DutyCycle,Frequency);
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
