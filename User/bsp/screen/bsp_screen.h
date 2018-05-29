#ifndef __SCREEN_H
#define __SCREEN_H


#include "stm32f10x.h"


/*
SCREEN_SDAR:   PD8
SCREEN_SDAC:   PD9
SCREEN_CLK:    PD10
*/

//PE12, 13, 14 SDAR SDAC CLK
#define macSCREEN_SDAR_GPIO_PORT           GPIOD                       /* GPIO端口 */
#define macSCREEN_SDAR_GPIO_CLK            RCC_APB2Periph_GPIOD        /* GPIO端口时钟 */
#define macSCREEN_SDAR_GPIO_PIN            GPIO_Pin_9 //GPIO_Pin_8                 /* SCREEN_SDAR 引脚号 */

#define macSCREEN_SDAC_GPIO_PORT           GPIOD                       /* GPIO端口 */
#define macSCREEN_SDAC_GPIO_CLK            RCC_APB2Periph_GPIOD        /* GPIO端口时钟 */
#define macSCREEN_SDAC_GPIO_PIN            GPIO_Pin_8 //GPIO_Pin_9                 /* SCREEN_SDAC 引脚号 */

#define macSCREEN_CLK_GPIO_PORT            GPIOD                       /* GPIO端口 */
#define macSCREEN_CLK_GPIO_CLK             RCC_APB2Periph_GPIOD        /* GPIO端口时钟 */
#define macSCREEN_CLK_GPIO_PIN              GPIO_Pin_10                 /* SCREEN_CLK 引脚号 */



/* 使用标准的固件库控制IO */
#define SCREEN_SDAR_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macSCREEN_SDAR_GPIO_PORT,macSCREEN_SDAR_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macSCREEN_SDAR_GPIO_PORT,macSCREEN_SDAR_GPIO_PIN)

#define SCREEN_SDAC_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macSCREEN_SDAC_GPIO_PORT,macSCREEN_SDAC_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macSCREEN_SDAC_GPIO_PORT,macSCREEN_SDAC_GPIO_PIN)

#define SCREEN_CLK_ONOFF(a)  if (a)  \
                    GPIO_SetBits(macSCREEN_CLK_GPIO_PORT,macSCREEN_CLK_GPIO_PIN);\
                    else        \
                    GPIO_ResetBits(macSCREEN_CLK_GPIO_PORT,macSCREEN_CLK_GPIO_PIN)
                    
                    
void SCREEN_GPIO_Config(void);

void set_row_bit(char highlow);
void set_col_bit(char highlow); 
void set_screen_clk(char highlow);



//PD8 : TM1640_CLK
//PD9 : TM1640_DIN
#define macSCREEN_GPIO_PORT                 GPIOD
#define macSCREEN_GPIO_CLK                  RCC_APB2Periph_GPIOD
#define macSCREEN_CLK_GPIO_PIN              GPIO_Pin_10 
#define macSCREEN_DIN_GPIO_PIN              GPIO_Pin_9 


//将TM1640的CLK设置为输出高电平
#define TM1640_CLK_HIGH     GPIO_SetBits(macSCREEN_GPIO_PORT, macSCREEN_CLK_GPIO_PIN)

//将TM1640的CLK设置为输出低电平
#define TM1640_CLK_LOW      GPIO_ResetBits(macSCREEN_GPIO_PORT, macSCREEN_CLK_GPIO_PIN)

//将TM1640的DIN设置为输出高电平
#define TM1640_DIN_HIGH     GPIO_SetBits(macSCREEN_GPIO_PORT, macSCREEN_DIN_GPIO_PIN)

//将TM1640的DIN设置为输出低电平
#define TM1640_DIN_LOW      GPIO_ResetBits(macSCREEN_GPIO_PORT,macSCREEN_DIN_GPIO_PIN)


#define DATA_COMMAND_Z      0x40                    //采用自动地址加1模式
#define DATA_COMMAND_G      0x44                    //采用固定地址模式
//#define ADDR_START        0xc0                    //开始地址
#define ADDR_START          0x08                    //开始地址
#define DISP_CLOSE          0x80                    //关闭显示
#define DISP_OPEN           0x8f                    //开启显示


void TM1640_Init(void);                             //初始化TM1640
void TM1640_Start(void);                            //命令传输开始
void TM1640_End(void);                              //命令传输结束
void TM1640_Write_Byte(unsigned char dat);          //写入字节
//void TM1640_Display(void);                        //数码管显示
void TM1640_Display(unsigned char *data);           //数码管显示

//void LED_Display_date(int year_number,int month_number,int day_number,int week_number,int sec);


#endif /* __SCREEN_H */
