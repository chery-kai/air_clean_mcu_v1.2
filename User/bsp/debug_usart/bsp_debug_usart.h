#ifndef __DEBUG_USART_H
#define __DEBUG_USART_H

#include "stm32f10x.h"
#include <stdio.h>


//串口1作为调试串口

#define             macUSART_BAUD_RATE                       115200

#define             macUSARTx                                USART1
#define             macUSART_APBxClock_FUN                   RCC_APB2PeriphClockCmd
#define             macUSART_CLK                             RCC_APB2Periph_USART1
#define             macUSART_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define             macUSART_GPIO_CLK                        (RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO)     
#define             macUSART_TX_PORT                         GPIOA   
#define             macUSART_TX_PIN                          GPIO_Pin_9
#define             macUSART_RX_PORT                         GPIOA   
#define             macUSART_RX_PIN                          GPIO_Pin_10

void DEBUG_USART_Config(void);

#endif /* __DEBUG_USART_H */
