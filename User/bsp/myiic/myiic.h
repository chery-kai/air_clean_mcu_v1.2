
#ifndef __MYIIC_H
#define __MYIIC_H

#include "stm32f10x.h"


//SCL PB6
//SDA PB7


#define macI2C_RTC_WR	0		/* д����bit */
#define macI2C_RTC_RD	1		/* ������bit */

/* ����I2C�������ӵ�GPIO�˿�, �û�ֻ��Ҫ�޸�����4�д��뼴������ı�SCL��SDA������ */
#define macGPIO_PORT_I2C_RTC	 GPIOB			/* GPIO�˿� */
#define macRCC_I2C_RTC_PORT 	 RCC_APB2Periph_GPIOB		/* GPIO�˿�ʱ�� */
#define macI2C_RTC_SCL_PIN		 GPIO_Pin_6			/* ���ӵ�SCLʱ���ߵ�GPIO */
#define macI2C_RTC_SDA_PIN		 GPIO_Pin_7			/* ���ӵ�SDA�����ߵ�GPIO */

// �����дSCL��SDA�ĺ꣬�����Ӵ���Ŀ���ֲ�ԺͿ��Ķ���
#if 1	//�������룺 1 ѡ��GPIO�Ŀ⺯��ʵ��IO��д
	#define macI2C_RTC_SCL_1()  GPIO_SetBits(macGPIO_PORT_I2C_RTC, macI2C_RTC_SCL_PIN)		/* SCL = 1 */
	#define macI2C_RTC_SCL_0()  GPIO_ResetBits(macGPIO_PORT_I2C_RTC, macI2C_RTC_SCL_PIN)		/* SCL = 0 */
	
	#define macI2C_RTC_SDA_1()  GPIO_SetBits(macGPIO_PORT_I2C_RTC, macI2C_RTC_SDA_PIN)		/* SDA = 1 */
	#define macI2C_RTC_SDA_0()  GPIO_ResetBits(macGPIO_PORT_I2C_RTC, macI2C_RTC_SDA_PIN)		/* SDA = 0 */
	
	#define macI2C_RTC_SDA_READ()  GPIO_ReadInputDataBit(macGPIO_PORT_I2C_RTC, macI2C_RTC_SDA_PIN)	/* ��SDA����״̬ */
#else	//�����֧ѡ��ֱ�ӼĴ�������ʵ��IO��д
    //ע�⣺����д������IAR��߼����Ż�ʱ���ᱻ�����������Ż� 
	#define macI2C_RTC_SCL_1()  macGPIO_PORT_I2C_RTC->BSRR = macI2C_RTC_SCL_PIN				/* SCL = 1 */
	#define macI2C_RTC_SCL_0()  macGPIO_PORT_I2C_RTC->BRR = macI2C_RTC_SCL_PIN				/* SCL = 0 */
	
	#define macI2C_RTC_SDA_1()  macGPIO_PORT_I2C_RTC->BSRR = macI2C_RTC_SDA_PIN				/* SDA = 1 */
	#define macI2C_RTC_SDA_0()  macGPIO_PORT_I2C_RTC->BRR = macI2C_RTC_SDA_PIN				/* SDA = 0 */
	
	#define macI2C_RTC_SDA_READ()  ((macGPIO_PORT_I2C_RTC->IDR & macI2C_RTC_SDA_PIN) != 0)	/* ��SDA����״̬ */
#endif



//IO��������
//#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(unsigned int)8<<28;}
//#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(unsigned int)3<<28;}

//IO��������	 
//#define IIC_SCL    PBout(6) //SCL
//#define IIC_SDA    PBout(7) //SDA	 
//#define READ_SDA   PBin(7)  //����SDA 


//IIC���в�������
//���ò��� bsp_DelayUS, �� i2c_Delay ���
//void i2c_Delay(int n);
void i2c_Delay();

void IIC_Init(void);                //��ʼ��IIC��IO��
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  


#endif //__MYIIC_H

















