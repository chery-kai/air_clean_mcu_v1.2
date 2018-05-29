#include "bsp_wifi.h"
#include <stdio.h>  



static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = WIFI_USART_IRQ;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;

    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;

    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}


void USART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    WIFI_USART_GPIO_APBxClkCmd(WIFI_USART_GPIO_CLK, ENABLE);

    WIFI_USART_APBxClkCmd(WIFI_USART_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = WIFI_USART_TX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(WIFI_USART_TX_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = WIFI_USART_RX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(WIFI_USART_RX_GPIO_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = WIFI_USART_BAUDRATE;

    USART_InitStructure.USART_WordLength = USART_WordLength_8b;

    USART_InitStructure.USART_StopBits = USART_StopBits_1;

    USART_InitStructure.USART_Parity = USART_Parity_No ;

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(WIFI_USARTx, &USART_InitStructure);


    NVIC_Configuration();


    USART_ITConfig(WIFI_USARTx, USART_IT_IDLE, ENABLE);


    USART_ITConfig(WIFI_USARTx, USART_IT_RXNE, ENABLE);

    USART_Cmd(WIFI_USARTx, ENABLE);

    //USART_ClearFlag(USART1, USART_FLAG_TC);     
}

void WIFI_UART_INIT()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOE, ENABLE); 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    USART_Config();
    printf("wifi init2\r\n");
}

