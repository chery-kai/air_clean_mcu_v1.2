
#ifndef __MYIIC_H
#define __MYIIC_H

#include "stm32f10x.h"


//SCL PB6
//SDA PB7


#define macI2C_RTC_WR	0		/* 写控制bit */
#define macI2C_RTC_RD	1		/* 读控制bit */

/* 定义I2C总线连接的GPIO端口, 用户只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define macGPIO_PORT_I2C_RTC	 GPIOB			/* GPIO端口 */
#define macRCC_I2C_RTC_PORT 	 RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define macI2C_RTC_SCL_PIN		 GPIO_Pin_6			/* 连接到SCL时钟线的GPIO */
#define macI2C_RTC_SDA_PIN		 GPIO_Pin_7			/* 连接到SDA数据线的GPIO */

// 定义读写SCL和SDA的宏，已增加代码的可移植性和可阅读性
#if 1	//条件编译： 1 选择GPIO的库函数实现IO读写
	#define macI2C_RTC_SCL_1()  GPIO_SetBits(macGPIO_PORT_I2C_RTC, macI2C_RTC_SCL_PIN)		/* SCL = 1 */
	#define macI2C_RTC_SCL_0()  GPIO_ResetBits(macGPIO_PORT_I2C_RTC, macI2C_RTC_SCL_PIN)		/* SCL = 0 */
	
	#define macI2C_RTC_SDA_1()  GPIO_SetBits(macGPIO_PORT_I2C_RTC, macI2C_RTC_SDA_PIN)		/* SDA = 1 */
	#define macI2C_RTC_SDA_0()  GPIO_ResetBits(macGPIO_PORT_I2C_RTC, macI2C_RTC_SDA_PIN)		/* SDA = 0 */
	
	#define macI2C_RTC_SDA_READ()  GPIO_ReadInputDataBit(macGPIO_PORT_I2C_RTC, macI2C_RTC_SDA_PIN)	/* 读SDA口线状态 */
#else	//这个分支选择直接寄存器操作实现IO读写
    //注意：如下写法，在IAR最高级别优化时，会被编译器错误优化 
	#define macI2C_RTC_SCL_1()  macGPIO_PORT_I2C_RTC->BSRR = macI2C_RTC_SCL_PIN				/* SCL = 1 */
	#define macI2C_RTC_SCL_0()  macGPIO_PORT_I2C_RTC->BRR = macI2C_RTC_SCL_PIN				/* SCL = 0 */
	
	#define macI2C_RTC_SDA_1()  macGPIO_PORT_I2C_RTC->BSRR = macI2C_RTC_SDA_PIN				/* SDA = 1 */
	#define macI2C_RTC_SDA_0()  macGPIO_PORT_I2C_RTC->BRR = macI2C_RTC_SDA_PIN				/* SDA = 0 */
	
	#define macI2C_RTC_SDA_READ()  ((macGPIO_PORT_I2C_RTC->IDR & macI2C_RTC_SDA_PIN) != 0)	/* 读SDA口线状态 */
#endif



//IO方向设置
//#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(unsigned int)8<<28;}
//#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(unsigned int)3<<28;}

//IO操作函数	 
//#define IIC_SCL    PBout(6) //SCL
//#define IIC_SDA    PBout(7) //SDA	 
//#define READ_SDA   PBin(7)  //输入SDA 


//IIC所有操作函数
//调用不到 bsp_DelayUS, 以 i2c_Delay 替代
//void i2c_Delay(int n);
void i2c_Delay();

void IIC_Init(void);                //初始化IIC的IO口
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  


#endif //__MYIIC_H

















