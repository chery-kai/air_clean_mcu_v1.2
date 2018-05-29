#ifndef __NH3_SENSOR_H
#define __NH3_SENSOR_H


#include "stm32f10x.h"
#include <stdio.h>


/*
串口2 -- 用于读取"NH3传感器信号"
UART_TXD: PA2 
UART_RXD: PA3 

NH3传感器通讯协议:
    波特率 9600
    数据位 8 位
    停止位 1 位
    校验位 无
*/

typedef enum{
    NH3_TRUE,
    NH3_FALSE,
}nh3_res_e;

// 串口2-USART2
#define  NH3_USARTx                     USART2
#define  NH3_USART_CLK                  RCC_APB1Periph_USART2
#define  NH3_USART_APBxClkCmd           RCC_APB1PeriphClockCmd
#define  NH3_USART_BAUDRATE             9600

// USART GPIO 引脚宏定义
#define  NH3_USART_GPIO_CLK             RCC_APB2Periph_GPIOA
#define  NH3_USART_GPIO_APBxClkCmd      RCC_APB2PeriphClockCmd
    
#define  NH3_USART_TX_GPIO_PORT         GPIOA   
#define  NH3_USART_TX_GPIO_PIN          GPIO_Pin_2
#define  NH3_USART_RX_GPIO_PORT         GPIOA
#define  NH3_USART_RX_GPIO_PIN          GPIO_Pin_3

#define  NH3_USART_IRQ                  USART2_IRQn
//#define  NH3_USART_IRQHandler           USART2_IRQHandler


void bsp_USART2_IRQHandler(void);

/**
 * bsp.c 文件 BSP_Init 函数中只能调用 NH3_USART_Config(引脚初始化), 
 * 若直接调用 NH3_Sensor_Init, 将导致系统起不来, 与外置RTC初始化相同情形
 * 
 * -- 有待寻找原因！！！
 * 可能是因为 NH3_Sensor_Init 函数中有延时函数(bsp_DelayUS)
 */
void NH3_USART_Config(void);    //连接NH3传感器的引脚初始化

//int NH3_Sensor_Init(void);      //返回0, 初始化成功
//int NH3_Sensor_Request(void);   //返回氨气浓度
/*
NH3_Sensor_Init, NH3_Sensor_Request 中有延时函数, 因此将每个函数的延时分开
*/
void NH3_Sensor_Init_Request(void);
int NH3_Sensor_Init_Check(void);

void NH3_Sensor_Value_Request(void);
int NH3_Sensor_Value_Read(void);

//test
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);

/*
氨气传感器工作原理:
模组在出厂时配置为主动上传的通讯模式, 模组每隔一秒会对外发送一次当前的浓度值(浓度为 16 进制)
这里采用问答式, STM32串口向氨气传感器请求数据, 氨气传感器发送当前浓度值, STM32再通过串口读出

串口通过发送 0x78 指令, 把通讯模式更改为 0x04(问答式):
    起始位 地址 命令 通讯模式            校验值
    FF      01   78     04    00 00 00 00  83  
串口接收氨气浓度传感器的返回数据判定是否修改成功
    起始位 命令 返回标定             校验值
    FF      78     01   00 00 00 00 00 87           //01 代表成功

模组在收到 0x86 指令(读取模组浓度)后会发送当前的浓度值
    起始位 地址 命令              校验值
    FF      01   86 00 00 00 00 00  79
0x86 读取传感器浓度
    起始位 命令 传感器浓度值               校验值
                高字节 低字节
    FF      86    00     00      00 00 00 00 7A     //浓度值为 0

气体浓度值=气体浓度高位*256+气体浓度低位。
*/

#endif /* __NH3_SENSOR_H */
