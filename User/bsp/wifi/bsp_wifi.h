#ifndef  __BSP_WIFI_H
#define	 __BSP_WIFI_H

#include "stm32f10x.h"
#include <stdio.h>
#include <stdbool.h>

void USART_Config(void);
void WIFI_UART_INIT();

#define  WIFI_USARTx                   USART3
#define  WIFI_USART_CLK                RCC_APB1Periph_USART3
#define  WIFI_USART_APBxClkCmd         RCC_APB1PeriphClockCmd
#define  WIFI_USART_BAUDRATE           9600

#define  WIFI_USART_GPIO_CLK           (RCC_APB2Periph_GPIOB)
#define  WIFI_USART_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd
    
#define  WIFI_USART_TX_GPIO_PORT       GPIOB   
#define  WIFI_USART_TX_GPIO_PIN        GPIO_Pin_10
#define  WIFI_USART_RX_GPIO_PORT       GPIOB
#define  WIFI_USART_RX_GPIO_PIN        GPIO_Pin_11

#define  WIFI_USART_IRQ                USART3_IRQn
#define  WIFI_USART_IRQHandler         USART3_IRQHandler

#endif
