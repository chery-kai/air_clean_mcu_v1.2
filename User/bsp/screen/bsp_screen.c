/**
  ******************************************************************************
  * @file    bsp_screen.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   屏幕显示应用函数接口
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
  
#include "bsp_screen.h"     

 /**
  * @brief  初始化控制LED的IO
  * @param  无
  * @retval 无
  */


void SCREEN_GPIO_Config(void)
{   
    /*定义一个 GPIO_InitTypeDef 类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;
    /*开启 GPIOE 的外设时钟*/
    RCC_APB2PeriphClockCmd(macSCREEN_SDAR_GPIO_CLK, ENABLE); 

    //*************************************************************************
    //初始化 PE12
    GPIO_InitStructure.GPIO_Pin = macSCREEN_SDAR_GPIO_PIN;
    //设置引脚模式为通用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    //设置引脚速率为50MHz
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    //调用库函数，初始化 PE12
    GPIO_Init(macSCREEN_SDAR_GPIO_PORT, &GPIO_InitStructure);

    //*************************************************************************
    //初始化 PE13
    GPIO_InitStructure.GPIO_Pin = macSCREEN_SDAC_GPIO_PIN;
    GPIO_Init(macSCREEN_SDAC_GPIO_PORT, &GPIO_InitStructure);

    //*************************************************************************
    //初始化 PE14
    GPIO_InitStructure.GPIO_Pin = macSCREEN_CLK_GPIO_PIN;
    GPIO_Init(macSCREEN_CLK_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_ResetBits(macSCREEN_SDAR_GPIO_PORT, macSCREEN_SDAR_GPIO_PIN);
    GPIO_ResetBits(macSCREEN_SDAC_GPIO_PORT, macSCREEN_SDAC_GPIO_PIN);
    GPIO_ResetBits(macSCREEN_CLK_GPIO_PORT, macSCREEN_CLK_GPIO_PIN);
}


void set_screen_clk(char highlow) {
    SCREEN_CLK_ONOFF(highlow);
}

void set_row_bit(char highlow) {
    SCREEN_SDAR_ONOFF(highlow);
}

void set_col_bit(char highlow) 
{
    SCREEN_SDAC_ONOFF(highlow);
}






//*********************************************
unsigned char Display_Num[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x40,0xb9,0x90,0X00};      //数码管段码
                             //  0    1    2    3    4    5    6    7    8    9    -    E    r    NC

void delay_uss(__IO u32 nCount)	 //简单的延时函数
{
    for(; nCount != 0; nCount--);
} 

//TM1640初始化
void TM1640_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(macSCREEN_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = macSCREEN_CLK_GPIO_PIN | macSCREEN_DIN_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(macSCREEN_GPIO_PORT, &GPIO_InitStructure);
}


void TM1640_Start(void)
{
    TM1640_DIN_HIGH;
    delay_uss(2);

    TM1640_CLK_HIGH;
    delay_uss(2);
        
    TM1640_DIN_LOW;
    delay_uss(2);

    TM1640_CLK_LOW;
    delay_uss(2);
}

void TM1640_End(void)
{
    TM1640_CLK_LOW;
    delay_uss(2);

    TM1640_DIN_LOW;
    delay_uss(2);

    TM1640_CLK_HIGH;
    delay_uss(2);

    TM1640_DIN_HIGH;
    delay_uss(2);
}


void TM1640_Write_Byte(unsigned char dat)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        TM1640_CLK_LOW;
        
        if (dat & 0x01)
        {
            TM1640_DIN_HIGH;
        }
        else
        {
            TM1640_DIN_LOW;
        }
        delay_uss(2);
        
        TM1640_CLK_HIGH;
        delay_uss(2);
        
        dat >>= 1;
    }
}


void TM1640_Display(unsigned char *data)
{
    int	i;
    //设置数据
    TM1640_Start();
    TM1640_Write_Byte(DATA_COMMAND_Z);        //这里是通过地址自动+1方法
    //TM1640_Write_Byte(DATA_COMMAND_G);          //这里是固定地址模式
    TM1640_End();

    //设置地址
    TM1640_Start();
    //*************************************************
    //写入刷新数据
    TM1640_Write_Byte(0xC0);
    TM1640_Write_Byte(data[10]);//GRID1 --> 第11行	
    TM1640_Write_Byte(data[9]);//GRID2 --> 第10行
    TM1640_Write_Byte(0xFF);	//无对应
    TM1640_Write_Byte(0xFF);//无对应
    TM1640_Write_Byte(0xFF);//无对应

    TM1640_Write_Byte(0xFF);//无对应
    TM1640_Write_Byte(0xFF);//无对应
    TM1640_Write_Byte(data[5]);//GRID8 --> 第6行
    TM1640_Write_Byte(data[6]);//GRID9 --> 第7行
    TM1640_Write_Byte(data[7]);//GRID10 --> 第8行

    TM1640_Write_Byte(data[8]);//GRID11 --> 第9行
    TM1640_Write_Byte(data[4]);//GRID12 --> 第5行
    TM1640_Write_Byte(data[3]);		//GRID13 --> 第4行
    TM1640_Write_Byte(data[2]);
    TM1640_Write_Byte(data[1]);		//GRID15 --> 第2行

    TM1640_Write_Byte(data[0]);		//GRID16 --> 第1行

    //*************************************************
    /*
    //测试
    TM1640_Write_Byte(0xC0);	//地址

    TM1640_Write_Byte(0xFF);	//数据
    TM1640_Write_Byte(0xFF);
    TM1640_Write_Byte(0xFF);
    TM1640_Write_Byte(0xFF);
    TM1640_Write_Byte(0xFF);

    TM1640_Write_Byte(0xFF);
    TM1640_Write_Byte(0xFF);
    TM1640_Write_Byte(0xFF);
    TM1640_Write_Byte(0xFF);
    TM1640_Write_Byte(0xFF);

    TM1640_Write_Byte(0xFF);
    TM1640_Write_Byte(0xFF);
    TM1640_Write_Byte(0x00);
    TM1640_Write_Byte(0x9B);    //显示 2 -- 1101 1001 --> 1001 1011 --> 0x9B
    TM1640_Write_Byte(0x00);

    TM1640_Write_Byte(0x06);    //显示 1 -- 0110 0000 --> 0000 0110 --> 0x06

    //芯片引脚:  SEG1  SEG2  SEG3  SEG4  SEG5  SEG6  SEG7  SEG8
    //原理图引脚：19    18    17    16    15    14    12    13
    */

    //*************************************************
    TM1640_End();

    //控制显示
    TM1640_Start();
    TM1640_Write_Byte(DISP_OPEN);				//控制显示
    TM1640_End();
}

