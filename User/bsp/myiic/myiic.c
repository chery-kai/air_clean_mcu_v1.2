#include "stm32f10x.h"
#include "myiic.h"


//////////////////////////////////////////////////////////////////////////////////	 

//////////////////////////////////////////////////////////////////////////////////
 
//bsp_DelayUS --> i2c_Delay
//void i2c_Delay(int n)
void i2c_Delay(void)
{
    /*
    uint8_t i,j;

    for(j = 0; j < n; j++)
        for (i = 0; i < 10; i++);
    */
    uint8_t i;
    for (i = 0; i < 10; i++);
}
 
 
 /*
 macI2C_RTC_SCL_1()
 macI2C_RTC_SCL_0()
 
 macI2C_RTC_SDA_1()
 macI2C_RTC_SDA_0()
 
 macI2C_RTC_SDA_READ()
 */
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = macI2C_RTC_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = macI2C_RTC_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ;   //设置为开漏输出，而不是推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	//PB6,PB7 输出高

    //注意这里的输出方式，选用"开漏输出"
    //IO配置成开漏输出，然后外接上拉，就实现了双向IO。


    /* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
    //IIC_Stop();
}

void IIC_Start(void)
{
    macI2C_RTC_SDA_1();
    macI2C_RTC_SCL_1();
    i2c_Delay();
    macI2C_RTC_SDA_0();
    i2c_Delay();
    macI2C_RTC_SCL_0();
    i2c_Delay();

    /*
    //SDA_OUT();     //sda线输出
     macI2C_RTC_SDA_1();
    i2c_Delay(1);
    macI2C_RTC_SCL_1();
    i2c_Delay(5);
    macI2C_RTC_SDA_0();//START:when CLK is high,DATA change form high to low 
    i2c_Delay(5);
    macI2C_RTC_SCL_0();//钳住I2C总线，准备发送或接收数据 
    i2c_Delay(2);
    */
}  
void IIC_Stop(void)
{
    /*
    //SDA_OUT();//sda线输出
    macI2C_RTC_SCL_0();
    macI2C_RTC_SDA_0();//STOP:when CLK is high DATA change form low to high
    i2c_Delay(4);
    macI2C_RTC_SCL_1(); 
    i2c_Delay(5);
     macI2C_RTC_SDA_1();//发送I2C总线结束信号
    i2c_Delay(4);
    */
    macI2C_RTC_SDA_0();
    macI2C_RTC_SCL_1();
    i2c_Delay();
    macI2C_RTC_SDA_1();
    
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
    /*
    u8 ucErrTime=0;
    //SDA_IN();      //SDA设置为输入  
     macI2C_RTC_SDA_1();i2c_Delay(1);	   
    macI2C_RTC_SCL_1();i2c_Delay(1);	 
    while(macI2C_RTC_SDA_READ())
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            IIC_Stop();
            return 1;
        }
    }
    macI2C_RTC_SCL_0();//时钟输出0 	   
    return 0;  
    */

    uint8_t re;

    macI2C_RTC_SDA_1();	/* CPU释放SDA总线 */
    i2c_Delay();
    macI2C_RTC_SCL_1();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
    i2c_Delay();
    if (macI2C_RTC_SDA_READ())	/* CPU读取SDA口线状态 */
    {
        re = 1;
    }
    else
    {
        re = 0;
    }
    macI2C_RTC_SCL_0();
    i2c_Delay();
    return re;
} 


//产生ACK应答
void IIC_Ack(void)
{
    /*
    macI2C_RTC_SCL_0();
    //SDA_OUT();
    macI2C_RTC_SDA_0();
    i2c_Delay(2);
    macI2C_RTC_SCL_1();
    i2c_Delay(2);
    macI2C_RTC_SCL_0();
    */
    macI2C_RTC_SDA_0();	/* CPU驱动SDA = 0 */
    i2c_Delay();
    macI2C_RTC_SCL_1();	/* CPU产生1个时钟 */
    i2c_Delay();
    macI2C_RTC_SCL_0();
    i2c_Delay();
    macI2C_RTC_SDA_1();	/* CPU释放SDA总线 */
}


//不产生ACK应答
void IIC_NAck(void)
{
    /*
    macI2C_RTC_SCL_0();
    //SDA_OUT();
     macI2C_RTC_SDA_1();
    i2c_Delay(2);
    macI2C_RTC_SCL_1();
    i2c_Delay(2);
    macI2C_RTC_SCL_0();
    */

    macI2C_RTC_SDA_1();	/* CPU驱动SDA = 1 */
    i2c_Delay();
    macI2C_RTC_SCL_1();	/* CPU产生1个时钟 */
    i2c_Delay();
    macI2C_RTC_SCL_0();
    i2c_Delay();
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
    //SDA_OUT();
    macI2C_RTC_SCL_0();//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        //IIC_SDA=(txd&0x80)>>7;
        if((txd&0x80)>>7)
             macI2C_RTC_SDA_1();
        else
            macI2C_RTC_SDA_0();
        txd<<=1; 	  
        i2c_Delay();   //对TEA5767这三个延时都是必须的
        macI2C_RTC_SCL_1();
        i2c_Delay(); 
        macI2C_RTC_SCL_0();	
        i2c_Delay();
    }
}
    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
    /*
    unsigned char i,receive=0;
    //SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
    {
        macI2C_RTC_SCL_0(); 
        i2c_Delay(2);
        macI2C_RTC_SCL_1();
        receive<<=1;
        if(macI2C_RTC_SDA_READ())receive++;   
        i2c_Delay(1); 
    }
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
    */
    
    unsigned char i,receive=0;
    //SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
    {
        macI2C_RTC_SCL_0(); 
        i2c_Delay();
        macI2C_RTC_SCL_1();
        receive<<=1;
        if(macI2C_RTC_SDA_READ())receive++;   
        i2c_Delay(); 
    }
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
    
}



















