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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ;   //����Ϊ��©������������������
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	//PB6,PB7 �����

    //ע������������ʽ��ѡ��"��©���"
    //IO���óɿ�©�����Ȼ�������������ʵ����˫��IO��


    /* ��һ��ֹͣ�ź�, ��λI2C�����ϵ������豸������ģʽ */
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
    //SDA_OUT();     //sda�����
     macI2C_RTC_SDA_1();
    i2c_Delay(1);
    macI2C_RTC_SCL_1();
    i2c_Delay(5);
    macI2C_RTC_SDA_0();//START:when CLK is high,DATA change form high to low 
    i2c_Delay(5);
    macI2C_RTC_SCL_0();//ǯסI2C���ߣ�׼�����ͻ�������� 
    i2c_Delay(2);
    */
}  
void IIC_Stop(void)
{
    /*
    //SDA_OUT();//sda�����
    macI2C_RTC_SCL_0();
    macI2C_RTC_SDA_0();//STOP:when CLK is high DATA change form low to high
    i2c_Delay(4);
    macI2C_RTC_SCL_1(); 
    i2c_Delay(5);
     macI2C_RTC_SDA_1();//����I2C���߽����ź�
    i2c_Delay(4);
    */
    macI2C_RTC_SDA_0();
    macI2C_RTC_SCL_1();
    i2c_Delay();
    macI2C_RTC_SDA_1();
    
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
    /*
    u8 ucErrTime=0;
    //SDA_IN();      //SDA����Ϊ����  
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
    macI2C_RTC_SCL_0();//ʱ�����0 	   
    return 0;  
    */

    uint8_t re;

    macI2C_RTC_SDA_1();	/* CPU�ͷ�SDA���� */
    i2c_Delay();
    macI2C_RTC_SCL_1();	/* CPU����SCL = 1, ��ʱ�����᷵��ACKӦ�� */
    i2c_Delay();
    if (macI2C_RTC_SDA_READ())	/* CPU��ȡSDA����״̬ */
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


//����ACKӦ��
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
    macI2C_RTC_SDA_0();	/* CPU����SDA = 0 */
    i2c_Delay();
    macI2C_RTC_SCL_1();	/* CPU����1��ʱ�� */
    i2c_Delay();
    macI2C_RTC_SCL_0();
    i2c_Delay();
    macI2C_RTC_SDA_1();	/* CPU�ͷ�SDA���� */
}


//������ACKӦ��
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

    macI2C_RTC_SDA_1();	/* CPU����SDA = 1 */
    i2c_Delay();
    macI2C_RTC_SCL_1();	/* CPU����1��ʱ�� */
    i2c_Delay();
    macI2C_RTC_SCL_0();
    i2c_Delay();
}

//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
    //SDA_OUT();
    macI2C_RTC_SCL_0();//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        //IIC_SDA=(txd&0x80)>>7;
        if((txd&0x80)>>7)
             macI2C_RTC_SDA_1();
        else
            macI2C_RTC_SDA_0();
        txd<<=1; 	  
        i2c_Delay();   //��TEA5767��������ʱ���Ǳ����
        macI2C_RTC_SCL_1();
        i2c_Delay(); 
        macI2C_RTC_SCL_0();	
        i2c_Delay();
    }
}
    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
    /*
    unsigned char i,receive=0;
    //SDA_IN();//SDA����Ϊ����
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
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK   
    return receive;
    */
    
    unsigned char i,receive=0;
    //SDA_IN();//SDA����Ϊ����
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
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK   
    return receive;
    
}



















