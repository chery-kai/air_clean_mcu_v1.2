#ifndef __EXTERNAL_RTC_H
#define __EXTERNAL_RTC_H

#include "stm32f10x.h"
#include <stdio.h>

/**
 * 使用时钟芯片: DS3231, 参考项目：clock_ok
 */
typedef struct 
{
    char hour;
    char min;
    char sec;
    char w_year;
    char w_month;
    char w_date;
    char week;
    u8 temph;
    u8 templ;
    u8 zhengfu;
}_calendar_obj;

extern _calendar_obj calendar;      //日历结构体
extern u8 const mon_table[12];      //月份日期数据表


void DS3231_Init(void);
void get_show_time(void);      
//u8 RTC_Get_Week(u16 year,u8 month,u8 day);
void DS3231_Set(u8 syear,u8 smon,u8 sday,u8 week,u8 hour,u8 min,u8 sec);//设置时间


void DS3231_Set_year(u8 yea);
void DS3231_Set_mon(u8 mon);
void DS3231_Set_day(u8 day);
void DS3231_Set_week(u8 week);
void DS3231_Set_hour(u8 hour);
void DS3231_Set_min(u8 min);
void DS3231_Set_sec(u8 sec);
#endif /* __EXTERNAL_RTC_H */
