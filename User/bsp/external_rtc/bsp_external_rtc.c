/**
  ******************************************************************************
  * @file    bsp_external_rtc.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   
  ******************************************************************************
  */ 
  
#include "myiic.h"
#include "bsp_external_rtc.h"

_calendar_obj calendar;


#define DS3231_WriteAddress 0xD0   
#define DS3231_ReadAddress  0xD1

u8 BCD2HEX(u8 val)
{
    u8 i;
    i= val&0x0f;
    val >>= 4;
    val &= 0x0f;
    val *= 10;
    i += val;

    return i;
}


u16 B_BCD(u8 val)
{
    u8 i,j,k;
    i=val/10;
    j=val%10;
    k=j+(i<<4);
    return k;
}


void I2cByteWrite(u8 addr,u8 bytedata)
{
    IIC_Start();
    //i2c_Delay(5);
    i2c_Delay();
    IIC_Send_Byte(DS3231_WriteAddress);
    IIC_Wait_Ack();
    //i2c_Delay(5);
    i2c_Delay();
    IIC_Send_Byte(addr);
    IIC_Wait_Ack();
    //i2c_Delay(5);
    i2c_Delay();
    IIC_Send_Byte(bytedata);
    IIC_Wait_Ack();
    //i2c_Delay(5);
    i2c_Delay();
    IIC_Stop();
}


u8 I2cByteRead(u8 addr)
{
    u8 Dat=0;

    IIC_Start();
    IIC_Send_Byte(DS3231_WriteAddress);
    IIC_Wait_Ack();
    //i2c_Delay(5);
    i2c_Delay();
    IIC_Send_Byte(addr);
    IIC_Wait_Ack();
    //i2c_Delay(5);
    i2c_Delay();
    IIC_Start();
    IIC_Send_Byte(DS3231_ReadAddress);
    IIC_Wait_Ack();
    //i2c_Delay(5);
    i2c_Delay();
    Dat=IIC_Read_Byte(1);
    IIC_Stop();
    return Dat;
} 


/**
 * 在 bsp.c 中相关初始化函数, 均为初始化硬件(GPIO)相关
 * 在初始化函数中, 不能有延时相关, 否则程序会挂住(可能的原因: 延时会导致程序切换)
 * 这里的初始化函数在屏幕显示线程中调用, 与氨气传感器的初始化函数处理类似
 */
void DS3231_Init(void)
{
    //IIC_Init();
    I2cByteWrite(0x0e,0x1C);
    //i2c_Delay(2000);
    i2c_Delay();
    I2cByteWrite(0x0f,0x0);
    //i2c_Delay(2000);
    i2c_Delay();    
}

void DS3231_Get(void)
{
    calendar.w_year=I2cByteRead(0x06);  
    calendar.w_month=I2cByteRead(0x05);  
    calendar.w_date=I2cByteRead(0x04); 
    calendar.hour=I2cByteRead(0x02);  
    calendar.min=I2cByteRead(0x01);
    calendar.sec=I2cByteRead(0x00);
    calendar.week=I2cByteRead(0x03);
}


void DS3231_Set_year(u8 yea)
{
    u8 temp=0;
    temp=B_BCD(yea);
    I2cByteWrite(0x06,temp);
}
void DS3231_Set_mon(u8 mon)
{
    u8 temp=0;
    temp=B_BCD(mon);
    I2cByteWrite(0x05,temp);
}
void DS3231_Set_day(u8 day)
{
    u8 temp=0; 
    temp=B_BCD(day);
    I2cByteWrite(0x04,temp);
}
void DS3231_Set_week(u8 week)
{
    u8 temp=0; 
    temp=B_BCD(week);
    I2cByteWrite(0x03,temp);
}
void DS3231_Set_hour(u8 hour)
{
    u8 temp=0; 
    temp=B_BCD(hour);
    I2cByteWrite(0x02,temp);
}
void DS3231_Set_min(u8 min)
{
    u8 temp=0; 
    temp=B_BCD(min);
    I2cByteWrite(0x01,temp);
}
void DS3231_Set_sec(u8 sec)
{
    u8 temp=0; 
    temp=B_BCD(sec);
    I2cByteWrite(0x00,temp);
}

void DS3231_Set(u8 yea,u8 mon,u8 da,u8 week,u8 hou,u8 min,u8 sec)
{
    u8 temp=0;

    temp=B_BCD(yea);
    I2cByteWrite(0x06,temp);

    temp=B_BCD(mon);
    I2cByteWrite(0x05,temp);

    temp=B_BCD(da);
    I2cByteWrite(0x04,temp);

    temp=B_BCD(week);
    I2cByteWrite(0x03,temp);

    temp=B_BCD(hou);
    I2cByteWrite(0x02,temp);

    temp=B_BCD(min);
    I2cByteWrite(0x01,temp);

    temp=B_BCD(sec);
    I2cByteWrite(0x00,temp);
}


void get_show_time(void)
{
    calendar.w_year=I2cByteRead(0x06);  
    calendar.w_year=BCD2HEX(calendar.w_year);

    calendar.w_month=I2cByteRead(0x05); 
    calendar.w_month=BCD2HEX(calendar.w_month);

    calendar.w_date=I2cByteRead(0x04);  
    calendar.w_date=BCD2HEX(calendar.w_date);

    calendar.hour=I2cByteRead(0x02); 
    calendar.hour&=0x3f;                   
    calendar.hour=BCD2HEX(calendar.hour);

    calendar.min=I2cByteRead(0x01);
    calendar.min=BCD2HEX(calendar.min);

    calendar.sec=I2cByteRead(0x00);
    calendar.sec=BCD2HEX(calendar.sec);

    calendar.week=I2cByteRead(0x03);
    calendar.week=BCD2HEX(calendar.week);


    calendar.temph=I2cByteRead(0x11);
    if(calendar.temph&0x80)
        calendar.zhengfu = 1;
    else
        calendar.zhengfu = 0;

    calendar.temph=I2cByteRead(0x11);
    calendar.temph=BCD2HEX(calendar.temph);
    calendar.templ=I2cByteRead(0x12);
    calendar.templ=BCD2HEX(calendar.templ);
}


/*********************************************END OF FILE**********************/
