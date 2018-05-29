
#include "appfunc.h"

/*
*********************************************************************************************************
*                                           全局变量定义
*********************************************************************************************************
*/

//每隔5秒采集一次
#define COLLECT_NUM 36                  //获取3分钟内的NH3平均浓度值
int nh3_collect[COLLECT_NUM];           //NH3采集, COLLECT_NUM=36:每隔5秒采集一次
                                        //若要每隔30秒采集一次, 可设置 COLLECT_NUM=6

char plant_icon_flick_flag = 0;             //植物液图标闪烁标志
char head_dig_flick_flag = 0;               //前两个数码管闪烁标志
char tail_dig_flick_flag = 0;               //后两个数码管闪烁标志
char sepa_dig_flick_flag = 0;               //数码管中间分隔符闪烁标志

char mode_icon_flick_flag = 0;              //模式图标闪烁标志
char atomizer_icon_flick_flag = 0;          //雾化器图标闪烁标志(A/B闪)
char fan_icon_flick_flag = 0;               //风扇图标闪烁标志
char wash_icon_flick_flag = 0;              //清洗图标闪烁标志
char net_icon_flick_flag = 0;               //网络图标闪烁标志


/*
*********************************************************************************************************
*                                           内部Flash读写操作
*********************************************************************************************************
*/
/******************************************************************************
 * FunctionName : configure_data_exists
 * Description  : 检查Flash中是否有配置数据
 * Parameters   : None
 * Returns      : 0(Flash 中没有配置数据); 1(Flash 中已有配置数据)
*******************************************************************************/
char configure_data_exists()
{
    air_clean_cfg_st tmp_cfg;
    unsigned char temp[512] = {'\0'};
    
    ReadFlashNBtye(0, (uint8_t*)temp, sizeof(temp));
    memcpy((unsigned char *)&tmp_cfg, temp, sizeof(air_clean_cfg_st));
    
    if(0x5A == tmp_cfg.cfg_flash_flag)
    {
        return 1;
    }
    
    return 0;
}

//从内部Flash读取配置文件
void read_cfg_from_flash()
{
    unsigned char temp[512] = {'\0'};
    //memset(temp, 0, sizeof(temp));
    ReadFlashNBtye(0, (uint8_t*)temp, sizeof(temp));
    //memcpy((char *)&air_clean_cfg, temp, sizeof(air_clean_cfg_st));
    memcpy((unsigned char *)&air_clean_cfg, temp, sizeof(air_clean_cfg_st));
}

//写入配置文件到内部Flash
void write_cfg_to_flash()
{
    uint32_t EraseCounter = 0x00;   //记录要擦除多少页
    uint32_t Address = 0x00;        //记录写入的地址
    uint32_t NbrOfPage = 0x00;      //记录写入多少页
    FLASH_Status FLASHStatus = FLASH_COMPLETE;  //记录每次擦除的结果
    
    int i;
    unsigned char temp[4] = {'\0'};
    
    unsigned char temp_air_clean[512];
    memset(temp_air_clean, 0, sizeof(temp_air_clean));
    memcpy(temp_air_clean, (unsigned char *)&air_clean_cfg, sizeof(air_clean_cfg_st));
    
    //WS_DEBUG("[write_cfg_to_flash] 开始执行写配置数据到Flash中 \r\n");
    
    //写 Flash 过程:
    //开始 ************************************************************
    //解锁
    FLASH_Unlock();
    
    //计算要擦除多少页  //FLASH_PAGE_SIZE = 2048 
    //只写512字节, 这里设置 CONF_END_ADDR/CONF_START_ADDR 只擦除一页
    NbrOfPage = (CONF_END_ADDR - CONF_START_ADDR) / FLASH_PAGE_SIZE;

    //清空所有标志位
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    //按页擦除
    for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        FLASHStatus = FLASH_ErasePage(CONF_START_ADDR + (FLASH_PAGE_SIZE * EraseCounter));
    }

    //向内部FLASH写入数据
    Address = CONF_START_ADDR;
    
    for(i=0; (i<(512/4)) && (FLASHStatus == FLASH_COMPLETE); i++)
    {
        memset(temp, 0, sizeof(temp));
        memcpy(temp, temp_air_clean+4*i, sizeof(temp));
        //这样转换是否会有 "存储大小端序" 问题？？
        //WS_DEBUG("*(int*)temp = %d", *(int*)temp);
        
        FLASHStatus = FLASH_ProgramWord(Address, *(int*)temp);
        //FLASHStatus = FLASH_ProgramWord(Address, 0x1234);
        Address = Address + 4;
    }
    FLASH_Lock();
    
    //WS_DEBUG("3333333333333333###### write_cfg_to_flash \r\n");
    //结束 ************************************************************
}

//测试 Flash 
void test_flash()
{
    date_record_st date_record;
    date_record.start_year = 17;
    date_record.start_month = 9;
    date_record.start_day = 20;
    air_clean_cfg.atomizer_start_time = date_record;
    air_clean_cfg.standard_mode_open_min = 66;
    air_clean_cfg.standard_mode_close_min = 999;
    air_clean_cfg.err_code_bitmap = 1213;

    air_clean_cfg.cfg_flash_flag = 0x5A;
    write_cfg_to_flash();
    read_cfg_from_flash();

    WS_DEBUG("air_clean_cfg.standard_mode_open_min = %d\r\n", air_clean_cfg.standard_mode_open_min);
    WS_DEBUG("air_clean_cfg.standard_mode_close_min = %d\r\n", air_clean_cfg.standard_mode_close_min);
    WS_DEBUG("air_clean_cfg.err_code_bitmap = %d\r\n", air_clean_cfg.err_code_bitmap);
    WS_DEBUG("air_clean_cfg.atomizer_start_time.start_day = %d\r\n", air_clean_cfg.atomizer_start_time.start_day);
    WS_DEBUG("air_clean_cfg.cfg_flash_flag = 0x%X\r\n", air_clean_cfg.cfg_flash_flag);
}


/*
*********************************************************************************************************
*                                           系统初始化相关函数定义
*********************************************************************************************************
*/

//配置数据初始化
//调用情形: 1.配置数据不存在; 2.设备复位时
//注意: 需要对 air_clean_cfg_st 结构体中的每个成员初始化
void cfg_data_init()
{
    //根据 air_clean_cfg_st 结构体定义进行初始化
    //无论智能模式(自定义模式)标准模式，复位后统一为07：30-17:00开3停7 周一到周5开 周六周日关
    init_standard_mode();
    air_clean_cfg.standard_mode_open_min = DEFAULT_OPEN_MIN;
    air_clean_cfg.standard_mode_close_min = DEFAULT_CLOSE_MIN;
    
    //初始化"自定义模式" -- 这里需设置为默认值
    memset((void *)&air_clean_cfg.st_custom_mode, 0, sizeof(air_clean_cfg.st_custom_mode));
    air_clean_cfg.st_custom_mode.custom_time_plan[0].start_hour = DEFAULT_START_HOUR;
    air_clean_cfg.st_custom_mode.custom_time_plan[0].start_minute = DEFAULT_START_MIN;
    air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_hour = DEFAULT_STOP_HOUR;
    air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_minute = DEFAULT_STOP_MIN;
    air_clean_cfg.st_custom_mode.custom_open_min = DEFAULT_OPEN_MIN;       //开5分钟
    air_clean_cfg.st_custom_mode.custom_close_min = DEFAULT_CLOSE_MIN;      //停5分钟
    //
    memset(st_mode_setting.custom_work_week, 0, sizeof(st_mode_setting.custom_work_week));
    air_clean_cfg.st_custom_mode.custom_work_week[1] = 1;
    air_clean_cfg.st_custom_mode.custom_work_week[2] = 1;
    air_clean_cfg.st_custom_mode.custom_work_week[3] = 1;
    air_clean_cfg.st_custom_mode.custom_work_week[4] = 1;
    air_clean_cfg.st_custom_mode.custom_work_week[5] = 1;
    
    air_clean_cfg.power_onoff = OFF;                        //这里初始化为0, 即电源为关闭状态
    air_clean_cfg.work_mode = MODE_STANDARD;                //初始化为标准模式
    air_clean_cfg.which_atomizer = ATOMIZER_ONE;            //初始化使用雾化器1
    
    //2018.02.22 : 出厂默认模式风量2浓度2
    air_clean_cfg.air_fan_level = AIR_FAN_LEVEL_TWO;        //风量初始化为等级2
    air_clean_cfg.liquid_density = LIQUID_DENSITY_TWO;      //浓度初始化为2档
    air_clean_cfg.err_code_bitmap = 0;                      //
    air_clean_cfg.org_low_to_work_count = 0;                //
    
    
    //更新清洗间隔时间
    air_clean_cfg.dev_work_time.start_year = calendar.w_year;
    air_clean_cfg.dev_work_time.start_month = calendar.w_month;
    air_clean_cfg.dev_work_time.start_day = calendar.w_date;
    
    //更新雾化器工作时间
    air_clean_cfg.atomizer_start_time.start_year = calendar.w_year;
    air_clean_cfg.atomizer_start_time.start_month = calendar.w_month;
    air_clean_cfg.atomizer_start_time.start_day = calendar.w_date;
    //"强制清洗/雾化器切换"重新计时, 设为当前时间(下面两行代码亦可)
    //update_cycle_time(ATOMIZER_INTERVAL_TYPE);
    //update_cycle_time(WASH_INTERVAL_TYPE);
    
    air_clean_cfg.wash_mode = 0;                            //初始化为自动清洗
    air_clean_cfg.atomizer_lock = 0;                        //雾化器初始化为未锁定状态
    air_clean_cfg.cfg_flash_flag = 0x5A;                    //固定标识
    air_clean_cfg.onekey_drain_fished = 0;                  //清除"一键排液完成标识"
    
    air_clean_cfg.cfg_data_init_flag = 1;                   //复位标识置为1
    
    write_cfg_to_flash();
}

int get_err_code()
{
    return air_clean_cfg.err_code_bitmap;
}


/*****************************************************************************/
/* 标准模式 */
//设置某一天是否工作
char std_set_day_plan_onoff(char week_day, char onoff)
{
    air_clean_cfg.st_standard_mode.week_plan[week_day].today_on_off = onoff;
}

//设置某一天工作的时间段
char std_set_time_plan(char which_day, char which_time, 
        char start_hour, char start_minute, char stop_hour, char stop_minute)
{
    air_clean_cfg.st_standard_mode.week_plan[which_day].
            time_plan[which_time].start_hour = start_hour;
    air_clean_cfg.st_standard_mode.week_plan[which_day].
            time_plan[which_time].start_minute = start_minute;
    air_clean_cfg.st_standard_mode.week_plan[which_day].
            time_plan[which_time].stop_hour = stop_hour;
    air_clean_cfg.st_standard_mode.week_plan[which_day].
            time_plan[which_time].stop_minute = stop_minute;
}

//初始化标准模式
//出厂模式：周一--周五工作，07:30-17:00工作，雾化器开3分停7分
void init_standard_mode()
{
    std_set_day_plan_onoff(SUNDAY,OFF);
    std_set_day_plan_onoff(MONDAY,ON);
    std_set_day_plan_onoff(TUESDAY,ON);
    std_set_day_plan_onoff(WEDNESDAY,ON);
    std_set_day_plan_onoff(THURSDAY,ON);
    std_set_day_plan_onoff(FRIDAY,ON);
    std_set_day_plan_onoff(SATURDAY,OFF);

    std_set_time_plan(SUNDAY,0,7,30,12,30);
    std_set_time_plan(SUNDAY,1,12,31,17,00);

    std_set_time_plan(MONDAY,0,7,30,12,30);
    std_set_time_plan(MONDAY,1,12,31,17,00);

    std_set_time_plan(TUESDAY,0,7,30,12,30);
    std_set_time_plan(TUESDAY,1,12,31,17,00);

    std_set_time_plan(WEDNESDAY,0,7,30,12,30);
    std_set_time_plan(WEDNESDAY,1,12,31,17,00);

    std_set_time_plan(THURSDAY,0,7,30,12,30);
    std_set_time_plan(THURSDAY,1,12,31,17,00);

    std_set_time_plan(FRIDAY,0,7,30,12,30);
    std_set_time_plan(FRIDAY,1,12,31,17,00);

    std_set_time_plan(SATURDAY,0,7,30,12,30);
    std_set_time_plan(SATURDAY,1,12,31,17,00);
}

/*
*********************************************************************************************************
*                                           AirCleanTask相关函数定义
*********************************************************************************************************
*/

/**
 * 函数功能: 
 *      更新"原液箱到低液位后还能抽液到工作仓的次数"
 * 液箱液位传感器到达最低液位，不作为控制设备停止的条件，到达最低液位开始计数，
 * 最淡1:240浓度为450次，中间1:180浓度为330次，最浓1:120浓度为220次。
 */
void update_org_low_to_work_count()
{
    if(LIQUID_DENSITY_ONE == air_clean_cfg.liquid_density){
        if(DENSITY_ONE_ORGTOWORK_COUT != air_clean_cfg.org_low_to_work_count)
        {
            air_clean_cfg.org_low_to_work_count = DENSITY_ONE_ORGTOWORK_COUT;       //低档
            write_cfg_to_flash();
        }
    }else if(LIQUID_DENSITY_TWO == air_clean_cfg.liquid_density){
        if(DENSITY_TWO_ORGTOWORK_COUT != air_clean_cfg.org_low_to_work_count)
        {
            air_clean_cfg.org_low_to_work_count = DENSITY_TWO_ORGTOWORK_COUT;       //中档
            write_cfg_to_flash();
        }
    }else if(LIQUID_DENSITY_THREE == air_clean_cfg.liquid_density){
        if(DENSITY_THREE_ORGTOWORK_COUT != air_clean_cfg.org_low_to_work_count)
        {
            air_clean_cfg.org_low_to_work_count = DENSITY_THREE_ORGTOWORK_COUT;     //高档
            write_cfg_to_flash();
        }
    }else{
        air_clean_cfg.org_low_to_work_count = DENSITY_THREE_ORGTOWORK_COUT;         //默认高档抽液次数
    }
}



//设置NH3浓度(从氨气传感器获取浓度值, 取平均值)
void set_nh3_concentration()
{
    int i;
    int nh3_value;
    int aver = 0, total = 0;
    int sum, num;
    
    char *str = NULL;
    float nh3_mg_m3;        //换算成 mg/m3 的氨气浓度值
    
    //氨气传感器读取到浓度值
    //nh3_value = NH3_Sensor_Request();
    OS_ERR err; 
    NH3_Sensor_Value_Request();
    OSTimeDlyHMSM(0,0,0,600,OS_OPT_TIME_HMSM_STRICT,&err);
    nh3_value = NH3_Sensor_Value_Read();
    
    WS_DEBUG("[set_nh3_concentration] 串口读取氨气传感器浓度值(ppm) : %d \r\n", nh3_value);
    
    //返回-1, 代表获取氨气值出错, 作为"氨气传感器故障"的依据
    if(-1 == nh3_value)
    {
        st_current_status.nh3_err_flag +=1;
        nh3_value = 0;
    }
    else
    {
        st_current_status.nh3_err_flag =0;
    }
    
    //从氨气传感器获取的值进行换算
    /*
    1ppm=M/22.4mg/m3
    M为该气体的相对分子质量

    NH3：
    1ppm=17/22.4=0.758mg/m3

    反馈的数据为整数数据，需要乘以分辨率的，比如你读到数据为，01 F4，转换为十进制应为500，实际值为500*0.01=5.00ppm

    整个模组量程0-50ppm
    */
    //将氨气传感器获取到的实时值赋给 nh3_show_real_value, 用于屏幕显示 
    st_current_status.nh3_show_real_value = nh3_value*0.01*0.758;
    
    
    for(i=COLLECT_NUM-1; i>0; i--)
    {
        nh3_collect[i] = nh3_collect[i-1];
    }
    nh3_collect[0] = nh3_value;
    
    /*
    for(i=0; i<COLLECT_NUM; i++)
    {
        WS_DEBUG("[set_nh3_concentration] nh3_collect[%d] = %d \r\n", i, nh3_collect[i]);
    }
    */
    
    sum = num = 0;
    for(i=0; i<COLLECT_NUM; i++)
    {
        if(nh3_collect[i] != 0)
        {
            sum += nh3_collect[i];
            num++;
        }
    }
    //氨气浓度平均值
    if(sum != 0 && num != 0)
    {
        aver = sum/num;
    }
    else
    {
        aver = 0;
    }
    
    st_current_status.current_nh3_ppm = aver;
    
    WS_DEBUG("[set_nh3_concentration] 当前氨气浓度平均值为：%d \r\n", aver);
    
    //换算
    //nh3_mg_m3 = aver*0.01*0.758;
    //st_current_status.current_nh3_value = nh3_mg_m3;    //current_nh3_value 无需使用, 这里先保留
}

//控制雾化器开关
void atomizer_onoff(char which_atomizer, char onoff)
{
    switch(which_atomizer)
    {
        case ATOMIZER_ONE:  //雾化器A
            ATOMIZER_A_ONOFF(onoff);
            break;
            
        case ATOMIZER_TWO:  //雾化器B
            ATOMIZER_B_ONOFF(onoff);
            break;
        
        case ATOMIZER_ALL:  //雾化器A && 雾化器B
            ATOMIZER_A_ONOFF(onoff);
            ATOMIZER_B_ONOFF(onoff);
            break;
    }
}

//控制液泵开关
void liquid_pump_onoff(char which_pump, char onoff)
{
    switch(which_pump)
    {
        case PUMP_TO_ORG:  //液泵1
            PUMP1_ONOFF(onoff);
            break;
            
        case PUMP_TO_WORK:  //液泵2
            PUMP2_ONOFF(onoff);
            break;
        
        case PUMP_ALL:  //液泵1 && 液泵2
            PUMP1_ONOFF(onoff);
            PUMP2_ONOFF(onoff);
            break;
    }
}

//控制电磁阀开关 
void solenoid_onoff(char which_solenoid, char onoff)
{
    switch(which_solenoid)
    {
        case SOLENOID_INTO_WORK:  //电磁阀1
            SOLENOID1_ONOFF(onoff);
            break;
            
        case SOLENOID_OUT_WORK:  //电磁阀2
            SOLENOID2_ONOFF(onoff);
            break;
        
        case SOLENOID_ALL:  //电磁阀1 && 电磁阀2
            SOLENOID1_ONOFF(onoff);
            SOLENOID2_ONOFF(onoff);
            break;
    }
}

//设置风扇的风量等级 
void set_air_fan_level()
{
    //两个风扇, 同时设置风量等级
    switch(air_clean_cfg.air_fan_level)
    {
        case AIR_FAN_LEVEL_ONE:         //风量等级1
            //TIM3 CH3
            TIM_SetCompare3(TIM4,20);
            //GENERAL_TIM_Init(25);     //占空比 80% (风量最小)    -- 30
            break;
            
        case AIR_FAN_LEVEL_TWO:         //风量等级2
            TIM_SetCompare3(TIM4,15);
            //GENERAL_TIM_Init(15);     //占空比 50% (风量中等)    -- 20
            break;
        
        case AIR_FAN_LEVEL_THREE:       //风量等级3
            TIM_SetCompare3(TIM4,10);
            //GENERAL_TIM_Init(10);     //占空比 30% (风量最大)    -- 12
            break;
    }
}

//控制风扇开关
void air_fan_onoff(char onoff)
{
    OS_ERR err;
    FAN_ONOFF(onoff);
    
    //两个风扇, 同时开关
    if(onoff == ON)
    {
        //WS_DEBUG("[air_fan_onoff] GENERAL_TIM_Init-->set_air_fan_level\r\n");
        GENERAL_TIM_Init();
        set_air_fan_level();
    }
    else
    {
    #if RELAY_EDITION
        CLOSE_FAN_Config();
    #else
        DISABLE_PWM_TIM();
    #endif
    }
}

//通过与RTC时间进行比较, 来确定当前时间设备运行还是停止
int get_whole_dev_onoff()
{
    char week_day,hour,minute;
    int rtc_minute = 0;
    
    week_day = calendar.week;   //周几
    hour = calendar.hour;       //小时
    minute = calendar.min;      //分钟
    rtc_minute = hour*60+minute;
    
    
    //自定义模式比对
    if(MODE_CUSTOM == air_clean_cfg.work_mode)
    {
        int i;
        int c_start_minute =0;
        int c_stop_minute = 0;
        
        //WS_DEBUG("\r\n [get_whole_dev_onoff] MODE_CUSTOM: air_clean_cfg.st_custom_mode.custom_work_week[%d] = %d ...\n\r", 
        //        week_day, air_clean_cfg.st_custom_mode.custom_work_week[week_day]);
        
        if(1 == air_clean_cfg.st_custom_mode.custom_work_week[week_day])    //代表这一天工作
        {
            for(i=0; i<10; i++)
            {
                c_start_minute = air_clean_cfg.st_custom_mode.custom_time_plan[i].start_hour*60 +
                        air_clean_cfg.st_custom_mode.custom_time_plan[i].start_minute;
                c_stop_minute = air_clean_cfg.st_custom_mode.custom_time_plan[i].stop_hour*60 +
                        air_clean_cfg.st_custom_mode.custom_time_plan[i].stop_minute;
                if(c_start_minute >= 0 && c_start_minute < c_stop_minute)
                {
                    //WS_DEBUG("\r\n [get_whole_dev_onoff] MODE_CUSTOM: start_hour = %d, start_minute = %d, stop_hour = %d, stop_minute = %d...\n\r", air_clean_cfg.st_custom_mode.custom_time_plan[i].start_hour,air_clean_cfg.st_custom_mode.custom_time_plan[i].start_minute,air_clean_cfg.st_custom_mode.custom_time_plan[i].stop_hour,air_clean_cfg.st_custom_mode.custom_time_plan[i].stop_minute);
                    //WS_DEBUG("\r\n [get_whole_dev_onoff] MODE_CUSTOM: c_start_minute = %d, c_stop_minute = %d, rtc_minute = %d ...\n\r", c_start_minute,c_stop_minute,rtc_minute);
                    
                    if(rtc_minute >= c_start_minute && rtc_minute < c_stop_minute) 
                    {
                        //WS_DEBUG("\r\n ********** [get_whole_dev_onoff] MODE_CUSTOM ON ... **********\n\r");
                        return 1;
                    }
                }
            }
        }
    }
    //标准模式比对
    else
    {
        char plan_hour_start, plan_hour_stop, plan_minute_start,plan_minute_stop;
        day_plan_st * day_plan;
        
        /*
        //day_plan = &(air_clean_cfg.mode_data_array[air_clean_cfg.work_mode].week_plan[week_day]);
        //本地模式一 --> mode_data_array[0]; 本地模式二 --> mode_data_array[1]
        if(7 == week_day)   //周日 --> week_plan[0]; 周一 --> week_plan[1]
            day_plan = &(air_clean_cfg.mode_data_array[air_clean_cfg.work_mode-1].week_plan[0]);
        else
            day_plan = &(air_clean_cfg.mode_data_array[air_clean_cfg.work_mode-1].week_plan[week_day]);
        */
        
        if(7 == week_day)   //周日 --> week_plan[0]; 周一 --> week_plan[1]
            day_plan = &(air_clean_cfg.st_standard_mode.week_plan[0]);
        else
            day_plan = &(air_clean_cfg.st_standard_mode.week_plan[week_day]);
        
        /*
        WS_DEBUG("[get_whole_dev_onoff] time_plan[0]: %d,%d,%d,%d\n\r",
                day_plan->time_plan[0].start_hour,
                day_plan->time_plan[0].stop_hour,
                day_plan->time_plan[0].start_minute,
                day_plan->time_plan[0].stop_minute
                );
        WS_DEBUG("[get_whole_dev_onoff] time_plan[1]: %d,%d,%d,%d\n\r",
                day_plan->time_plan[1].start_hour,
                day_plan->time_plan[1].stop_hour,
                day_plan->time_plan[1].start_minute,
                day_plan->time_plan[1].stop_minute
                );
        WS_DEBUG("[get_whole_dev_onoff] day_plan->today_on_off : %d\n\r",day_plan->today_on_off);
        */

        if(day_plan->today_on_off) 
        {
            int start_minute =0;
            int stop_minute = 0;
            
            //比对上午工作时间
            plan_hour_start = day_plan->time_plan[0].start_hour;
            plan_hour_stop = day_plan->time_plan[0].stop_hour;
            plan_minute_start = day_plan->time_plan[0].start_minute;
            plan_minute_stop = day_plan->time_plan[0].stop_minute;
            
            start_minute = plan_hour_start*60+plan_minute_start;
            stop_minute = plan_hour_stop*60+plan_minute_stop;
            
            //WS_DEBUG("rtc_minute: %d, start_minute: %d,stop_minute: %d \n\r",
            //      rtc_minute,start_minute,stop_minute);
            if(rtc_minute >= start_minute && rtc_minute <= stop_minute) {
                //WS_DEBUG("[get_whole_dev_onoff] time_plan[0] is ON\r\n");
                return 1;
            }
            
            //比对下午工作时间
            plan_hour_start = day_plan->time_plan[1].start_hour;
            plan_hour_stop = day_plan->time_plan[1].stop_hour;
            plan_minute_start = day_plan->time_plan[1].start_minute;
            plan_minute_stop = day_plan->time_plan[1].stop_minute;
            
            start_minute = plan_hour_start*60+plan_minute_start;
            stop_minute = plan_hour_stop*60+plan_minute_stop;

            if(rtc_minute >= start_minute && rtc_minute < stop_minute) {
                //WS_DEBUG("[get_whole_dev_onoff] time_plan[1] is ON\r\n");
                return 1;
            }
        }
    }
    //WS_DEBUG("[get_whole_dev_onoff] get_whole_dev_onoff is OFF\r\n");
    //WS_DEBUG("[get_whole_dev_onoff] Enter out get_whole_dev_onoff Func\r\n");
    
    return 0;
}

/*
*******************************************************************************
*   函 数 名: close_all_device
*   功能说明: 关闭所有外部设备
*       //当雾化器停止工作后, 所有风扇需要延迟5秒停止 
*   形    参：无
*   返 回 值: 无
*******************************************************************************
*/
//检查所有外设状态并关闭所有外设 -- 如何处理延迟5秒关闭风扇??
int close_all_device()
{
    if(!is_running)
        return 1;

    //关闭两个电磁阀
    solenoid_onoff(SOLENOID_ALL, OFF);

    //关闭两个液泵
    liquid_pump_onoff(PUMP_ALL, OFF);

    //关闭两个雾化器
    atomizer_onoff(ATOMIZER_ALL,OFF);
    
    //关闭风扇(风扇需延迟5秒停止)
    //air_fan_onoff(OFF);
    st_current_status.close_fan_flag = 1;

    is_running = 0;
    return 0;
}

//设备开始工作
int dev_start_work()
{
    if(is_running)
        return 1;
    
    if(!get_whole_dev_onoff())
    {
        return 0;
    }
    
    if( (ON == st_current_status.current_wash_status) || 
            (ON == st_current_status.onekey_drain_status) || 
            (OFF == air_clean_cfg.power_onoff))
    {
        return 0;
    }
    
    //启动雾化器
    WS_DEBUG("[dev_start_work] 执行启动雾化器操作: atomizer_onoff ON \r\n");
    atomizer_onoff(air_clean_cfg.which_atomizer,ON);
    
    if(1 == st_current_status.close_fan_flag)
    {
        st_current_status.close_fan_flag = 0;
    }
    //启动风扇
    air_fan_onoff(ON);

    is_running = 1;
    
    return 0;
}


//设备停止工作的定义：雾化器、风扇、配比泵、进水阀不工作
int dev_stop_work()
{
    if(!is_running)
        return 1;

    //关闭进水阀
    solenoid_onoff(SOLENOID_INTO_WORK, OFF);

    //关闭配比泵
    liquid_pump_onoff(PUMP_TO_WORK, OFF);

    //关闭两个雾化器
    atomizer_onoff(ATOMIZER_ALL,OFF);
    
    //关闭风扇(风扇需延迟5秒停止)
    //air_fan_onoff(OFF);
    st_current_status.close_fan_flag = 1;

    is_running = 0;
}


//"一键出雾" 功能对应的 "开始工作"
int atomizer_start_work()
{
    if(is_running)
        return 1;
    
    //正在清洗 / 正在一键排液 ，不执行开始工作
    if( (ON == st_current_status.current_wash_status) || 
            (ON == st_current_status.onekey_drain_status) || 
            (OFF == air_clean_cfg.power_onoff))
        return 0;
    
    //启动雾化器
    WS_DEBUG("[atomizer_start_work] 执行启动雾化器操作: atomizer_onoff ON \r\n");
    atomizer_onoff(air_clean_cfg.which_atomizer,ON);
    
    if(1 == st_current_status.close_fan_flag)
    {
        st_current_status.close_fan_flag = 0;
    }
    //启动风扇
    air_fan_onoff(ON);

    is_running = 1;
    
    return 0;
}

/*
*******************************************************************************
*   函 数 名: add_plant_liquid
*   功能说明: 添加植物液
*       加液过程:
*           遥控器“加液”按钮可控制开始加液和停止加液, 当按动“加液”按钮时, 启动
*           加液程序, 加液泵开始抽液, 抽液保护时间为210秒, 当液位传感器到达最
*           高液位点, 自动停止加液程序, 关闭加液泵, 加液完成
*           //加液超时不设置故障码, 可以从屏幕显示看出来
*   形    参：无
*   返 回 值: 无
*******************************************************************************
*/
int add_plant_liquid() 
{
    OS_ERR err;
    int overtime = 0;       //超时时间
    
    WS_DEBUG("[add_plant_liquid] 开始执行 add_plant_liquid(添加植物液) 函数 \r\n");
    
    //若植物液为高液位, 则不执行加液相关操作
    if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_HIGH) == PULLDOWN)
    {
        WS_DEBUG("[add_plant_liquid] 液箱已位于高液位，无需执行添加植物液操作，非正常退出 \r\n");
        st_current_status.add_plant_liquid_flag = 0;                    //加液完成, 设回状态
        BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);   //清除必须加液的错误码 E7
    }
    else
    {
        dev_stop_work();                        //设备停止工作
        liquid_pump_onoff(PUMP_TO_ORG, ON);     //打开"加液泵"
        while(1)
        {
            //加液过程中, 检查达到中液位时更新状态
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_MID) == PULLDOWN)
            {
                WS_DEBUG("[add_plant_liquid] 添加植物液达到中液位(LIQUID_LEVEL_SENSOR_ORG_MID)...\r\n");
                st_current_status.current_plant_liquid = PLANT_LIQUID_MID;
                BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);   //清除必须加液的错误码 E7
            }
            
            //加液过程中, 检查达到高液位时更新状态, 此时加液完成跳出循环
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_HIGH) == PULLDOWN)
            {
                WS_DEBUG("[add_plant_liquid] 添加植物液达到高液位(LIQUID_LEVEL_SENSOR_ORG_HIGH)...\r\n");
                st_current_status.current_plant_liquid = PLANT_LIQUID_HIGH;
                BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);   //清除必须加液的错误码 E7
                WS_DEBUG("[add_plant_liquid] 添加植物液完成，正常退出 \r\n");
                break;
            }
            
            //加液过程中: (1)清洗键应该无效; (2)电源键/加液键, 停止加液过程
            if((OFF == air_clean_cfg.power_onoff) || 
                    (0 == st_current_status.add_plant_liquid_flag)) 
            {
                WS_DEBUG("[add_plant_liquid] 被动停止添加植物液操作(电源关闭或清除add_plant_liquid_flag标识)，非正常退出 \r\n");
                break;
            }
            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时1秒 
            
            overtime++;
            if(overtime >= ADD_LIQUID_SAFE_TIME) 
            {
                //这里报错: 液箱加液异常 只有仍在最低液位点再报错
                if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) == PULLDOWN)
                {
                    WS_DEBUG("[add_plant_liquid] 加液出现异常，液箱仍在最低液位，设置ERR_ORG_ADD_LIQUID错误码 \r\n");
                    BIT_SET(air_clean_cfg.err_code_bitmap,ERR_ORG_ADD_LIQUID);
                }
                
                WS_DEBUG("[add_plant_liquid] 添加植物液到达加液保护时间，非正常退出 \r\n");
                break;
            }
        }
        
        liquid_pump_onoff(PUMP_TO_ORG, OFF);            //关闭"加液泵"
        st_current_status.add_plant_liquid_flag = 0;    //加液完成, 设回状态
    }
    
    return 0;
}


#if MANUAL_EDITION
//手动版加液
/*
与自动版加液的区别:
    1.手动版加液保护时间为4分30秒(270秒)
    2.在加液保护时间内，若加液加到液箱液位传感器最高点后，需要继续加液20秒后再执行抽配比液到工作仓
    若超过加液保护时间，则直接退出执行抽配比液到工作仓
*/
#define KEEP_ADD_PLANT_TIME     20
int manual_add_plant_liquid() 
{
    OS_ERR err;
    
    int overtime = 0;       //超时时间
    
    int keeptime = 0;       //继续加液时间
    char org_high_flag = 0; //原液箱到达高液位的标识
    
    
    //若植物液为高液位, 则不执行加液相关操作
    if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_HIGH) == PULLDOWN)
    {
        st_current_status.add_plant_liquid_flag = 0;    //加液完成, 设回状态
        BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);           //清除必须加液的错误码 E7
    }
    else
    {
        dev_stop_work();
        liquid_pump_onoff(PUMP_TO_ORG, ON);             //打开"加液泵"
        while(1)
        {
            //加液过程中, 检查达到中液位时更新状态
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_MID) == PULLDOWN)
            {
                //WS_DEBUG("[manual_add_plant_liquid] LIQUID_LEVEL_SENSOR_ORG_MID ...\r\n");
                st_current_status.current_plant_liquid = PLANT_LIQUID_MID;
                BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);   //清除必须加液的错误码 E7
            }
            
            //加液过程中, 检查达到高液位时更新状态
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_HIGH) == PULLDOWN)
            {
                //WS_DEBUG("[manual_add_plant_liquid] LIQUID_LEVEL_SENSOR_ORG_HIGH ...\r\n");
                st_current_status.current_plant_liquid = PLANT_LIQUID_HIGH;
                BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);   //清除必须加液的错误码 E7
                //break;
                org_high_flag = 1;
            }
            
            //加液过程中: (1)清洗键应该无效; (2)电源键/加液键, 停止加液过程
            if((OFF == air_clean_cfg.power_onoff) || 
                    (0 == st_current_status.add_plant_liquid_flag)) {
                break;
            }
            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时1秒 
            
            //加液加到液箱液位传感器最高点后, 需要继续加液20秒
            if(1 == org_high_flag)
            {
                keeptime ++;
                if(keeptime >= KEEP_ADD_PLANT_TIME)
                {
                    break;
                }
            }
            
            //手动版加液保护时间: 270
            overtime++;
            if(overtime >= MANUAL_ADD_LIQUID_SAFE_TIME) 
            {
                liquid_pump_onoff(PUMP_TO_ORG, OFF);
                //这里报错: 液箱加液异常 只有仍在最低液位点再报错
                if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) == PULLDOWN)
                {
                    WS_DEBUG("[manual_add_plant_liquid] 手动版加液保护时间超时, 设置 ERR_ORG_ADD_LIQUID 故障码 \r\n");
                    BIT_SET(air_clean_cfg.err_code_bitmap,ERR_ORG_ADD_LIQUID);
                }
                st_current_status.add_plant_liquid_flag = 0;
                return -1;
            }
        }
        liquid_pump_onoff(PUMP_TO_ORG, OFF);            //关闭"加液泵"
        st_current_status.add_plant_liquid_flag = 0;    //加液完成, 设回状态
    }
    
    return 0;
}
#endif

/*
*******************************************************************************
*   函 数 名: do_wash
*   功能说明: 清洗
*       清洗过程:
*           1.打开排水电磁阀, 工作仓开始放液, 放至最低液位, 延时100秒, 关闭排水
*               电磁阀
*           2.打开进水电磁阀, 开始加水, 加至最高液位, 保护时间23秒, 循环1次, 清
*               洗完成
*   形    参：无
*   返 回 值: -1:出故障; 1:停止清洗
*******************************************************************************
*/
/*
//该函数没有用到
void auto_close_device()
{
    //这里与电源按键关机执行相同的操作
    
    air_clean_cfg.power_onoff = OFF;
    
    
    //电源状态有记忆功能, 需要写入Flash
    write_cfg_to_flash(); 
    
    //清除设置"时间/模式"标志
    st_setting_status.setting_time_flag = 0;
    st_setting_status.setting_mode_flag = 0;
    
    //清除时间设置相关项, 复位"时间结构体"
    st_time_setting.hour_set_value = 0;     
    st_time_setting.minu_set_value = 0;     
    st_time_setting.week_set_value = 0;     
    st_time_setting.time_set_location = 0;  
    
    //清除模式设置相关项, 复位"模式结构体"中的相关标识
    st_mode_setting.set_mode_type_flag = 0;
    st_mode_setting.set_work_week_flag = 0;
    st_mode_setting.set_work_time_flag = 0;
    st_mode_setting.set_open_close_flag = 0;
    st_mode_setting.std_set_open_close_flag = 0;
    
    //清除"清洗"标志 / "一键排液"标志
    st_current_status.current_wash_status = 0;
    st_current_status.onekey_drain_status = 0;
}
*/


//更新 "雾化器工作时间/清洗时间"
void update_cycle_time(int type)
{
    
    //get_show_time();      //屏幕刷新线程一直在更新时间数据
    switch(type)
    {
        //1.更新雾化器间隔时间
        case ATOMIZER_INTERVAL_TYPE:
            //WS_DEBUG("\r\n ++++++++++++++++++++++++ update_cycle_time--> ATOMIZER_INTERVAL_TYPE \r\n");
            air_clean_cfg.atomizer_start_time.start_year = calendar.w_year;
            air_clean_cfg.atomizer_start_time.start_month = calendar.w_month;
            air_clean_cfg.atomizer_start_time.start_day = calendar.w_date;
        break;
        
        //2.更新清洗间隔时间
        case WASH_INTERVAL_TYPE:
            //WS_DEBUG("\r\n ++++++++++++++++++++++++ update_cycle_time--> WASH_INTERVAL_TYPE \r\n");
            air_clean_cfg.dev_work_time.start_year = calendar.w_year;
            air_clean_cfg.dev_work_time.start_month = calendar.w_month;
            air_clean_cfg.dev_work_time.start_day = calendar.w_date;
        break;
        
        default:
            break;
    }
    //写 Flash 
    //WS_DEBUG("\r\n ++++++++++++++++++++++++ update_cycle_time--> write_cfg_to_flash begin \r\n");
    write_cfg_to_flash();
    //WS_DEBUG("\r\n ++++++++++++++++++++++++ update_cycle_time--> write_cfg_to_flash end \r\n");
}


/*
报警出现E4, 第1次出现排液故障, 报E4报警, 继续第2次排液; (利用 err_drain_liquid_flag 标识)
若第2次排液出现故障, 则设备停止工作, E4保持; (不需要另作处理, 出现错误码, 设备停止工作)
若第2次排液正常, 则消除E4 (通过 BIT_CLEAR 操作清除错误码)

何时用到排液功能: 1.清洗; 2.一键排液; 3.工作仓出现"溢出液位"

argument 
    1:清洗功能排液; 2:一键排液功能排液; 3:工作仓出现"溢出液位"排液
*/
int wash_drain_liquid()
{
    OS_ERR err;
    
    int out_time = 0;       //排液保护时间
    int delay_count = 0;    //延时计数
    
    char err_drain_liquid_flag = 0;     //第1次出现排液故障标识
    
    //正在执行清洗排液
    //st_current_status.wash_drain_liquid_status = ON;
    
    WS_DEBUG("+++++++++ [wash_drain_liquid] 开始执行 wash_drain_liquid 函数 \r\n");
    
    solenoid_onoff(SOLENOID_OUT_WORK, ON);
    while(1)
    {
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
        {
            break;
        }
        
        //排液停止情形: 
        //  清洗与一键排液功能关闭: 断电或者关闭电源键可取消
        //电源关闭 跳出
        if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.current_wash_status))
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            WS_DEBUG("[wash_drain_liquid] 001 电源关闭 return  \r\n");
            return 1;
        }

        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
        out_time ++;
        //当启动清洗程序 ** 秒后液位传感器未感应到最低液位点, 设备停止工作, 报E4排液故障
        if(out_time >= DRAIN_LIQUID_SAFE_TIME)
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            //E4排液故障第一次, 不设置故障码
            WS_DEBUG("[wash_drain_liquid] E4排液故障 第一次 \r\n");
            err_drain_liquid_flag = 1;
            
            break;
        }
    }
    
    WS_DEBUG("[wash_drain_liquid] flag 000001 \r\n");
    //第1次出现排液故障，报E4报警，继续第2次排液
    if(1 == err_drain_liquid_flag)
    {
        out_time = 0;
        solenoid_onoff(SOLENOID_OUT_WORK, ON);
        while(1)
        {
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
            {
                //若第2次排液正常，则消除E4
                //BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID);
                break;
            }
            
            //排液停止情形: 1.电源关闭; 2.清洗关闭; 3.一键排液关闭
            //电源关闭 跳出
            if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.current_wash_status))
            {
                solenoid_onoff(SOLENOID_OUT_WORK, OFF);
                WS_DEBUG("[wash_drain_liquid] 002 电源关闭 return  \r\n");
                return 1;
            }

            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
            out_time ++;
            //当启动清洗程序 ** 秒后液位传感器未感应到最低液位点, 设备停止工作, 报E4排液故障
            if(out_time >= DRAIN_LIQUID_SAFE_TIME)
            {
                solenoid_onoff(SOLENOID_OUT_WORK, OFF);
                //设置E4排液故障
                BIT_SET(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID);
                WS_DEBUG("[wash_drain_liquid] 清洗排液故障，设置E4排液故障 \r\n");
                
                WS_DEBUG("[wash_drain_liquid] E4排液故障 第二次 return \r\n");
                
                return -1;
            }
        }
    }
    
    WS_DEBUG("[wash_drain_liquid] flag 000002 \r\n");

    //延时 DRAIN_LIQUID_DELAY_TIME 秒, 关闭排水电磁阀
    while(1)
    {
        //电源键关闭/清洗键关闭, 跳出
        if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.current_wash_status))
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            WS_DEBUG("[wash_drain_liquid] 003 电源关闭 return  \r\n");
            return 1;
        }
        
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); 
        delay_count++;
        if(delay_count >= DRAIN_LIQUID_DELAY_TIME)
        {
            WS_DEBUG("[wash_drain_liquid] 延时 DRAIN_LIQUID_DELAY_TIME 秒关闭排水电磁阀 \r\n");
            break;
        }
    }
    
    solenoid_onoff(SOLENOID_OUT_WORK, OFF);
    WS_DEBUG("[wash_drain_liquid] 正常退出 wash_drain_liquid 函数 \r\n");
    
    return 0;
}




//一键排液功能执行排液
int onekey_drain_liquid()
{
    OS_ERR err;
    
    int out_time = 0;       //排液保护时间
    int delay_count = 0;    //延时计数
    
    char err_drain_liquid_flag = 0;     //第1次出现排液故障标识
    
    //正在执行清洗排液
    //st_current_status.wash_drain_liquid_status = ON;
    
    WS_DEBUG("+++++++++ [onekey_drain_liquid] 开始执行 onekey_drain_liquid 函数 \r\n");
    
    solenoid_onoff(SOLENOID_OUT_WORK, ON);
    while(1)
    {
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
        {
            break;
        }
        
        //电源键关闭 / 一键排液标识被清除, 跳出
        if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.onekey_drain_status))
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            WS_DEBUG("[onekey_drain_liquid] 001 电源关闭 return  \r\n");
            return 1;
        }

        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
        out_time ++;
        //当启动清洗程序 ** 秒后液位传感器未感应到最低液位点, 设备停止工作, 报E4排液故障
        if(out_time >= DRAIN_LIQUID_SAFE_TIME)
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            WS_DEBUG("[onekey_drain_liquid] E4排液故障 第一次 \r\n");
            err_drain_liquid_flag = 1;
            
            break;
        }
    }
    
    WS_DEBUG("[onekey_drain_liquid] flag 000001 \r\n");
    //第1次出现排液故障，报E4报警，继续第2次排液
    if(1 == err_drain_liquid_flag)
    {
        out_time = 0;
        solenoid_onoff(SOLENOID_OUT_WORK, ON);
        while(1)
        {
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
            {
                //若第2次排液正常，则消除E4
                //BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID);
                break;
            }
            
            //电源键关闭 / 一键排液标识被清除, 跳出
            if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.onekey_drain_status))
            {
                solenoid_onoff(SOLENOID_OUT_WORK, OFF);
                WS_DEBUG("[onekey_drain_liquid] 002 电源关闭 return  \r\n");
                return 1;
            }

            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
            out_time ++;
            //当启动清洗程序 ** 秒后液位传感器未感应到最低液位点, 设备停止工作, 报E4排液故障
            if(out_time >= DRAIN_LIQUID_SAFE_TIME)
            {
                solenoid_onoff(SOLENOID_OUT_WORK, OFF);
                //设置E4排液故障
                BIT_SET(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID);
                WS_DEBUG("[onekey_drain_liquid] 一键排液过程中出现排液故障，设置E4排液故障 \r\n");
                
                WS_DEBUG("[onekey_drain_liquid] E4排液故障 第二次 return \r\n");
                
                return -1;
            }
        }
    }
    
    WS_DEBUG("[onekey_drain_liquid] flag 000002 \r\n");

    //延时 DRAIN_LIQUID_DELAY_TIME 秒, 关闭排水电磁阀
    while(1)
    {
        //电源键关闭 / 一键排液标识被清除, 跳出
        if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.onekey_drain_status))
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            WS_DEBUG("[onekey_drain_liquid] 003 电源关闭 return  \r\n");
            return 1;
        }
        
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); 
        delay_count++;
        if(delay_count >= DRAIN_LIQUID_DELAY_TIME)
        {
            WS_DEBUG("[onekey_drain_liquid] 延时 DRAIN_LIQUID_DELAY_TIME 秒关闭排水电磁阀 \r\n");
            break;
        }
    }
    
    solenoid_onoff(SOLENOID_OUT_WORK, OFF);
    WS_DEBUG("[onekey_drain_liquid] 正常退出 onekey_drain_liquid 函数 \r\n");
    
    return 0;
}




//溢出排液

/*
//"电源关闭时执行溢出排液"与"电源开启时执行溢出排液"不同之处:
    电源关闭时执行溢出排液不需要设置错误码 
//

与普通排液的区别：
    1.不需要考虑返回值  
    2.不需要设置错误码  -- 需要确认？？
    3.排液过程中，无需再考虑检测到溢出液位 -- 排液过程不需要检测，所有检测都放到任务函数中
*/
void overflow_drain_liquid()
{
    OS_ERR err;
    
    int out_time = 0;       //排液保护时间
    int delay_count = 0;    //延时计数
    
    char err_drain_liquid_flag = 0;     //第1次出现排液故障标识
    
    st_current_status.overflow_drain_liquid_status = 1;
    
    //先保证设备停止工作
    dev_stop_work();
    
    WS_DEBUG("+++++++++ [overflow_drain_liquid] 开始执行 overflow_drain_liquid 函数 \r\n");
    
    solenoid_onoff(SOLENOID_OUT_WORK, ON);
    while(1)
    {
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
        {
            break;
        }

        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
        out_time ++;
        //当启动清洗程序 ** 秒后液位传感器未感应到最低液位点, 设备停止工作, 报E4排液故障
        if(out_time >= DRAIN_LIQUID_SAFE_TIME)
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            
            WS_DEBUG("[overflow_drain_liquid] E4排液故障 第一次 \r\n");
            err_drain_liquid_flag = 1;
            
            break;
        }
    }
    
    WS_DEBUG("[overflow_drain_liquid] flag 000001 \r\n");
    //第1次出现排液故障，报E4报警，继续第2次排液
    if(1 == err_drain_liquid_flag)
    {
        out_time = 0;
        solenoid_onoff(SOLENOID_OUT_WORK, ON);
        while(1)
        {
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
            {
                //若第2次排液正常，则消除E4
                //BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID);
                break;
            }

            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
            out_time ++;
            //当启动清洗程序 ** 秒后液位传感器未感应到最低液位点, 设备停止工作, 报E4排液故障
            if(out_time >= DRAIN_LIQUID_SAFE_TIME)
            {
                solenoid_onoff(SOLENOID_OUT_WORK, OFF);
                
                ///*
                //设置E4排液故障
                BIT_SET(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID);
                WS_DEBUG("[overflow_drain_liquid] 溢出液位排液过程中出现排液故障，设置E4排液故障 \r\n");
                
                //*/
                WS_DEBUG("[overflow_drain_liquid] E4排液故障 第二次 return \r\n");
                
                
                //如果不要设置故障码，则想办法调整到最后才退出
                st_current_status.overflow_drain_liquid_status = 0;
                return ;
            }
        }
    }
    
    WS_DEBUG("[overflow_drain_liquid] flag 000002 \r\n");

    //延时 DRAIN_LIQUID_DELAY_TIME 秒, 关闭排水电磁阀
    while(1)
    {
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); 
        delay_count++;
        if(delay_count >= DRAIN_LIQUID_DELAY_TIME)
        {
            WS_DEBUG("[overflow_drain_liquid] 延时 DRAIN_LIQUID_DELAY_TIME 秒关闭排水电磁阀 \r\n");
            break;
        }
    }
    
    solenoid_onoff(SOLENOID_OUT_WORK, OFF);
    WS_DEBUG("[overflow_drain_liquid] 正常退出 overflow_drain_liquid 函数 \r\n");
    
    st_current_status.overflow_drain_liquid_status = 0;
    
    return ;
}



//    time_record_st  first_overflow_time;    //工作仓第一次溢出时间点
//    time_record_st  second_overflow_time;   //工作仓第二次溢出时间点
void record_first_overflow_time()
{
    st_current_status.first_overflow_date.start_year = calendar.w_year;
    st_current_status.first_overflow_date.start_month = calendar.w_month;
    st_current_status.first_overflow_date.start_day = calendar.w_date;
    
    st_current_status.first_overflow_time.st_hour = calendar.hour;
    st_current_status.first_overflow_time.st_min = calendar.min;
    st_current_status.first_overflow_time.st_sec = calendar.sec;
}


void clear_first_overflow_time()
{
    st_current_status.first_overflow_date.start_year = 0;
    st_current_status.first_overflow_date.start_month = 0;
    st_current_status.first_overflow_date.start_day = 0;
    
    st_current_status.first_overflow_time.st_hour = 0;
    st_current_status.first_overflow_time.st_min = 0;
    st_current_status.first_overflow_time.st_sec = 0;
}

//待完善
int get_overflow_time_interval()
{
    int cur_min, rec_min;
    int interval_min;
    int interval_days;
    
    int year_start;
    int month_start;
    int day_start;

    //get_show_time();      //屏幕刷新线程一直在更新时间数据
    int year_end = calendar.w_year;
    int month_end = calendar.w_month;
    int day_end = calendar.w_date;
    
    year_start = st_current_status.first_overflow_date.start_year;
    month_start = st_current_status.first_overflow_date.start_month;
    day_start = st_current_status.first_overflow_date.start_day;
    interval_days = day_diff(year_start, month_start, day_start, year_end, month_end, day_end);
            
    if(interval_days == 0)
    {
        cur_min = calendar.hour * 60 + calendar.min;
        rec_min = st_current_status.first_overflow_time.st_hour * 60 + \
                st_current_status.first_overflow_time.st_min;
        interval_min = cur_min - rec_min;
    }
    else if(interval_days == 1)
    {
        cur_min = calendar.hour * 60 + calendar.min;
        rec_min = st_current_status.first_overflow_time.st_hour * 60 + \
                st_current_status.first_overflow_time.st_min;
        interval_min = cur_min + (24*60 - rec_min);
    }
    else if(interval_days > 1)
    {
        interval_min = 100; //大于 60 即可
    }
    else
    {
        interval_min = -1;
    }
    
    if(interval_min > 0)
        return interval_min;
    else
        return -1;
}


/*
何时调用？
    1.清洗时调用 --> supply_water(该函数何时被调用？) --> overflow_setting
    2.出现非E12错误码的过程中 --> 出现E12 (应该还要考虑E11)
    3.自动配比过程中
*/
void overflow_setting()
{
    int min_interval;
    //没有设置 E11 (第一次检测到"溢出液位")
    if(0 == st_current_status.first_overflow_flag ||
        BIT_GET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_FIRST) != 1)
    {
        //记录工作仓第一次溢出时间点
        record_first_overflow_time();
        //设置错误码 E11
        BIT_SET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_FIRST);
        
        st_current_status.first_overflow_flag = 1;
        
        st_current_status.peibi_overflow_num = 1;
    }
    //已经设置 E11 
    else if( BIT_GET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_FIRST) )
    {
        min_interval = get_overflow_time_interval();
        if(min_interval > 60)   //错误码仍旧保持E11不变
        {
            //时间间隔超过1小时, 重新记录溢出的时间
            record_first_overflow_time();
            
            st_current_status.peibi_overflow_num = 1;
        }
        else
        {
            //设置错误码 E12
            BIT_SET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_MANY_TIMES);
            dev_stop_work();
        }
    }
}

void overflow_handle()
{
    
}



//清洗过程中"进水"操作 (注意: 该函数只用于清洗时)
int supply_water()
{
    OS_ERR err;
    
    char timeout = 0;               //注水超时时间
    
    //char overflow_flag;         //注水过程中出现"溢出"液位
    //overflow_flag = 0;
    
    WS_DEBUG("+++++++++ [supply_water] 开始执行 supply_water 函数 \r\n");
    
    solenoid_onoff(SOLENOID_INTO_WORK, ON);
    while(1)
    {
        /*
        //工作仓到达溢出液位
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_OVERFLOW) == PULLDOWN)
        {
            overflow_setting();
            
            overflow_flag = 1;
            break;
        }
        */
        
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) == PULLDOWN)
        {
            break;
        }
        
        //电源键关闭 / 清洗标识被清除, 跳出
        if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.current_wash_status))
        {
            solenoid_onoff(SOLENOID_INTO_WORK, OFF);
            return 1;
        }
        
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
        
        //注水启动，150S内未到达最高液位点，出现E3
        timeout ++;
        if(timeout >= SUPPLY_WATER_TIMEOUT)
        {
            WS_DEBUG("+++++++++ [supply_water] timeout = %d \r\n", timeout);
            solenoid_onoff(SOLENOID_INTO_WORK, OFF);
            //设置E3进水故障
            //if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) != PULLDOWN)     //工作仓未到达最高液位点
            {
                WS_DEBUG("[supply_water] 清洗加水过程中出现加水故障，设置 ERR_WORK_SUPPLY_WATER 故障码\r\n");
                BIT_SET(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER);
                
                return -1;
            }
            
            return 0;
        }
    }
    solenoid_onoff(SOLENOID_INTO_WORK, OFF);
    
    //补水导致工作仓到达"溢出液位", 由 DetectOverflowTask 任务中检测执行排液
    /*
    if(1 == overflow_flag)
    {
        drain_liquid();
        //设置E12, 则清洗结束; E11不影响正常工作
        if(BIT_GET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_MANY_TIMES))
        {
            st_current_status.current_wash_status = 0;      //清洗结束
            
            return -1;
        }
    }
    */

    return 0;
}



//执行通常清洗
//排液-->进水-->排液-->进水-->排液-->更新清洗时间 , 接下来可加液工作
int do_wash()
{
    dev_stop_work();
    
    WS_DEBUG("[do_wash] 正在执行清洗，Step 1. 排液  \r\n");
    //Step 1. 排液
    if(wash_drain_liquid() != 0)
    {
        st_current_status.current_wash_status = 0; 
        return -1;
    }
    
    WS_DEBUG("[do_wash] 正在执行清洗，Step 2. 进水  \r\n");
    //Step 2. 进水
    if(supply_water() != 0)
    {
        st_current_status.current_wash_status = 0; 
        return -1;
    }
    
    WS_DEBUG("[do_wash] 正在执行清洗，Step 3. 排液  \r\n");
    //Step 3. 排液
    if(wash_drain_liquid() != 0)
    {
        st_current_status.current_wash_status = 0; 
        return -1;
    }
    
    WS_DEBUG("[do_wash] 正在执行清洗，Step 4. 进水  \r\n");
    //Step 4. 进水
    if(supply_water() != 0)
    {
        st_current_status.current_wash_status = 0; 
        return -1;
    }
    
    WS_DEBUG("[do_wash] 正在执行清洗，Step 5. 排液  \r\n");
    //Step 5. 排液
    if(wash_drain_liquid() != 0)
    {
        st_current_status.current_wash_status = 0; 
        return -1;
    }
    
    WS_DEBUG("[do_wash] 正在执行清洗，Step 6. 更新清洗时间  \r\n");
    //Step 6. 更新清洗时间
    update_cycle_time(WASH_INTERVAL_TYPE);
    if(air_clean_cfg.wash_mode == AUTO_WASH_MODE)
    {
        st_current_status.auto_wash_prompt = 1;     //自动清洗完成提示标志置为1
        st_current_status.auto_wash_past_hours = 0; //自动清洗完成的小时数清0
    }
    
    WS_DEBUG("[do_wash] 正在执行清洗，Step 7. 清除相关清洗标志  \r\n");
    //Step 7. 清除相关清洗标志
    st_current_status.current_wash_status = 0; 
    st_current_status.manual_wash_stopdev = 0;
    BIT_CLEAR(air_clean_cfg.err_code_bitmap, FORCE_WASH_STOP_DEV);
    
    return 0;
}



//**************************************************************************
//**************************************************************************


#if MANUAL_EDITION

//手动版清洗时 -- 补液, 注意: 只能用于手动版清洗
int supply_liquid()
{
    OS_ERR err;
    
    char tmp_count = 0;
    char over_time_flag = 0;
    
    liquid_pump_onoff(PUMP_TO_WORK, ON);        //液泵2开始工作进行配比
    while(1)
    {
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) == PULLDOWN)
        {
            break;
        }
        
        //关闭电源/执行清洗/添加原液, 则停止配比
        //if((OFF == air_clean_cfg.power_onoff)
        //    || (ON == st_current_status.current_wash_status)
        //    || (ON == st_current_status.add_plant_liquid_flag)) 
        if(OFF == air_clean_cfg.power_onoff)
        {
            break;
        }
        
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时1秒 
        tmp_count++;
        if(tmp_count >= MANUAL_ORG_TO_WORK_SAFE_TIME)
        {
            over_time_flag = 1;
            break;
        }
    }
    liquid_pump_onoff(PUMP_TO_WORK, OFF);       //液泵2停止工作
    
    if(over_time_flag)
    {
        //加液泵开始抽液，在保护时间 32 秒后，工作仓液位传感器仍未离开最低液位点，
        //液箱补液出错，设备停止工作并报警，显示屏显示故障代码 E3
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
        {
            BIT_SET(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER);
            WS_DEBUG("[supply_liquid] 手动版清洗过程中出现补液故障，设置 ERR_WORK_SUPPLY_WATER 故障码 \r\n");
            return -1;
        }
    }
    
    return 0;
}

//手动版清洗过程
//1.打开排液电磁阀，保护时间个机制和自动版一样，排液结束关闭排位电磁阀
//2.启动补液泵，加液至最高点或32秒时间结束
//3.打开排液电磁阀，排液完成关闭电磁阀
int manual_do_wash()
{
    OS_ERR err;
    char tmp_count;
    char over_time_flag;
    
    char org_low_flag = 0;      //原液箱低液位标识
    
    //WS_DEBUG("################### [manual_do_wash] enter in manual_do_wash func ....\r\n");
    //原液箱1液位，没有进行记次时，清洗程序启动结束后，报E7告警
    
    //原液箱1液位进行记次后，不管是1次还是2次，不可以执行清洗程序 -- 不要这个需求
    //应该都可以执行清洗，不然出货没法清空工作仓的液
    if(PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW))
    {
        if(0 == air_clean_cfg.org_low_to_work_count)
        {
            if(BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT) != 1)
            {
                BIT_SET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT);      //设置必须加液的错误码 E7
                write_cfg_to_flash();   //E7 需记录 Flash
            }
        }
        
        //原液箱低液位标识置为 1
        org_low_flag = 1;
        
        //手动版: 低液位不执行自动清洗; manual_need_auto_wash:自动清洗标识
        if(1 == st_current_status.manual_need_auto_wash)
        {
            st_current_status.manual_need_auto_wash = 0;
            st_current_status.current_wash_status = 0; 
            return -1;
        }
    }
    //清除自动清洗标识(手动版特有的自动清洗标识)
    st_current_status.manual_need_auto_wash = 0;
    
    dev_stop_work();
    
    //Step 1. 排液
    WS_DEBUG("[manual_do_wash] 正在执行手动清洗 Step 1. 排液  \r\n");
    if(wash_drain_liquid() != 0)
    {
        st_current_status.current_wash_status = 0; 
        return -1;
    }
    
    //Step 2. 启动补液泵，加液至最高点或32秒时间结束
    WS_DEBUG("[manual_do_wash] 正在执行手动清洗 Step 2. 启动补液泵，加液至最高点或32秒时间结束  \r\n");
    if(supply_liquid() != 0)
    {
        st_current_status.current_wash_status = 0;
        return -1;
    }
        
    
    //Step 3. 排液
    WS_DEBUG("[manual_do_wash] 正在执行手动清洗 Step 3. 排液  \r\n");
    if(wash_drain_liquid() != 0)
    {
        st_current_status.current_wash_status = 0; 
        return -1;
    }
    
    //Step 4. 更新清洗时间
    WS_DEBUG("[manual_do_wash] 正在执行手动清洗 Step 4. 更新清洗时间  \r\n");
    update_cycle_time(WASH_INTERVAL_TYPE);
    if(air_clean_cfg.wash_mode == AUTO_WASH_MODE)
    {
        st_current_status.auto_wash_prompt = 1;     //自动清洗完成提示标志置为1
        st_current_status.auto_wash_past_hours = 0; //自动清洗完成的小时数清0
    }
    
    //Step 5. 清除相关清洗标志
    WS_DEBUG("[manual_do_wash] 正在执行手动清洗 Step 5. 清除相关清洗标志  \r\n");
    st_current_status.current_wash_status = 0; 
    st_current_status.manual_wash_stopdev = 0;
    BIT_CLEAR(air_clean_cfg.err_code_bitmap, FORCE_WASH_STOP_DEV);
    
    
    //原液箱低液位，没有进行记次时，清洗程序启动结束后，报E7告警
    if(1 == org_low_flag)
    {
        air_clean_cfg.org_low_to_work_count = 0;
    }
    
    return 0;
}
#endif


//一键排液功能
/*
第一步:排液;  第二步:设备停止工作,并锁定;  第三步:清除相关标志
*/
int do_onekey_drain()
{
    OS_ERR err;
    dev_stop_work();
    
    //WS_DEBUG("################### [do_onekey_drain] enter in do_onekey_drain func ....\r\n");
    
    WS_DEBUG("[do_onekey_drain] 执行 onekey_drain_liquid  \r\n");
    //Step 1. 排液
    if(onekey_drain_liquid() != 0)
    {
        WS_DEBUG("[do_onekey_drain] onekey_drain_liquid 函数返回非0 \r\n");
        st_current_status.onekey_drain_status = 0;
        return -1;
    }
    
    //Step 3. 清除一键排液标志, 设置一键排液完成标识, 写入到Flash
    st_current_status.onekey_drain_status = 0;
    air_clean_cfg.onekey_drain_fished = 1;
    write_cfg_to_flash();
    
    WS_DEBUG("[do_onekey_drain] 一键排液正常退出, \r\n");
    return 0;
}


void do_current_wash()
{
    #if MANUAL_EDITION
        WS_DEBUG("\r\n[do_current_wash] 手动版通常清洗开始执行 ...\r\n");
        manual_do_wash();           //手动版通常清洗
        WS_DEBUG("\r\n[do_current_wash] 手动版通常清洗执行结束 ...\r\n");
    #else
        WS_DEBUG("\r\n[do_current_wash] 自动版通常清洗开始执行 ...\r\n");
        do_wash();                  //自动版通常清洗
        WS_DEBUG("\r\n[do_current_wash] 自动版通常清洗执行结束 ...\r\n");
    #endif
}

/*
*********************************************************************************************************
*                                           KEY CONTROL TASK
*********************************************************************************************************
*/

//自定义模式设置周几是否工作
//set_custom_mode_work_week(周几[1-7], 是否工作[0:不工作; 1:工作]);
void set_custom_mode_work_week(char week_num, char work_flag)
{
    //设置周一是否工作
    if(1 == week_num)
    {
        st_mode_setting.custom_work_week[1] = work_flag;
        st_mode_setting.set_work_week_flag = 2;
    }
    //设置周二是否工作
    if(2 == week_num)
    {
        st_mode_setting.custom_work_week[2] = work_flag;
        st_mode_setting.set_work_week_flag = 3;
    }
    //设置周三是否工作
    if(3 == week_num)
    {
        st_mode_setting.custom_work_week[3] = work_flag;
        st_mode_setting.set_work_week_flag = 4;
    }
    //设置周四是否工作
    if(4 == week_num)
    {
        st_mode_setting.custom_work_week[4] = work_flag;
        st_mode_setting.set_work_week_flag = 5;
    }
    //设置周一是否工作
    if(5 == week_num)
    {
        st_mode_setting.custom_work_week[5] = work_flag;
        st_mode_setting.set_work_week_flag = 6;
    }
    //设置周一是否工作
    if(6 == week_num)
    {
        st_mode_setting.custom_work_week[6] = work_flag;
        st_mode_setting.set_work_week_flag = 7;
    }
    //设置周日是否工作
    if(7 == week_num)
    {
        st_mode_setting.custom_work_week[7] = work_flag;
        st_mode_setting.set_work_week_flag = 8;             //设置完周日之后, 这里设为8, 代表"设置周几是否工作"完成
        st_mode_setting.set_work_time_flag = 1;             //这里赋值为1, 代表"设置工作时间段"开始
    }
}


void ds3231_set_time(u8 hour, u8 min, u8 week)
{
    /*
    周日: 2017.10.01
    周一: 2017.10.02
    周二: 2017.10.03
    周三: 2017.10.04
    周四: 2017.10.05
    周五: 2017.10.06
    周六: 2017.10.07
    void DS3231_Set(u8 syear,u8 smon,u8 sday,u8 week,u8 hour,u8 min,u8 sec);
    */
    u8 sec = 0;
    
    u8 syear = 17;
    u8 smon = 10;
    
    u8 sday = 1;
    if(7 == week)       //周日 --> 10.01
        sday = 1;
    else
        sday = week + 1;
    
    DS3231_Set(syear, smon, sday, week, hour, min, sec);
    
    #if TEST_AUTO_WASH
        get_show_time();
    #else
        get_show_time();
        //时间重设之后, 需要对"强制清洗/雾化器切换"等重新计时
        update_cycle_time(ATOMIZER_INTERVAL_TYPE);
        update_cycle_time(WASH_INTERVAL_TYPE);
    #endif
}


//根据按键值执行相关操作
char update_status_by_data_code(unsigned char data_code)
{
    int i;
    char str[10] = {'\0'};
    
    switch(data_code)
    {
        //1.热点
        case IR_CODE_NET_LONG:
            #if NETWORK_VERSION
                //初始化网络配置
                WS_DEBUG("按键操作 [IR_CODE_NET_LONG] 初始化网络配置\r\n");
                TriggleWifi();
                st_current_status.net_trigger_flag = 1;
            #endif
        break;
        
        //2.电源(power_onoff -- 写入Flash)
        case IR_CODE_POWER:
            air_clean_cfg.power_onoff = !(air_clean_cfg.power_onoff);
            if(ON == air_clean_cfg.power_onoff)
            {
                //WS_DEBUG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ IR_CODE_POWER is ON ....\r\n");
                st_current_status.start_up_flag = 1;
            }
            
            //电源状态有记忆功能, 需要写入Flash
            write_cfg_to_flash(); 
            
            //清除设置"时间/模式"标志
            st_setting_status.setting_time_flag = 0;
            st_setting_status.setting_mode_flag = 0;
            
            //清除时间设置相关项, 复位"时间结构体"
            st_time_setting.hour_set_value = 0;     
            st_time_setting.minu_set_value = 0;     
            st_time_setting.week_set_value = 0;     
            st_time_setting.time_set_location = 0;  
            
            //清除模式设置相关项, 复位"模式结构体"中的相关标识
            st_mode_setting.set_mode_type_flag = 0;
            st_mode_setting.set_work_week_flag = 0;
            st_mode_setting.set_work_time_flag = 0;
            st_mode_setting.set_open_close_flag = 0;
            st_mode_setting.std_set_open_close_flag = 0;
            
            //清除"清洗"标志 "一键排液"标志
            st_current_status.current_wash_status = 0;
            st_current_status.onekey_drain_status = 0;
        break;
        
        //3.时间(设置时间)
        case IR_CODE_SET_TIME:
            
            WS_DEBUG("按键操作 [IR_CODE_SET_TIME] 设置时间，当前时间： %d-%d-%d WEEK%d %d:%d:%d \r\n", \
                    calendar.w_year, calendar.w_month, calendar.w_date, calendar.week, calendar.hour,calendar.min,calendar.sec);
            
            //置位"设置时间标志"
            if(st_setting_status.setting_time_flag == 0)
            {
                st_setting_status.setting_time_flag = 1;
                //"时间设置结构体"赋初值
                st_time_setting.hour_set_value = calendar.hour;     //小时设置值:0-23
                st_time_setting.minu_set_value = calendar.min;      //分钟设置值:0-59
                st_time_setting.week_set_value = calendar.week;     //周设置值  :1-7
                st_time_setting.time_set_location = 1;              //设置位置  :1-3 -- 1:小时; 2:分钟; 3:周
            }
        break;
        
        //4.模式(切换"标准模式"/设置"自定义模式")
        case IR_CODE_SET_MODE:
            if(st_setting_status.setting_mode_flag == 0)
            {
                st_setting_status.setting_mode_flag = 1;
                
                //"模式结构体"中的相关标识初始化
                st_mode_setting.set_mode_type_flag = 0;
                st_mode_setting.set_work_week_flag = 0;
                st_mode_setting.set_work_time_flag = 0;
                st_mode_setting.set_open_close_flag = 0;
                st_mode_setting.std_set_open_close_flag = 0;

                //标准模式初始化
                //*******************************************************************
                st_mode_setting.standard_open_min = air_clean_cfg.standard_mode_open_min;   //开几分钟
                st_mode_setting.standard_close_min = air_clean_cfg.standard_mode_close_min; //停几分钟
                //*******************************************************************
                
                
                //自定义模式初始化 -- 这里初始化为 Flash 中保存的数据
                //*******************************************************************
                //周开关初始化 (这里周没法显示 Flash 中保存的数据)
                memset(st_mode_setting.custom_work_week, 0, sizeof(st_mode_setting.custom_work_week));
                
                //工作时间段初始化 (显示 Flash 中保存的数据)
                //开始工作时间段:小时
                st_mode_setting.custom_start_work_hour = air_clean_cfg.st_custom_mode.custom_time_plan[0].start_hour;
                //开始工作时间段:分钟
                st_mode_setting.custom_start_work_min = air_clean_cfg.st_custom_mode.custom_time_plan[0].start_minute;
                //停止工作时间段:小时
                st_mode_setting.custom_stop_work_hour = air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_hour;
                //停止工作时间段:分钟
                st_mode_setting.custom_stop_work_min = air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_minute;
                /*
                st_mode_setting.custom_start_work_hour = 8;             //开始工作时间段:小时
                st_mode_setting.custom_start_work_min = 30;             //开始工作时间段:分钟
                st_mode_setting.custom_stop_work_hour = 16;             //停止工作时间段:小时
                st_mode_setting.custom_stop_work_min = 30;              //停止工作时间段:分钟
                */
                
                //开几停几初始化 (显示 Flash 中保存的数据)
                st_mode_setting.custom_open_min = air_clean_cfg.st_custom_mode.custom_open_min;     //开几分钟
                st_mode_setting.custom_close_min = air_clean_cfg.st_custom_mode.custom_close_min;   //停几分钟
                //*******************************************************************
                
                //模式类型初始化
                st_mode_setting.mode_type_select = 1;               //1:标准模式 2:自定义模式
            }
        break;
        
        //5.留存向上键
        case IR_CODE_UP:
            //设置时间时
            if(1 == st_setting_status.setting_time_flag)
            {
                if(1 == st_time_setting.time_set_location)          //设置小时: 0-23
                {
                    if(23 == st_time_setting.hour_set_value)
                        st_time_setting.hour_set_value = 0;
                    else
                        st_time_setting.hour_set_value += 1;
                }
                else if(2 == st_time_setting.time_set_location)     //设置分钟: 0-23
                {
                    
                    if(59 == st_time_setting.minu_set_value)
                        st_time_setting.minu_set_value = 0;
                    else
                        st_time_setting.minu_set_value += 1;
                }
                else if(3 == st_time_setting.time_set_location)     //设置周: 1-7
                {
                    if(7 == st_time_setting.week_set_value)
                        st_time_setting.week_set_value = 1;
                    else
                        st_time_setting.week_set_value += 1;
                }
            }
            
            //*****************************************************************
            
            //设置模式时
            else if(1 == st_setting_status.setting_mode_flag)
            {
                //选择设置的"模式类型"(标准模式/自定义模式)
                if(0 == st_mode_setting.set_mode_type_flag)
                {
                    if(1 == st_mode_setting.mode_type_select)
                        st_mode_setting.mode_type_select = 2;
                    else if(2 == st_mode_setting.mode_type_select)
                        st_mode_setting.mode_type_select = 1;
                }
                
                //标准模式: 设置开几停几
                else if(1 == st_mode_setting.mode_type_select)
                {
                    if(st_mode_setting.std_set_open_close_flag>0 && st_mode_setting.std_set_open_close_flag<3)
                    {
                        if(1 == st_mode_setting.std_set_open_close_flag)
                        {
                            if(9 == st_mode_setting.standard_open_min)
                                st_mode_setting.standard_open_min = 1;
                            else
                                st_mode_setting.standard_open_min += 1;
                        }
                        else if(2 == st_mode_setting.std_set_open_close_flag)
                        {
                            if(9 == st_mode_setting.standard_close_min)
                                st_mode_setting.standard_close_min = 1;
                            else
                                st_mode_setting.standard_close_min += 1;
                        }
                    }
                }
                
                //自定义模式相关设置
                else if(2 == st_mode_setting.mode_type_select)
                {
                    //1.自定义模式下:设置时间段
                    if(st_mode_setting.set_work_time_flag>0 && st_mode_setting.set_work_time_flag<5)
                    {
                        if(1 == st_mode_setting.set_work_time_flag)         //设置"开始工作时间段:小时"
                        {
                            if(23 == st_mode_setting.custom_start_work_hour)
                                st_mode_setting.custom_start_work_hour = 0;
                            else
                                st_mode_setting.custom_start_work_hour += 1;
                        }
                        
                        else if(2 == st_mode_setting.set_work_time_flag)    //设置"开始工作时间段:分钟"
                        {
                            if(59 == st_mode_setting.custom_start_work_min)
                                st_mode_setting.custom_start_work_min = 0;
                            else
                                st_mode_setting.custom_start_work_min += 1;
                        }
                        
                        else if(3 == st_mode_setting.set_work_time_flag)    //设置"停止工作时间段:小时"
                        {
                            if(23 == st_mode_setting.custom_stop_work_hour)
                                st_mode_setting.custom_stop_work_hour = 0;
                            else
                                st_mode_setting.custom_stop_work_hour += 1;
                        }
                        else if(4 == st_mode_setting.set_work_time_flag)    //设置"停止工作时间段:分钟"
                        {
                            if(59 == st_mode_setting.custom_stop_work_min)
                                st_mode_setting.custom_stop_work_min = 0;
                            else
                                st_mode_setting.custom_stop_work_min += 1;
                        }
                    }
                    
                    //2.自定义模式下:设置开几停几
                    else if(st_mode_setting.set_open_close_flag>0 && st_mode_setting.set_open_close_flag<3)
                    {
                        if(1 == st_mode_setting.set_open_close_flag)
                        {
                            if(9 == st_mode_setting.custom_open_min)
                                st_mode_setting.custom_open_min = 1;
                            else
                                st_mode_setting.custom_open_min += 1;
                        }
                        else if(2 == st_mode_setting.set_open_close_flag)
                        {
                            if(9 == st_mode_setting.custom_close_min)
                                st_mode_setting.custom_close_min = 1;
                            else
                                st_mode_setting.custom_close_min += 1;
                        }
                    }
                }
            }
        break;

        //6.留存向下键
        case IR_CODE_DOWN:
            //设置时间时
            if(1 == st_setting_status.setting_time_flag)
            {
                if(1 == st_time_setting.time_set_location)          //设置小时: 0-23
                {
                    if(0 == st_time_setting.hour_set_value)
                        st_time_setting.hour_set_value = 23;
                    else
                        st_time_setting.hour_set_value -= 1;
                }
                else if(2 == st_time_setting.time_set_location)     //设置分钟: 0-23
                {
                    if(0 == st_time_setting.minu_set_value)
                        st_time_setting.minu_set_value = 59;
                    else
                        st_time_setting.minu_set_value -= 1;
                        
                }
                else if(3 == st_time_setting.time_set_location)     //设置周: 1-7
                {
                    if(1 == st_time_setting.week_set_value)
                        st_time_setting.week_set_value = 7;
                    else
                        st_time_setting.week_set_value -= 1;
                }
            }
            
            //*****************************************************************
            
            //设置模式时
            else if(1 == st_setting_status.setting_mode_flag)
            {
                //选择设置的"模式类型"(标准模式/自定义模式)
                if(0 == st_mode_setting.set_mode_type_flag)
                {
                    if(1 == st_mode_setting.mode_type_select)
                        st_mode_setting.mode_type_select = 2;
                    else if(2 == st_mode_setting.mode_type_select)
                        st_mode_setting.mode_type_select = 1;
                }
                
                //标准模式: 设置开几停几
                else if(1 == st_mode_setting.mode_type_select)
                {
                    if(st_mode_setting.std_set_open_close_flag>0 && st_mode_setting.std_set_open_close_flag<3)
                    {
                        if(1 == st_mode_setting.std_set_open_close_flag)
                        {
                            if(1 == st_mode_setting.standard_open_min)
                                st_mode_setting.standard_open_min = 9;
                            else
                                st_mode_setting.standard_open_min -= 1;
                        }
                        if(2 == st_mode_setting.std_set_open_close_flag)
                        {
                            if(1 == st_mode_setting.standard_close_min)
                                st_mode_setting.standard_close_min = 9;
                            else
                                st_mode_setting.standard_close_min -= 1;
                        }
                    }
                }
                
                //自定义模式相关设置
                else if(2 == st_mode_setting.mode_type_select)
                {
                    //1.自定义模式下:设置时间段
                    if(st_mode_setting.set_work_time_flag>0 && st_mode_setting.set_work_time_flag<5)
                    {
                        if(1 == st_mode_setting.set_work_time_flag)     //设置"开始工作时间段:小时"
                        {
                            if(0 == st_mode_setting.custom_start_work_hour)
                                st_mode_setting.custom_start_work_hour = 23;
                            else
                                st_mode_setting.custom_start_work_hour -= 1;
                        }
                        
                        if(2 == st_mode_setting.set_work_time_flag)     //设置"开始工作时间段:分钟"
                        {
                            if(0 == st_mode_setting.custom_start_work_min)
                                st_mode_setting.custom_start_work_min = 59;
                            else
                                st_mode_setting.custom_start_work_min -= 1;
                        }
                        
                        if(3 == st_mode_setting.set_work_time_flag)     //设置"停止工作时间段:小时"
                        {
                            if(0 == st_mode_setting.custom_stop_work_hour)
                                st_mode_setting.custom_stop_work_hour = 23;
                            else
                                st_mode_setting.custom_stop_work_hour -= 1;
                        }
                        if(4 == st_mode_setting.set_work_time_flag)     //设置"停止工作时间段:分钟"
                        {
                            if(0 == st_mode_setting.custom_stop_work_min)
                                st_mode_setting.custom_stop_work_min = 59;
                            else
                                st_mode_setting.custom_stop_work_min -= 1;
                        }
                    }
                    
                    //2.自定义模式下:设置开几停几
                    else if(st_mode_setting.set_open_close_flag>0 && st_mode_setting.set_open_close_flag<3)
                    {
                        if(1 == st_mode_setting.set_open_close_flag)
                        {
                            if(1 == st_mode_setting.custom_open_min)
                                st_mode_setting.custom_open_min = 9;
                            else
                                st_mode_setting.custom_open_min -= 1;
                        }
                        if(2 == st_mode_setting.set_open_close_flag)
                        {
                            if(1 == st_mode_setting.custom_close_min)
                                st_mode_setting.custom_close_min = 9;
                            else
                                st_mode_setting.custom_close_min -= 1;
                        }
                    }
                }
            }
        break;
        
        //7.留存向左键
        case IR_CODE_LEFT:
            //设置时间
            //WS_DEBUG("[IR_CODE_LEFT] st_setting_status.setting_time_flag = %d\r\n", st_setting_status.setting_time_flag);
            if(1 == st_setting_status.setting_time_flag)
            {
                if(1 == st_time_setting.time_set_location)
                    st_time_setting.time_set_location = 3;
                else
                    st_time_setting.time_set_location -= 1;
                
                //WS_DEBUG("[IR_CODE_LEFT] st_time_setting.time_set_location = %d\r\n", st_time_setting.time_set_location);
            }
        break;
        
        //8.留存向右键
        case IR_CODE_RIGHT:
            //设置时间
            //WS_DEBUG("[IR_CODE_RIGHT] st_setting_status.setting_time_flag = %d\r\n", st_setting_status.setting_time_flag);
            if(1 == st_setting_status.setting_time_flag)
            {
                if(3 == st_time_setting.time_set_location)
                    st_time_setting.time_set_location = 1;
                else
                    st_time_setting.time_set_location += 1;
                
                //WS_DEBUG("[IR_CODE_RIGHT] st_time_setting.time_set_location = %d\r\n", st_time_setting.time_set_location);
            }
        break;
        
        //9.确认键短按
        case IR_CODE_CONFIRM_SHORT:
            //设置时间
            if(1 == st_setting_status.setting_time_flag)
            {
                //1.外置RTC重设时间
                //这里需要验证是否需要设置所有项: 年月日时分秒周???
                //这里需要设置所有时间项, 否则"强制清洗/雾化器切换"时间比对无法实现
                //DS3231_Set_hour(st_time_setting.hour_set_value);
                //DS3231_Set_min(st_time_setting.minu_set_value);
                //DS3231_Set_week(st_time_setting.week_set_value);
                ds3231_set_time(st_time_setting.hour_set_value, st_time_setting.minu_set_value, st_time_setting.week_set_value);
                
                //2.清除"时间设置标志"
                st_setting_status.setting_time_flag = 0;
                
                //3.复位"时间结构体"
                st_time_setting.hour_set_value = 0;     
                st_time_setting.minu_set_value = 0;     
                st_time_setting.week_set_value = 0;     
                st_time_setting.time_set_location = 0; 
                
                //4.设置时间结束标识置为1
                st_setting_status.mt_setting_finished = 1;
            }
            
            //*****************************************************************
            
            //设置模式时
            else if(1 == st_setting_status.setting_mode_flag)
            {
                //确认选择设置的"模式类型"(标准模式/自定义模式)
                if(0 == st_mode_setting.set_mode_type_flag)
                {
                    st_mode_setting.set_mode_type_flag = 1;                 //确认选择好"模式类型"
                    if(1 == st_mode_setting.mode_type_select)               //选择的"标准模式"
                    {
                        st_mode_setting.std_set_open_close_flag = 1;        //设置标准模式的开几停几
                    }
                    else if(2 == st_mode_setting.mode_type_select)          //选择的"自定义模式"
                    {
                        st_mode_setting.set_work_week_flag = 1;             //第一步: 设置周开关
                    }
                }
                else                                                        //已选择好"模式类型"
                {
                    //标准模式相关设置
                    if(1 == st_mode_setting.mode_type_select)
                    {
                        if(st_mode_setting.std_set_open_close_flag>0 && st_mode_setting.std_set_open_close_flag<3)
                        {
                            if(1 == st_mode_setting.std_set_open_close_flag)
                            {
                                st_mode_setting.std_set_open_close_flag = 2;
                            }
                            else
                            {
                                //至此, 整个标准模式设置完成, 设置标准模式的全局变量, 写入Flash
                                air_clean_cfg.standard_mode_open_min = st_mode_setting.standard_open_min;
                                air_clean_cfg.standard_mode_close_min = st_mode_setting.standard_close_min;
                                
                                //模式设为标准模式
                                air_clean_cfg.work_mode = MODE_STANDARD;
                                //记录到 Flash
                                write_cfg_to_flash(); 
                                
                                //这里也清除一下相关设置标志
                                //1.清除"模式设置标志"
                                st_setting_status.setting_mode_flag = 0;
                                
                                //2.复位"模式结构体"中的相关标识
                                st_mode_setting.set_mode_type_flag = 0;
                                st_mode_setting.set_work_week_flag = 0;
                                st_mode_setting.set_work_time_flag = 0;
                                st_mode_setting.set_open_close_flag = 0;
                                st_mode_setting.std_set_open_close_flag = 0;
                                
                                //3.设置模式结束标识置为1
                                st_setting_status.mt_setting_finished = 1;
                            }
                        }
                    }
                    
                    
                    
                    //-----------------------------------------------
                    
                    //自定义模式相关设置
                    else if(2 == st_mode_setting.mode_type_select)
                    {
                        //1.自定义模式设置周几是"工作"状态
                        if(st_mode_setting.set_work_week_flag>0 && st_mode_setting.set_work_week_flag<8)    //1-7 进入设置
                        {
                            set_custom_mode_work_week(st_mode_setting.set_work_week_flag, 1);               
                            //7 设置完之后, set_work_week_flag 设为8, 代表设置完成; 同时 set_work_time_flag 设为1, 准备设置工作时间段
                        }
                        
                        //2.自定义模式设置工作时间段
                        else if(8 == st_mode_setting.set_work_week_flag && st_mode_setting.set_work_time_flag>0 && st_mode_setting.set_work_time_flag<5)
                        {
                            if(4 == st_mode_setting.set_work_time_flag)
                            {
                                st_mode_setting.set_work_time_flag = 5;
                                st_mode_setting.set_open_close_flag = 1;
                                //4 设置完之后, set_work_time_flag 设为5, 代表工作时间段设置完成; 同时 set_open_close_flag 设为1, 准备设置开几停几
                            }
                            else
                            {
                                st_mode_setting.set_work_time_flag += 1;
                            }
                        }
                        
                        //3.自定义模式设置开几停几
                        else if(8 == st_mode_setting.set_work_week_flag && 5 == st_mode_setting.set_work_time_flag && st_mode_setting.set_open_close_flag>0 && st_mode_setting.set_open_close_flag<3)
                        {
                            if(1 == st_mode_setting.set_open_close_flag)
                            {
                                st_mode_setting.set_open_close_flag = 2;
                            }
                            else
                            {
                                //至此, 整个自定义模式设置完成, 设置自定义模式的全局变量, 写入Flash
                                
                                memcpy(air_clean_cfg.st_custom_mode.custom_work_week, st_mode_setting.custom_work_week, sizeof(st_mode_setting.custom_work_week));
                                
                                air_clean_cfg.st_custom_mode.custom_time_plan[0].start_hour = st_mode_setting.custom_start_work_hour;
                                air_clean_cfg.st_custom_mode.custom_time_plan[0].start_minute = st_mode_setting.custom_start_work_min;
                                air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_hour = st_mode_setting.custom_stop_work_hour;
                                air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_minute = st_mode_setting.custom_stop_work_min;
                                for(i=1; i<10; i++)
                                {
                                    air_clean_cfg.st_custom_mode.custom_time_plan[i].start_hour = 0;
                                    air_clean_cfg.st_custom_mode.custom_time_plan[i].start_minute = 0;
                                    air_clean_cfg.st_custom_mode.custom_time_plan[i].stop_hour = 0;
                                    air_clean_cfg.st_custom_mode.custom_time_plan[i].stop_minute = 0;
                                }
                                
                                
                                air_clean_cfg.st_custom_mode.custom_open_min = st_mode_setting.custom_open_min;
                                air_clean_cfg.st_custom_mode.custom_close_min = st_mode_setting.custom_close_min;
                                
                                //模式设为自定义模式
                                air_clean_cfg.work_mode = MODE_CUSTOM;
                                //记录到 Flash
                                write_cfg_to_flash(); 
                                
                                //这里也清除一下相关设置标志
                                //1.清除"模式设置标志"
                                st_setting_status.setting_mode_flag = 0;
                                
                                //2.复位"模式结构体"中的相关标识
                                st_mode_setting.set_mode_type_flag = 0;
                                st_mode_setting.set_work_week_flag = 0;
                                st_mode_setting.set_work_time_flag = 0;
                                st_mode_setting.set_open_close_flag = 0;
                                st_mode_setting.std_set_open_close_flag = 0;
                                
                                //3.设置模式结束标识置为1
                                st_setting_status.mt_setting_finished = 1;
                            }
                        }
                    }
                }
            }
            
            //*****************************************************************
            
            //清洗 等待 确认
            else if(1 == st_current_status.wait_wash_confirm)
            {
                //WS_DEBUG("2222222222222222222222222222222222222222222\r\n");
                st_current_status.wait_wash_confirm = 0;        //清除"等待清洗确认"标志
                st_current_status.current_wash_status = 1;      //
            }
        break;
        
        //10.确认键长按(清除错误码)
        case IR_CODE_CONFIRM_LONG:
            //if(BIT_GET(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT))
            //{}
            air_clean_cfg.err_code_bitmap = 0;
            
            //这里清除一下相关操作的标识: 清洗标识/加液标识 (应该不需要吧)
            //st_current_status.current_wash_status = 0;
            //st_current_status.add_plant_liquid_flag = 0;
            
            //错误码不需要记录到 Flash
            //write_cfg_to_flash(); 
        break;
        
        //11.复位键
        case IR_CODE_RESET:
            //恢复出厂设置, 时间不重置
            cfg_data_init();
            //清空 st_current_status
            memset(&st_current_status, 0, sizeof(st_current_status));
        break;
        
        //12.取消键
        case IR_CODE_CANCEL:
            //设置时间
            if(1 == st_setting_status.setting_time_flag)
            {
                //1.清除"时间设置标志"
                st_setting_status.setting_time_flag = 0;
                
                //2.复位"时间结构体"
                st_time_setting.hour_set_value = 0;     
                st_time_setting.minu_set_value = 0;     
                st_time_setting.week_set_value = 0;     
                st_time_setting.time_set_location = 0;  
            }
            
            //*****************************************************************
            
            //设置模式时
            else if(1 == st_setting_status.setting_mode_flag)
            {
                //自定义模式设置周几是"非工作"状态
                if(st_mode_setting.set_work_week_flag>0 && st_mode_setting.set_work_week_flag<8)
                {
                    set_custom_mode_work_week(st_mode_setting.set_work_week_flag, 0);
                }
                else
                {
                    //1.清除"模式设置标志"
                    st_setting_status.setting_mode_flag = 0;
                    
                    //2.复位"模式结构体"
                    st_mode_setting.set_mode_type_flag = 0;
                    st_mode_setting.set_work_week_flag = 0;
                    st_mode_setting.set_work_time_flag = 0;
                    st_mode_setting.set_open_close_flag = 0;
                    st_mode_setting.std_set_open_close_flag = 0;
                }
            }
            
            //显示工作仓液位时
            else if(SHOW_WORK_LIQUID_LEVEL == st_current_status.current_number_show)
            {
                st_current_status.current_number_show = SHOW_TIME;
            }
            
            //清洗+取消 : 用作一键排液功能
            else if(1 == st_current_status.wait_wash_confirm)
            {
                if(0 == air_clean_cfg.onekey_drain_fished)      //一键清洗未完成
                {
                    st_current_status.wait_wash_confirm = 0;    //清除"等待确认清洗"标志
                    if(0 == st_current_status.onekey_drain_status)
                    {
                        st_current_status.onekey_drain_status = 1;   //执行一键排液的标识
                    }
                    //正在一键排液则忽略
                }
                else                                            //一键清洗已完成，取消已完成标志
                {
                    air_clean_cfg.onekey_drain_fished = 0;
                    write_cfg_to_flash();
                }
            }
        break;
        
        //13.清洗短按 + 确认 (手动进入清洗程序)
        case IR_CODE_WASH_SHORT:
            //无论手动还是自动清洗状态, 按清洗加确认都可进行人为清洗
            //这里按了清洗按键, 等待清洗的选择: 通常清洗, 保养清洗
            st_current_status.wait_wash_confirm = 1;
        break;
        
        //14.清洗长按(自动清洗与手动清洗模式切换)
        case IR_CODE_WASH_LONG:
            //0: 自动清洗; 1: 手动清洗
            if(0 == air_clean_cfg.wash_mode)
                air_clean_cfg.wash_mode = 1;
            else
                air_clean_cfg.wash_mode = 0;
            write_cfg_to_flash(); 
        break;
        
        //15.浓度调节(liquid_density -- 写入Flash)
        //根据调节的浓度, 抽液时间分3挡: 3秒、5秒、7秒
        case IR_CODE_DENSITY:
            //原液箱不在低液位时才能改变浓度
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) != PULLDOWN)
            {
                //浓度: 1,2,3 (为了与WIFI协议统一) 不再使用0,1,2
                //air_clean_cfg.liquid_density = ((air_clean_cfg.liquid_density+1)%3);
                if(LIQUID_DENSITY_THREE == air_clean_cfg.liquid_density)    //3
                {
                    air_clean_cfg.liquid_density = LIQUID_DENSITY_ONE;      //1
                }else
                {
                    air_clean_cfg.liquid_density += 1;
                }
                //WS_DEBUG("[update_status_by_data_code] air_clean_cfg.liquid_density = %d \r\n",air_clean_cfg.liquid_density);
                //加液浓度有记忆功能, 需要写入Flash
                write_cfg_to_flash(); 
            }
        break;
        
        //16.风量调节(air_fan_level -- 写入Flash)
        case IR_CODE_FAN_LEVEL:
            WS_DEBUG("按键操作 [IR_CODE_FAN_LEVEL] 设置风量等级为：%d \r\n", air_clean_cfg.air_fan_level);
            //风量等级: 1,2,3 (为了与WIFI协议统一) 不再使用0,1,2
            //air_clean_cfg.air_fan_level = ((air_clean_cfg.air_fan_level+1)%3);
            if(AIR_FAN_LEVEL_THREE == air_clean_cfg.air_fan_level)  //3
            {
                air_clean_cfg.air_fan_level = AIR_FAN_LEVEL_ONE;    //1
            }else
            {
                air_clean_cfg.air_fan_level += 1;
            }
            st_current_status.siwtch_fan_level_flag = ON;
            //风量等级有记忆功能, 需要写入Flash
            write_cfg_to_flash(); 
        break;
        
        //17.加液键短按
        case IR_CODE_ADD_LIQUID_SHORT:
            //液箱启动或停止加液程序
            if(st_current_status.add_plant_liquid_flag == 0)
                st_current_status.add_plant_liquid_flag = 1;
            else
                st_current_status.add_plant_liquid_flag = 0;
        break;
        
        //18.加液键长按
        case IR_CODE_ADD_LIQUID_LONG:
            //查看工作仓液位, 工作仓液位实时显示, 在屏幕刷新中获取液位值; 按返回键, 显示时间
            st_current_status.current_number_show = SHOW_WORK_LIQUID_LEVEL;
        break;
        
        //19.雾化器短按
        case IR_CODE_ATOMIZER_SHORT:
            //雾化器只能在未锁定状态下进行切换, 锁定状态禁止切换
            if(0 == air_clean_cfg.atomizer_lock)
            {
                if(air_clean_cfg.which_atomizer == ATOMIZER_ONE)
                    air_clean_cfg.which_atomizer = ATOMIZER_TWO;
                else
                    air_clean_cfg.which_atomizer = ATOMIZER_ONE;
                
                //WS_DEBUG("[IR_CODE_ATOMIZER_SHORT] air_clean_cfg.which_atomizer = %d \r\n", air_clean_cfg.which_atomizer);
                
                //更新雾化器使用起始时间
                update_cycle_time(ATOMIZER_INTERVAL_TYPE);
                
                st_current_status.siwtch_atomizer_flag = ON;
                //雾化器切换之后, 写入Flash
                write_cfg_to_flash(); 
            }
            
        break;
        
        //20.雾化器长按
        case IR_CODE_ATOMIZER_LONG:
            
            //长按3秒“雾化器”键蜂鸣声后切换并锁定雾化器，再次长按3秒“雾化器”键蜂鸣声后，解锁雾化器锁定
            if(0 == air_clean_cfg.atomizer_lock)
            {
                air_clean_cfg.atomizer_lock = 1;
                /*
                if(air_clean_cfg.which_atomizer == ATOMIZER_ONE)
                    air_clean_cfg.which_atomizer = ATOMIZER_TWO;
                else
                    air_clean_cfg.which_atomizer = ATOMIZER_ONE;
                st_current_status.siwtch_atomizer_flag = ON;
                */
            }
            else
                air_clean_cfg.atomizer_lock = 0;
            //锁定状态写入Flash
            write_cfg_to_flash(); 
        break;
        
        //21.氨气值(显示屏显示氨气值)
        case IR_CODE_SHOW_NH3:
            if(SHOW_NH3 == st_current_status.current_number_show)
                st_current_status.current_number_show = SHOW_TIME;
            else
                st_current_status.current_number_show = SHOW_NH3;
            //WS_DEBUG("[IR_CODE_SHOW_NH3] st_current_status.current_number_show = %d \r\n", st_current_status.current_number_show);
            /*
            WS_DEBUG("\r\n[IR_CODE_SHOW_NH3] st_current_status.current_number_show = %d \r\n", st_current_status.current_number_show);
            st_current_status.nh3_show_real_value = 1.3;
            WS_DEBUG("[IR_CODE_SHOW_NH3] st_current_status.nh3_show_real_value = %2.2f \r\n", st_current_status.nh3_show_real_value);
            sprintf(str, "%2.2f", st_current_status.nh3_show_real_value);
            WS_DEBUG("[IR_CODE_SHOW_NH3] nh3_show_real_value str = %s \r\n\r\n", str);
            */
        break;
        
        //22.取消键长按
        //一键雾化: 当机器不处于运行时间，通过长按返回键使雾化器雾化2分钟后自动恢复当前设置模式
        case IR_CODE_CANCEL_LONG:
                WS_DEBUG("按键操作 [IR_CODE_CANCEL_LONG] 执行一键雾化功能 \r\n");
                if(!get_whole_dev_onoff())
                {
                    st_current_status.one_key_atomize_flag = 1;
                }
                else
                {
                    st_current_status.one_key_atomize_flag = 0;
                }
                
            break;
        /*
        #if TEST_DS3231
            u8 ds_year, ds_month, ds_date;
            u8 ds_hour, ds_mint, ds_secd;
            
            WS_DEBUG("[IR_CODE_CANCEL_LONG] 测试用途，当前时间： %d-%d-%d WEEK%d %d:%d:%d \r\n", \
                    calendar.w_year, calendar.w_month, calendar.w_date, calendar.week, \
                    calendar.hour,calendar.min,calendar.sec);
            
            WS_DEBUG("\r\n ************** 手动设置日期时间 ************** \r\n");
            WS_DEBUG("\r\n 当前年份为: 2018 \r\n");
            ds_year = 18;
            
            WS_DEBUG("\r\n 请输入月(1-12): \r\n");
            scanf("%d",&ds_month);
            
            WS_DEBUG("\r\n 请输入日(1-30): \r\n");
            scanf("%d",&ds_date);
            
            WS_DEBUG("\r\n 请输入小时(0-23): \r\n");
            scanf("%d",&ds_hour);
            
            WS_DEBUG("\r\n 请输入分钟(0-59): \r\n");
            scanf("%d",&ds_mint);
            
            WS_DEBUG("\r\n 即将设置的日期时间为: 2018-%d-%d %d:%d:%d \r\n", \
                    ds_month, ds_date, ds_hour, ds_mint, ds_secd);
            
            WS_DEBUG("\r\n 确认设置？请输入 y/Y \r\n");
            scanf("%c",&ds_confirm);
            if( ('y' == ds_confirm) || ('Y' == ds_confirm) )
            {
                DS3231_Set(syear, smon, sday, week, hour, min, sec);
                WS_DEBUG("[IR_CODE_CANCEL_LONG] 测试用途，设置成功时间： %d-%d-%d WEEK%d %d:%d:%d \r\n", \
                        calendar.w_year, calendar.w_month, calendar.w_date, calendar.week, \
                        calendar.hour,calendar.min,calendar.sec);
            }
        #endif
        */
        //23.默认
        default:
            //pass
        break;
    }
}


/*
*********************************************************************************************************
*                                          SCREEN REFRESH TASK
*********************************************************************************************************
*/
//char digit_to_led_data[10] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
char digit_to_led_data[10] = {0x3f,0x06,0x9b,0x8f,0xa6,0xad,0xbd,0x07,0xbf,0xaf};
//0: 1111 1100 --> 1111 1100 --> 0011 1111 --> 0x3f
//1: 0110 0000 --> 0110 0000 --> 0000 0110 --> 0x06
//2: 1101 1010 --> 1101 1001 --> 1001 1011 --> 0x9B
//3: 1111 0010 --> 1111 0001 --> 1000 1111 --> 0x8F
//4: 0110 0110 --> 0110 0101 --> 1010 0110 --> 0xA6
//5: 1011 0110 --> 1011 0101 --> 1010 1101 --> 0xAD
//6: 1011 1110 --> 1011 1101 --> 1011 1101 --> 0xBD
//7: 1110 0000 --> 1110 0000 --> 0000 0111 --> 0x07
//8: 1111 1110 --> 1111 1101 --> 1011 1111 --> 0xBF
//9: 1111 0110 --> 1111 0101 --> 1010 1111 --> 0xAF

char display_to_led_data[4] = {0xb9,0xb1,0xb6,0x80};
//E: 1001 1110 --> 1001 1101 --> 1011 1001 --> 0xB9
//F: 1000 1110 --> 1000 1101 --> 1011 0001 --> 0xB1
//H: 0110 1110 --> 0110 1101 --> 1011 0110 --> 0xB6
//-: 0000 0010 --> 0000 0001 --> 1000 0000 --> 0x80


int display_current_version(char position)
{
    //8L自动单机通用版 -- E112
    if(AUTO_ALONE_8L == PRODUCT_TYPE)
    {
        switch(position)
        {
            case 0:
                return display_to_led_data[0];
                break;
            case 1:
                return digit_to_led_data[1];
                break;
            case 2:
                return digit_to_led_data[1];
                break;
            case 3:
                return digit_to_led_data[2];
                break;
        }
    }
    //8L自动教育版 -- E212
    else if(AUTO_EDUCATE_8L == PRODUCT_TYPE)
    {
        switch(position)
        {
            case 0:
                return display_to_led_data[0];
                break;
            case 1:
                return digit_to_led_data[2];
                break;
            case 2:
                return digit_to_led_data[1];
                break;
            case 3:
                return digit_to_led_data[2];
                break;
        }
    }
}


int get_err_num()
{
    int err_num = 0;
    int display_dig = 0;
    
    if((get_err_code() != 0))    //第四个数字显示错误码
    {
        display_dig = get_err_code();
        //
        if( BIT_GET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_MANY_TIMES) )
        {
            return ERR_OVERFLOW_MANY_TIMES;
        }
        else if( BIT_GET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_FIRST) )
        {
            return ERR_OVERFLOW_FIRST;
        }
        
        //WS_DEBUG("[get_err_num] display_dig = %d \r\n", display_dig);
        //同时发生的故障码，可能有多个，但屏幕上只显示优先级最高的那个错误码。
        while(!(display_dig & 0x00000001)) {
            err_num++;
            display_dig = display_dig >> 1;
        }
        //WS_DEBUG("[get_err_num] err_num = %d \r\n", err_num);
        return err_num;
    }
}

/*
//出现错误码, 第四个数码管调用该函数, 显示错误码
int display_err_code()
{
    int err_num = 0;
    int display_dig = 0;
    
    if((get_err_code() != 0))    //第四个数字显示错误码
    {
        display_dig = get_err_code();
        //WS_DEBUG("[display_err_code] display_dig = %d \r\n", display_dig);
        //同时发生的故障码，可能有多个，但屏幕上只显示优先级最高的那个错误码。
        while(!(display_dig & 0x00000001)) {
            err_num++;
            display_dig = display_dig >> 1;
        }
        //WS_DEBUG("[display_err_code] err_num = %d \r\n", err_num);
        return digit_to_led_data[err_num];
    }
}
*/

int display_current_time(char position)
{
    //get_show_time();              //屏幕刷新线程一直在更新时间数据
    int tm_hour = calendar.hour;
    int tm_min = calendar.min;
    
    switch(position)
    {
        case 0:
            return digit_to_led_data[tm_hour/10];
            break;
        case 1:
            return digit_to_led_data[tm_hour%10];
            break;
        case 2:
            return digit_to_led_data[tm_min/10];
            break;
        case 3:
            return digit_to_led_data[tm_min%10];
            break;
    }
}

int display_NH3_value(char position)
{
    //获取氨气值
    //float nh3_value = 0.5;
    //float nh3_value = 1.8978;
    //float nh3_value = 0.05;
    
    //float nh3_value = st_current_status.current_nh3_value;
    float nh3_value = st_current_status.nh3_show_real_value;
    
    int iInteger, iDecimal;
    iInteger = nh3_value;
    iDecimal = (int)(nh3_value*100)%100;
    
    switch(position)
    {
        case 0:
            if(0 == iInteger/10)
                return 0x00;
            else
                return digit_to_led_data[iInteger/10];
            break;
        case 1:
            return digit_to_led_data[iInteger%10];
            break;
        case 2:
            return digit_to_led_data[iDecimal/10];
            break;
        case 3:
            return digit_to_led_data[iDecimal%10];
            break;
    }
}

int display_setting_time(char position)
{
    int tm_hour = st_time_setting.hour_set_value;
    int tm_min = st_time_setting.minu_set_value;
    int tm_week = st_time_setting.week_set_value;
    
    switch(position)
    {
        case 0:
            return digit_to_led_data[tm_hour/10];
            break;
        case 1:
            return digit_to_led_data[tm_hour%10];
            break;
        case 2:
            return digit_to_led_data[tm_min/10];
            break;
        case 3:
            return digit_to_led_data[tm_min%10];
            break;
        case 4:
            return digit_to_led_data[tm_week];
            break;
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//控制"植物液图标"
//light_off: 0--灭; 1--亮  
void plant_icon_display_control(unsigned char * row_data, char light_off)
{
    if(1 == light_off)          //亮
    {
        BIT_SET(row_data[4],1);             //B5
        BIT_SET(row_data[4],2);             //C5
        BIT_SET(row_data[4],3);             //D5
        //原液箱低液位
        if(st_current_status.current_plant_liquid == PLANT_LIQUID_LOW)
        {
            //WS_DEBUG("[update_display_data] 00000 PLANT_LIQUID_LOW\r\n");
            //三片红
            BIT_CLEAR(row_data[4],0);       //A5
            BIT_SET(row_data[9],0);         //A10
            BIT_CLEAR(row_data[4],4);       //E5
            BIT_SET(row_data[9],1);         //B10
            BIT_CLEAR(row_data[4],5);       //F5
            BIT_SET(row_data[9],2);         //C10
        }
        //原液箱中液位
        else if(st_current_status.current_plant_liquid == PLANT_LIQUID_MID)
        {
            //WS_DEBUG("[update_display_data] 11111 PLANT_LIQUID_MID\r\n");
            //一片白两片红
            BIT_CLEAR(row_data[4],0);       //A5
            BIT_SET(row_data[9],0);         //A10
            BIT_CLEAR(row_data[4],4);       //E5
            BIT_SET(row_data[9],1);         //B10
            BIT_SET(row_data[4],5);         //F5
            BIT_CLEAR(row_data[9],2);       //C10
        }
        //原液箱高液位
        else if(st_current_status.current_plant_liquid == PLANT_LIQUID_HIGH)
        {
            //WS_DEBUG("[update_display_data] 22222 PLANT_LIQUID_HIGH\r\n");
            BIT_SET(row_data[4],0);         //A5
            BIT_CLEAR(row_data[9],0);       //A10
            BIT_SET(row_data[4],4);         //E5
            BIT_CLEAR(row_data[9],1);       //B10
            BIT_SET(row_data[4],5);         //F5
            BIT_CLEAR(row_data[9],2);       //C10
        }
    }
    else                        //灭
    {
        BIT_CLEAR(row_data[4],1);               //B5
        BIT_CLEAR(row_data[4],2);               //C5
        BIT_CLEAR(row_data[4],3);               //D5
        
        BIT_CLEAR(row_data[4],0);               //A5
        BIT_CLEAR(row_data[9],0);               //A10
        BIT_CLEAR(row_data[4],4);               //E5
        BIT_CLEAR(row_data[9],1);               //B10
        BIT_CLEAR(row_data[4],5);               //F5
        BIT_CLEAR(row_data[9],2);               //C10
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//控制"NH3字体+图标"
//light_off: 0--灭; 1--亮(全部点亮)
void NH3_mark_display_control(unsigned char * row_data, char light_off)
{
    if(1 == light_off)          //亮(这里全部点亮, 可区分颜色)
    {
        BIT_SET(row_data[0],6);         //H1
        BIT_SET(row_data[9],3);         //D10
        
        BIT_SET(row_data[5],0);         //A6
        BIT_SET(row_data[5],1);         //B6
        BIT_SET(row_data[9],4);         //E10
        BIT_SET(row_data[9],5);         //F10
    }
    else                        //灭
    {
        BIT_CLEAR(row_data[0],6);       //H1
        BIT_CLEAR(row_data[9],3);       //D10
        
        BIT_CLEAR(row_data[5],0);       //A6
        BIT_CLEAR(row_data[5],1);       //B6
        BIT_CLEAR(row_data[9],4);       //E10
        BIT_CLEAR(row_data[9],5);       //F10
    }
}

//控制"时间字体+图标"
//light_off: 0--灭; 1--亮白灯; 2--亮粉灯; 3--橘黄色
//正常显示时间, 显示白色; 设置时间相关, 显示粉色或橘黄色
void time_mark_display_control(unsigned char * row_data, char light_off)
{
    if(1 == light_off)          //显示白色
    {
        //时间图标
        BIT_SET(row_data[3],6);             //H4
        BIT_CLEAR(row_data[9],7);           //G10
        //时间字体
        BIT_SET(row_data[5],2);             //C6
        BIT_CLEAR(row_data[9],6);           //H10
    }
    else if(2 == light_off)     //显示粉色
    {
        //时间图标
        BIT_SET(row_data[3],6);             //H4
        BIT_SET(row_data[9],7);             //G10
        //时间字体
        BIT_SET(row_data[5],2);             //C6
        BIT_SET(row_data[9],6);             //H10
    }
    else if(3 == light_off)     //显示橘黄色
    {
        //时间图标
        BIT_CLEAR(row_data[3],6);           //H4
        BIT_SET(row_data[9],7);             //G10
        //时间字体
        BIT_CLEAR(row_data[5],2);           //C6
        BIT_SET(row_data[9],6);             //H10
    }
    else                        //灭
    {
        //时间图标
        BIT_CLEAR(row_data[3],6);           //H4
        BIT_CLEAR(row_data[9],7);           //G10
        //时间字体
        BIT_CLEAR(row_data[5],2);           //C6
        BIT_CLEAR(row_data[9],6);           //H10
    }
}

//控制"周字体+图标"
//light_off: 0--灭; 1--亮白灯; 2--亮红灯
void week_mark_display_control(unsigned char * row_data, char light_off)
{
    if(1 == light_off)          //显示白色
    {
        //周图标 : 白色
        BIT_SET(row_data[5],6);             //H6
        BIT_CLEAR(row_data[10],0);          //A11
        //周字体 : 白色
        BIT_SET(row_data[8],6);             //H9
        BIT_CLEAR(row_data[10],1);          //B11
    }
    else if(2 == light_off)     //显示粉色(全部点亮)
    {
        //周图标 : 粉色
        BIT_SET(row_data[5],6);             //H6
        BIT_SET(row_data[10],0);            //A11
        //周字体 : 粉色
        BIT_SET(row_data[8],6);             //H9
        BIT_SET(row_data[10],1);            //B11
    }
    else                        //灭
    {
        //周图标
        BIT_CLEAR(row_data[5],6);           //H6
        BIT_CLEAR(row_data[10],0);          //A11
        //周字体
        BIT_CLEAR(row_data[8],6);           //H9
        BIT_CLEAR(row_data[10],1);          //B11
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//条件控制显示
//主要控制以下图标: 1."四个数码管+中间两点"; 2.NH3标识; 3.时间标识; 4.周标识
/**
 * condition():
 *      0: 显示错误码
 *      1: 显示当前时间
 *      2: 显示氨气值
 *      3: 时间设置显示
 *      4: 模式设置显示
 *      5: 显示工作仓液位
 */
void condition_display_control(unsigned char * row_data, char condition)
{
    int err_num = 0;
    //显示错误码
    if(0 == condition)
    {
        err_num = get_err_num();
        if(err_num>=10)
        {
            row_data[0] = 0x00;         //数码管第一个数字(灭)
            row_data[1] = 0xB9;         //数码管第二个数字显示E
            
            //数码管第三四两个数字显示错误码
            row_data[2] = digit_to_led_data[err_num/10];
            row_data[3] = digit_to_led_data[err_num%10];
        }
        else
        {
            row_data[0] = 0x00;         //数码管第一个数字(灭)
            row_data[1] = 0x00;         //数码管第二个数字(灭)
            
            row_data[2] = 0xB9;         //数码管第三个数字显示E, 根据原理图置为'1001 1101' 
            //row_data[3] = display_err_code();   //数码管第四个数字显示错误码值
            row_data[3] = digit_to_led_data[err_num];
        }

        
        //中间两点
        //BIT_CLEAR(row_data[1],6);       //H2
        //BIT_CLEAR(row_data[2],6);       //H3
        
        NH3_mark_display_control(row_data, 0);
        time_mark_display_control(row_data, 0);
        week_mark_display_control(row_data, 0);
    }
    
    //显示当前时间值
    else if(1 == condition)
    {
        if(0 == sepa_dig_flick_flag)
            sepa_dig_flick_flag = 1;
        else
            sepa_dig_flick_flag = 0;
        
        row_data[0] = display_current_time(0);  //第一个数字
        row_data[1] = display_current_time(1);  //第二个数字
        row_data[2] = display_current_time(2);  //第三个数字
        row_data[3] = display_current_time(3);  //第四个数字
        
        if(1 == sepa_dig_flick_flag)    //亮
        {
            //WS_DEBUG("[condition_display_control] display_current_time: 1 == sepa_dig_flick_flag \r\n");
            BIT_SET(row_data[1],6);         //H2
            BIT_SET(row_data[2],6);         //H3
        }
        else                            //灭
        {
            //WS_DEBUG("[condition_display_control] display_current_time: 0 == sepa_dig_flick_flag \r\n");
            BIT_CLEAR(row_data[1],6);       //H2
            BIT_CLEAR(row_data[2],6);       //H3
        }
        
        NH3_mark_display_control(row_data, 0);
        time_mark_display_control(row_data, 1);
        week_mark_display_control(row_data, 0);
    }
    
    //显示氨气值
    else if(2 == condition)
    {
        row_data[0] = display_NH3_value(0);     //第一个数字
        row_data[1] = display_NH3_value(1);     //第二个数字
        row_data[2] = display_NH3_value(2);     //第三个数字
        row_data[3] = display_NH3_value(3);     //第四个数字

        BIT_CLEAR(row_data[1],6);       //H2
        BIT_SET(row_data[2],6);         //H3
        
        NH3_mark_display_control(row_data, 1);
        time_mark_display_control(row_data, 0);
        week_mark_display_control(row_data, 0);
    }
    
    //时间设置显示
    else if(3 == condition)
    {   
        //WS_DEBUG("[condition_display_control] st_time_setting.time_set_location = %d \r\n", st_time_setting.time_set_location);
        //正在设置小时  
        if(1 == st_time_setting.time_set_location)
        {
            if(0 == head_dig_flick_flag)
                head_dig_flick_flag = 1;
            else
                head_dig_flick_flag = 0;
            
            if(1 == head_dig_flick_flag)    //亮
            {
                row_data[0] = display_setting_time(0);  //第一个数字
                row_data[1] = display_setting_time(1);  //第二个数字
            }
            else                            //灭
            {
                row_data[0] = 0x00;
                row_data[1] = 0x00;
            }
            
            row_data[2] = display_setting_time(2);  //第三个数字
            row_data[3] = display_setting_time(3);  //第四个数字
            
            BIT_SET(row_data[1],6);         //H2
            BIT_SET(row_data[2],6);         //H3
            
            time_mark_display_control(row_data, 1);
            week_mark_display_control(row_data, 0);
        }
        
        //正在设置分钟
        else if(2 == st_time_setting.time_set_location)
        {
            if(0 == tail_dig_flick_flag)
                tail_dig_flick_flag = 1;
            else
                tail_dig_flick_flag = 0;
            
            row_data[0] = display_setting_time(0);  //第一个数字
            row_data[1] = display_setting_time(1);  //第二个数字
            
            if(1 == tail_dig_flick_flag)    //亮
            {
                row_data[2] = display_setting_time(2);  //第三个数字
                row_data[3] = display_setting_time(3);  //第四个数字
            }
            else                            //灭
            {
                row_data[2] = 0x00;
                row_data[3] = 0x00;
            }
            
            BIT_SET(row_data[1],6);         //H2
            BIT_SET(row_data[2],6);         //H3
            
            time_mark_display_control(row_data, 1);
            week_mark_display_control(row_data, 0);
        }
        
        //正在设置周
        else if(3 == st_time_setting.time_set_location)
        {
            if(0 == tail_dig_flick_flag)
                tail_dig_flick_flag = 1;
            else
                tail_dig_flick_flag = 0;
            
            row_data[0] = 0x00;
            row_data[1] = 0x00;
            row_data[2] = 0x00;
            if(1 == tail_dig_flick_flag)    //亮
            {
                //获取正在设置的周
                row_data[3] = display_setting_time(4);  //第四个数字
            }
            else                            //灭
            {
                row_data[3] = 0x00;
            }
            
            time_mark_display_control(row_data, 0);
            week_mark_display_control(row_data, 1);
        }
        
        NH3_mark_display_control(row_data, 0);
    }
    
    //模式设置显示
    else if(4 == condition)
    {
        //标准模式设置开几停几
        if(1 == st_mode_setting.mode_type_select)
        {
            if(st_mode_setting.std_set_open_close_flag>0 && st_mode_setting.std_set_open_close_flag<3)
            {
                if(1 == st_mode_setting.std_set_open_close_flag)        //开几
                {
                    if(0 == tail_dig_flick_flag)
                        tail_dig_flick_flag = 1;
                    else
                        tail_dig_flick_flag = 0;
                    
                    row_data[0] = 0x00;
                    row_data[2] = 0x00;
                    row_data[3] = digit_to_led_data[st_mode_setting.standard_close_min%10];
                    
                    if(1 == tail_dig_flick_flag)    //亮
                    {
                        row_data[1] = digit_to_led_data[st_mode_setting.standard_open_min%10];  //第二个数字
                        time_mark_display_control(row_data, 3);
                    }
                    else                            //灭
                    {
                        row_data[1] = 0x00;
                        time_mark_display_control(row_data, 0);
                    }
                }
                
                if(2 == st_mode_setting.std_set_open_close_flag)        //停几
                {
                    if(0 == tail_dig_flick_flag)
                        tail_dig_flick_flag = 1;
                    else
                        tail_dig_flick_flag = 0;
                    
                    row_data[0] = 0x00;
                    row_data[1] = digit_to_led_data[st_mode_setting.standard_open_min%10];        //第二个数字(开几)
                    row_data[2] = 0x00;
                    
                    if(1 == tail_dig_flick_flag)    //亮
                    {
                        row_data[3] = digit_to_led_data[st_mode_setting.standard_close_min%10];   //第四个数字(停几)
                        time_mark_display_control(row_data, 3);
                    }
                    else                            //灭
                    {
                        row_data[3] = 0x00;
                        time_mark_display_control(row_data, 0);
                    }
                }
                week_mark_display_control(row_data, 0);
            }
        }
        
        
        //自定义模式设置
        else if(2 == st_mode_setting.mode_type_select)
        {
            //正在设置周开关
            if(st_mode_setting.set_work_week_flag>0 && st_mode_setting.set_work_week_flag<8)
            {
                if(0 == tail_dig_flick_flag)
                    tail_dig_flick_flag = 1;
                else
                    tail_dig_flick_flag = 0;
                
                row_data[0] = 0x00;
                row_data[1] = 0x00;
                row_data[2] = 0x00;
                if(1 == tail_dig_flick_flag)    //亮
                {
                    //获取正在设置的周开关
                    row_data[3] = digit_to_led_data[st_mode_setting.set_work_week_flag];  //第四个数字
                    //周图标显示粉色, 闪烁
                    week_mark_display_control(row_data, 2);
                }
                else                            //灭
                {
                    row_data[3] = 0x00;
                    week_mark_display_control(row_data, 0);
                }
                
                time_mark_display_control(row_data, 0);
            }
            
            //正在设置工作时间段
            if(8 == st_mode_setting.set_work_week_flag && st_mode_setting.set_work_time_flag>0 && st_mode_setting.set_work_time_flag<5)
            {
                //WS_DEBUG("[condition_display_control] st_mode_setting.set_work_time_flag = %d \r\n", st_mode_setting.set_work_time_flag);
                if(1 == st_mode_setting.set_work_time_flag)
                {
                    if(0 == head_dig_flick_flag)
                        head_dig_flick_flag = 1;
                    else
                        head_dig_flick_flag = 0;
                    
                    row_data[2] = digit_to_led_data[st_mode_setting.custom_start_work_min/10];      //第三个数字
                    row_data[3] = digit_to_led_data[st_mode_setting.custom_start_work_min%10];      //第四个数字
                    
                    if(1 == head_dig_flick_flag)    //亮
                    {
                        row_data[0] = digit_to_led_data[st_mode_setting.custom_start_work_hour/10];  //第一个数字
                        row_data[1] = digit_to_led_data[st_mode_setting.custom_start_work_hour%10];  //第二个数字
                        time_mark_display_control(row_data, 2);             //时间图标粉色闪烁
                    }
                    else                            //灭
                    {
                        row_data[0] = 0x00;
                        row_data[1] = 0x00;
                        time_mark_display_control(row_data, 0);
                    }
                    
                    //row_data[2] = 0x00;
                    //row_data[3] = 0x00;
                }
                
                else if(2 == st_mode_setting.set_work_time_flag)
                {
                    if(0 == tail_dig_flick_flag)
                        tail_dig_flick_flag = 1;
                    else
                        tail_dig_flick_flag = 0;
                    
                    //row_data[0] = 0x00;
                    //row_data[1] = 0x00;
                    row_data[0] = digit_to_led_data[st_mode_setting.custom_start_work_hour/10];     //第一个数字
                    row_data[1] = digit_to_led_data[st_mode_setting.custom_start_work_hour%10];     //第二个数字
                    
                    if(1 == tail_dig_flick_flag)    //亮
                    {
                        row_data[2] = digit_to_led_data[st_mode_setting.custom_start_work_min/10];  //第三个数字
                        row_data[3] = digit_to_led_data[st_mode_setting.custom_start_work_min%10];  //第四个数字
                        time_mark_display_control(row_data, 2);             //时间图标粉色闪烁
                    }
                    else                            //灭
                    {
                        row_data[2] = 0x00;
                        row_data[3] = 0x00;
                        time_mark_display_control(row_data, 0);
                    }
                }
                
                else if(3 == st_mode_setting.set_work_time_flag)
                {
                    if(0 == head_dig_flick_flag)
                        head_dig_flick_flag = 1;
                    else
                        head_dig_flick_flag = 0;
                    
                    row_data[2] = digit_to_led_data[st_mode_setting.custom_stop_work_min/10];       //第三个数字
                    row_data[3] = digit_to_led_data[st_mode_setting.custom_stop_work_min%10];       //第四个数字
                        
                    if(1 == head_dig_flick_flag)    //亮
                    {
                        row_data[0] = digit_to_led_data[st_mode_setting.custom_stop_work_hour/10];  //第一个数字
                        row_data[1] = digit_to_led_data[st_mode_setting.custom_stop_work_hour%10];  //第二个数字
                        time_mark_display_control(row_data, 2);             //时间图标粉色闪烁
                    }
                    else                            //灭
                    {
                        row_data[0] = 0x00;
                        row_data[1] = 0x00;
                        time_mark_display_control(row_data, 0);
                    }
                    
                    //row_data[2] = 0x00;
                    //row_data[3] = 0x00;
                }
                
                else if(4 == st_mode_setting.set_work_time_flag)
                {
                    if(0 == tail_dig_flick_flag)
                        tail_dig_flick_flag = 1;
                    else
                        tail_dig_flick_flag = 0;
                    
                    //row_data[0] = 0x00;
                    //row_data[1] = 0x00;
                    row_data[0] = digit_to_led_data[st_mode_setting.custom_stop_work_hour/10];      //第一个数字
                    row_data[1] = digit_to_led_data[st_mode_setting.custom_stop_work_hour%10];      //第二个数字
                    
                    if(1 == tail_dig_flick_flag)    //亮
                    {
                        row_data[2] = digit_to_led_data[st_mode_setting.custom_stop_work_min/10];  //第三个数字
                        row_data[3] = digit_to_led_data[st_mode_setting.custom_stop_work_min%10];  //第四个数字
                        time_mark_display_control(row_data, 2);             //时间图标粉色闪烁
                    }
                    else                            //灭
                    {
                        row_data[2] = 0x00;
                        row_data[3] = 0x00;
                        time_mark_display_control(row_data, 0);
                    }
                }
                
                BIT_SET(row_data[1],6);         //H2
                BIT_SET(row_data[2],6);         //H3
                
                week_mark_display_control(row_data, 0);
            }
            
            //正在设置开几停几
            if(8 == st_mode_setting.set_work_week_flag && 5 == st_mode_setting.set_work_time_flag && st_mode_setting.set_open_close_flag>0 && st_mode_setting.set_open_close_flag<3)
            {
                if(1 == st_mode_setting.set_open_close_flag)        //开几
                {
                    if(0 == tail_dig_flick_flag)
                        tail_dig_flick_flag = 1;
                    else
                        tail_dig_flick_flag = 0;
                    
                    row_data[0] = 0x00;
                    row_data[2] = 0x00;
                    row_data[3] = digit_to_led_data[st_mode_setting.custom_close_min%10];
                    
                    if(1 == tail_dig_flick_flag)    //亮
                    {
                        row_data[1] = digit_to_led_data[st_mode_setting.custom_open_min%10];  //第四个数字
                        time_mark_display_control(row_data, 3);
                    }
                        
                    else                            //灭
                    {
                        row_data[1] = 0x00;
                        time_mark_display_control(row_data, 0);
                    }
                        
                }
                
                if(2 == st_mode_setting.set_open_close_flag)        //停几
                {
                    if(0 == tail_dig_flick_flag)
                        tail_dig_flick_flag = 1;
                    else
                        tail_dig_flick_flag = 0;
                    
                    row_data[0] = 0x00;
                    row_data[1] = digit_to_led_data[st_mode_setting.custom_open_min%10];        //第二个数字(开几)
                    row_data[2] = 0x00;
                    
                    if(1 == tail_dig_flick_flag)    //亮
                    {
                        row_data[3] = digit_to_led_data[st_mode_setting.custom_close_min%10];   //第四个数字(停几)
                        time_mark_display_control(row_data, 3);
                    }
                        
                    else                            //灭
                    {
                        row_data[3] = 0x00;
                        time_mark_display_control(row_data, 0);
                    }
                }
                
                week_mark_display_control(row_data, 0);
            }
        }
        
        NH3_mark_display_control(row_data, 0);
    }
    
    //显示工作仓液位
    if(5 == condition)
    {
        row_data[0] = 0x00;         //数码管第一个数字(灭)
        row_data[1] = 0x00;         //数码管第二个数字(灭)
        
        row_data[2] = 0x00;         //数码管第三个数字(灭)
        
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_OVERFLOW) == PULLDOWN)
            row_data[3] = digit_to_led_data[4];         //溢出液位, 显示 4
        else if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
            row_data[3] = digit_to_led_data[1];         //低液位, 显示 1
        else if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) == PULLDOWN)
            row_data[3] = digit_to_led_data[3];         //高液位, 显示 3
        else
            row_data[3] = digit_to_led_data[2];         //中液位, 显示 2

        
        NH3_mark_display_control(row_data, 0);
        time_mark_display_control(row_data, 0);
        week_mark_display_control(row_data, 0);
    }
    
    //显示工作仓液位
    if(6== condition)
    {
        row_data[0] = display_current_version(0);
        row_data[1] = display_current_version(1);
        row_data[2] = display_current_version(2);
        row_data[3] = display_current_version(3);
        
        BIT_CLEAR(row_data[1],6);       //H2
        BIT_CLEAR(row_data[2],6);       //H3
        
        NH3_mark_display_control(row_data, 0);
        time_mark_display_control(row_data, 0);
        week_mark_display_control(row_data, 0);
    }
}





#define DEBUG_COUNT 9
static int debug_count = 0;
void update_display_data(unsigned char * row_data)
{
    int flag = 0, i;
    
    //******** 调试打印 *********
    debug_count++;
    if(10 == debug_count)
        debug_count = 0;
    //******** 调试打印 *********
    
    //电源关
    if(OFF == air_clean_cfg.power_onoff)
    {
        for(i=0; i<11; i++) {
            row_data[i] = 0;
        }
    }
    else
    {
        //OK 
        //控制 "数码管/氨气标识/时间标识/周标识" 根据条件显示
        //*******************************************************************************
        
        //开机启动显示    (第一优先级)
        if(1 == st_current_status.start_up_flag)
        {
            condition_display_control(row_data, 6);
        }
        
        //错误码值显示
        //else if(get_err_code() != 0)
        else if( (get_err_code() != 0) && (0 == st_setting_status.setting_time_flag) )
        {
            condition_display_control(row_data, 0);
        }
        //#endif
        
        //其他情形显示
        else
        {
            //"设定时间"时的显示
            if(1 == st_setting_status.setting_time_flag)
            {
                //if(DEBUG_COUNT == debug_count)
                //    WS_DEBUG("condition_display_control --> setting_time ... \r\n");
                
                condition_display_control(row_data, 3);
            }
            //"设定模式"时的显示(需要选择好设定本地模式/自定义模式)
            else if((1 == st_setting_status.setting_mode_flag) && (1 == st_mode_setting.set_mode_type_flag))
            {
                //if(DEBUG_COUNT == debug_count)
                //    WS_DEBUG("condition_display_control --> setting_mode ... \r\n");
                
                condition_display_control(row_data, 4);
            }
            //平时显示
            else
            {
                if(SHOW_NH3 == st_current_status.current_number_show)
                {
                    //if(DEBUG_COUNT == debug_count)
                    //    WS_DEBUG("condition_display_control --> SHOW_NH3 ... \r\n");
                    
                    condition_display_control(row_data, 2);
                }
                else if(SHOW_TIME == st_current_status.current_number_show)
                {
                    //if(DEBUG_COUNT == debug_count)
                    //    WS_DEBUG("condition_display_control --> SHOW_TIME ... \r\n");
                    
                    condition_display_control(row_data, 1);
                }
                else if(SHOW_WORK_LIQUID_LEVEL == st_current_status.current_number_show)
                {
                    //if(DEBUG_COUNT == debug_count)
                    //    WS_DEBUG("condition_display_control --> SHOW_WORK_LIQUID_LEVEL ... \r\n");
                    
                    condition_display_control(row_data, 5);
                }
            }
        }
        
        //OK
        //植物液: A5 B5 C5 D5 E5 F5 G5 H5 A10 B10 C10
        //*******************************************************************************
        //植物液图标(正在加液时闪烁)
        if(1 == st_current_status.add_plant_liquid_flag)
        {
            if(0 == plant_icon_flick_flag)
                plant_icon_flick_flag = 1;
            else
                plant_icon_flick_flag = 0;
            
            if(0 == plant_icon_flick_flag)  //灭
                plant_icon_display_control(row_data, 0);
            else                            //亮
                plant_icon_display_control(row_data, 1);
        }
        else
        {
            //考虑情形: 正在清洗/一键排液的过程中, 原液箱放液(只能手动打开阀门放液), 需实时显示原液箱液位
            if((st_current_status.current_wash_status == ON) || (st_current_status.onekey_drain_status == ON))
            {
                //实时获取原液箱的液位
                if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) == PULLDOWN)
                {
                    st_current_status.current_plant_liquid = PLANT_LIQUID_LOW;
                }
                else if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_MID) == PULLDOWN)
                {
                    st_current_status.current_plant_liquid = PLANT_LIQUID_MID;
                }
                else if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_HIGH) == PULLDOWN)
                {
                    st_current_status.current_plant_liquid = PLANT_LIQUID_HIGH;
                }
            }
            plant_icon_display_control(row_data, 1);
        }
        //植物液字体(常亮)
        BIT_SET(row_data[4],7);             //G5
        BIT_SET(row_data[4],6);             //H5
        
        //OK
        //音量: D6 E6 F6 G6
        //*******************************************************************************
        if(0)    //设置音量 -- 常灭
        {
            //音量图标
            BIT_SET(row_data[5],3);         //D6
            BIT_SET(row_data[5],4);         //E6
            BIT_SET(row_data[5],5);         //F6
            //音量字体
            BIT_SET(row_data[5],7);         //G6
        }
        
        //OK
        //工作模式: A7 B7 C7 D7
        //*******************************************************************************
        //"正在设置工作模式"
        if(1 == st_setting_status.setting_mode_flag)
        {
            //"正在设置工作模式" 且 "模式类型还没选择好", 此时对应的模式类型闪烁
            if(0 == st_mode_setting.set_mode_type_flag)
            {
                //选择本地模式, 本地模式闪烁, 自定义(智能)模式灭
                if(1 == st_mode_setting.mode_type_select)
                {
                    if(0 == mode_icon_flick_flag) 
                        mode_icon_flick_flag = 1;
                    else
                        mode_icon_flick_flag = 0;
                    
                    if(1 == mode_icon_flick_flag)    //亮
                    {
                        BIT_SET(row_data[6],0);
                        BIT_SET(row_data[6],1);
                    }
                    else                            //灭
                    {
                        BIT_CLEAR(row_data[6],0);
                        BIT_CLEAR(row_data[6],1);
                    }
                    
                    BIT_CLEAR(row_data[6],2);
                    BIT_CLEAR(row_data[6],3);
                }
                //选择自定义(智能)模式, 自定义模式闪烁, 本地模式灭
                else if(2 == st_mode_setting.mode_type_select)
                {
                    if(0 == mode_icon_flick_flag) 
                        mode_icon_flick_flag = 1;
                    else
                        mode_icon_flick_flag = 0;
                    
                    BIT_CLEAR(row_data[6],0);
                    BIT_CLEAR(row_data[6],1);
                    
                    if(1 == mode_icon_flick_flag)    //亮
                    {
                        BIT_SET(row_data[6],2);
                        BIT_SET(row_data[6],3);
                    }
                    else                            //灭
                    {
                        BIT_CLEAR(row_data[6],2);
                        BIT_CLEAR(row_data[6],3);
                    }
                }
            }
            else
            {
                if(1 == st_mode_setting.mode_type_select)       //标准模式
                {
                    BIT_SET(row_data[6],0);
                    BIT_SET(row_data[6],1);
                    BIT_CLEAR(row_data[6],2);
                    BIT_CLEAR(row_data[6],3);
                }
                else                                            //自定义(智能)模式
                {
                    BIT_CLEAR(row_data[6],0);
                    BIT_CLEAR(row_data[6],1);
                    BIT_SET(row_data[6],2);
                    BIT_SET(row_data[6],3);
                }
            }
        }
        //"平时状态(未设置工作模式)"
        else
        {
            if(air_clean_cfg.work_mode != MODE_CUSTOM)      //标准模式
            {
                BIT_SET(row_data[6],0);
                BIT_SET(row_data[6],1);
                BIT_CLEAR(row_data[6],2);
                BIT_CLEAR(row_data[6],3);
            }
            else                                            //自定义(智能)模式
            {
                BIT_CLEAR(row_data[6],0);
                BIT_CLEAR(row_data[6],1);
                BIT_SET(row_data[6],2);
                BIT_SET(row_data[6],3);
            }
        }
        
        //OK 
        //雾化器: E7 F7 G7 H7 A8 B8
        //*******************************************************************************
        //雾化器图标
        //E7 F7 -- 代表雾化器是"自动切换"还是"锁定状态", 自动切换:亮; 锁定状态:灭
        if(1 == air_clean_cfg.atomizer_lock)    
        {
            BIT_CLEAR(row_data[6],4);       //E7
            BIT_CLEAR(row_data[6],5);       //F7
        }
        else                                    
        {
            BIT_SET(row_data[6],4);         //E7
            BIT_SET(row_data[6],5);         //F7
        }
        
        if(1 == is_running)
        {
            if(0 == atomizer_icon_flick_flag) 
                atomizer_icon_flick_flag = 1;
            else
                atomizer_icon_flick_flag = 0;
            
            if(1 == atomizer_icon_flick_flag)
            {
                if(air_clean_cfg.which_atomizer == ATOMIZER_ONE)
                {
                    BIT_SET(row_data[6],7);     //G7
                    BIT_CLEAR(row_data[6],6);   //H7
                }
                else if(air_clean_cfg.which_atomizer == ATOMIZER_TWO)
                {
                    BIT_CLEAR(row_data[6],7);   //G7
                    BIT_SET(row_data[6],6);     //H7
                }
            }
            else
            {
                 BIT_CLEAR(row_data[6],7);   //G7
                 BIT_CLEAR(row_data[6],6);   //H7
            }
        }
        else
        {
            if(air_clean_cfg.which_atomizer == ATOMIZER_ONE)
            {
                BIT_SET(row_data[6],7);     //G7
                BIT_CLEAR(row_data[6],6);   //H7
            }
            else if(air_clean_cfg.which_atomizer == ATOMIZER_TWO)
            {
                BIT_CLEAR(row_data[6],7);   //G7
                BIT_SET(row_data[6],6);     //H7
            }
        }
        //雾化器字体
        BIT_SET(row_data[7],0);
        BIT_SET(row_data[7],1);
        
        //OK 
        //风量: C8 D8 E8 F8
        //*******************************************************************************
        if((1 == is_running) || (1 == st_current_status.close_fan_flag))    //延时5秒关闭风扇也需要闪烁
        {
            if(0 == fan_icon_flick_flag) 
                fan_icon_flick_flag = 1;
            else
                fan_icon_flick_flag = 0;
            
            if(1 == fan_icon_flick_flag)
            {
                if(air_clean_cfg.air_fan_level == AIR_FAN_LEVEL_ONE){
                    BIT_CLEAR(row_data[7],2);   //C8
                    BIT_CLEAR(row_data[7],3);   //D8
                    BIT_SET(row_data[7],4);     //E8
                }else if(air_clean_cfg.air_fan_level == AIR_FAN_LEVEL_TWO){
                    BIT_CLEAR(row_data[7],2);   //C8
                    BIT_SET(row_data[7],3);     //D8
                    BIT_SET(row_data[7],4);     //E8
                }else if(air_clean_cfg.air_fan_level == AIR_FAN_LEVEL_THREE){
                    BIT_SET(row_data[7],2);     //C8
                    BIT_SET(row_data[7],3);     //D8
                    BIT_SET(row_data[7],4);     //E8
                }
            }
            else
            {
                BIT_CLEAR(row_data[7],2);   //C8
                BIT_CLEAR(row_data[7],3);   //D8
                BIT_CLEAR(row_data[7],4);     //E8
            }
        }
        else
        {
            if(air_clean_cfg.air_fan_level == AIR_FAN_LEVEL_ONE){
                BIT_CLEAR(row_data[7],2);   //C8
                BIT_CLEAR(row_data[7],3);   //D8
                BIT_SET(row_data[7],4);     //E8
            }else if(air_clean_cfg.air_fan_level == AIR_FAN_LEVEL_TWO){
                BIT_CLEAR(row_data[7],2);   //C8
                BIT_SET(row_data[7],3);     //D8
                BIT_SET(row_data[7],4);     //E8
            }else if(air_clean_cfg.air_fan_level == AIR_FAN_LEVEL_THREE){
                BIT_SET(row_data[7],2);     //C8
                BIT_SET(row_data[7],3);     //D8
                BIT_SET(row_data[7],4);     //E8
            }
        }
        
        BIT_SET(row_data[7],5);         //F8 : 风量字体常亮
        
        //OK 
        //浓度: G8 H8 A9 B9 C9 (植物液浓度)
        //浓度字体/浓度最上面的图标(G8) 常亮
        //*******************************************************************************
        #if MANUAL_EDITION      //手动版本, 浓度标识长灭
            BIT_CLEAR(row_data[7],7);   //G8
            BIT_CLEAR(row_data[7],6);   //H8
            BIT_CLEAR(row_data[8],0);   //A9
            BIT_CLEAR(row_data[8],1);   //B9
            BIT_CLEAR(row_data[8],2);   //C9
        #else
            BIT_SET(row_data[7],7);         //G8
            if(air_clean_cfg.liquid_density == LIQUID_DENSITY_ONE)          //浓度最小
            {
                BIT_CLEAR(row_data[7],6);   //H8
                BIT_CLEAR(row_data[8],0);   //A9
                BIT_SET(row_data[8],1);     //B9
            }
            else if(air_clean_cfg.liquid_density == LIQUID_DENSITY_TWO)     //浓度中等
            {
                BIT_CLEAR(row_data[7],6);   //H8
                BIT_SET(row_data[8],0);     //A9
                BIT_SET(row_data[8],1);     //B9
            }
            else if(air_clean_cfg.liquid_density == LIQUID_DENSITY_THREE)   //浓度最大
            {
                BIT_SET(row_data[7],6);     //H8
                BIT_SET(row_data[8],0);     //A9
                BIT_SET(row_data[8],1);     //B9
            }
            BIT_SET(row_data[8],2);         //C9 : 浓度字体常亮
        #endif
        
        //OK 
        //wash_mode: 0 自动清洗  手动清洗 
        //清洗: D9 C11 E9 D11
        //*******************************************************************************
        //正在清洗 或者 正在一键排液：清洗灯白色闪烁
        if((st_current_status.current_wash_status == ON) || (st_current_status.onekey_drain_status == ON) ||
                (ON == st_current_status.power_off_e7_maintance_status) || (ON == st_current_status.overflow_drain_liquid_status) )     
        {
            //正在清洗, 清洗灯亮白色闪烁
            if(0 == wash_icon_flick_flag) 
                wash_icon_flick_flag = 1;
            else
                wash_icon_flick_flag = 0;
            
            if(1 == wash_icon_flick_flag)
            {
                //清洗图标
                BIT_SET(row_data[8],3);         //D9
                BIT_CLEAR(row_data[10],2);      //C11
                //清洗字体
                BIT_SET(row_data[8],4);         //E9
                BIT_CLEAR(row_data[10],3);      //D11
            }
            else
            {
                BIT_CLEAR(row_data[8],3);       //D9
                BIT_CLEAR(row_data[10],2);      //C11
                BIT_CLEAR(row_data[8],4);       //E9
                BIT_CLEAR(row_data[10],3);      //D11
            }
        }
        else                                                //非正在清洗
        {
            /**
             * 非正在清洗情形: 
             *      1.手动清洗模式, 超时未清洗; 
             *      2.自动清洗模式, 清洗完成24小时以内; 
             *      3.自动清洗模式, 清洗完成24小时以后或者一直未清洗
             */
            if(st_current_status.manual_wash_stopdev == 1)
            {
                //manual_wash_stopdev == 1 只有在手动清洗模式下
                /*
                if(air_clean_cfg.wash_mode == AUTO_WASH_MODE)
                {
                    BIT_CLEAR(row_data[8],3);       //D9
                    BIT_CLEAR(row_data[10],2);      //C11
                    BIT_CLEAR(row_data[8],4);       //E9
                    BIT_CLEAR(row_data[10],3);      //D11
                }
                */
                //else if(air_clean_cfg.wash_mode == MANUAL_WASH_MODE)    //手动清洗模式, 超时未清洗, 常亮红灯
                //{
                    //自动清洗: 显示红色
                    BIT_CLEAR(row_data[8],3);       //D9
                    BIT_SET(row_data[10],2);        //C11
                    BIT_CLEAR(row_data[8],4);       //E9
                    BIT_SET(row_data[10],3);        //D11
                //}
            }
            else
            {
                if(1 == air_clean_cfg.onekey_drain_fished)     //一键排液功能执行完以后, 清洗灯常亮粉色
                {
                    //清洗图标
                    BIT_SET(row_data[8],3);         //D9
                    BIT_SET(row_data[10],2);      //C11
                    //清洗字体
                    BIT_SET(row_data[8],4);         //E9
                    BIT_SET(row_data[10],3);      //D11
                }
                else if(air_clean_cfg.wash_mode == AUTO_WASH_MODE)           //自动清洗模式, 清洗灯灭
                {
                    BIT_CLEAR(row_data[8],3);       //D9
                    BIT_CLEAR(row_data[10],2);      //C11
                    BIT_CLEAR(row_data[8],4);       //E9
                    BIT_CLEAR(row_data[10],3);      //D11
                }
                else if(air_clean_cfg.wash_mode == MANUAL_WASH_MODE)         //手动清洗模式, 清洗灯常亮白灯 (D9 E9 亮)
                {
                    //清洗图标
                    BIT_SET(row_data[8],3);         //D9
                    BIT_CLEAR(row_data[10],2);      //C11
                    //清洗字体
                    BIT_SET(row_data[8],4);         //E9
                    BIT_CLEAR(row_data[10],3);      //D11
                }
            }
        }
    }
    
    //OK 
    //网络: F9 E11 G9 F11 
    //正在配网状态 -- 白色闪烁
    //未连接网络且未连接后台 -- 灯灭
    //连接网络, 未连接到后台 -- 红色常亮
    //连接网络, 且连接到后台 -- 白色常亮
    //*******************************************************************************
    #if NETWORK_VERSION
        if(st_current_status.net_trigger_flag == 1)         //正在配网状态
        {
            if(0 == net_icon_flick_flag) 
                net_icon_flick_flag = 1;
            else
                net_icon_flick_flag = 0;
            
            if(1 == net_icon_flick_flag)        //网络图标/字体闪烁
            {
                //网络图标(显示白色)
                BIT_SET(row_data[8],5);         //F9
                BIT_CLEAR(row_data[10],4);      //E11
                //网络字体(显示白色)
                BIT_SET(row_data[8],7);         //G9
                BIT_CLEAR(row_data[10],5);      //F11
            }
            else
            {
                BIT_CLEAR(row_data[8],5);       //F9
                BIT_CLEAR(row_data[10],4);      //E11
                BIT_CLEAR(row_data[8],7);       //G9
                BIT_CLEAR(row_data[10],5);      //F11
            }
        }
        else if(g_current_net_status == 0)  //连接路由器(网络)失败
        {
            //网络图标/字体闪烁
            BIT_CLEAR(row_data[8],5);           //F9
            BIT_CLEAR(row_data[10],4);          //E11
            BIT_CLEAR(row_data[8],7);           //G9
            BIT_CLEAR(row_data[10],5);          //F11
        }
        else if(g_current_net_status == 1)  //连接路由器(网络)成功, 连接后台失败 
        {
            //网络图标/字体红色
            //网络图标(显示白色)
            BIT_CLEAR(row_data[8],5);           //F9
            BIT_SET(row_data[10],4);            //E11
            //网络字体(显示白色)
            BIT_CLEAR(row_data[8],7);           //G9
            BIT_SET(row_data[10],5);            //F11
        }
        else if(g_current_net_status == 2)  //连接路由器(网络)成功, 连接后台成功
        {
            //网络图标(显示白色)
            BIT_SET(row_data[8],5);         //F9
            BIT_CLEAR(row_data[10],4);      //E11
            //网络字体(显示白色)
            BIT_SET(row_data[8],7);         //G9
            BIT_CLEAR(row_data[10],5);      //F11
        }
    #else
        BIT_CLEAR(row_data[8],5);       //F9
        BIT_CLEAR(row_data[10],4);      //E11
        BIT_CLEAR(row_data[8],7);       //G9
        BIT_CLEAR(row_data[10],5);      //F11
    #endif
    
    //OK 
    //电源: G11 H11 
    //设备上电后, 电源图标/电源字体 常亮
    //*******************************************************************************
    //电源图标(常亮)
    BIT_SET(row_data[10],7);        //G11
    //电源字体(常亮)
    BIT_SET(row_data[10],6);        //H11
}


/*
*********************************************************************************************************
*                                           CYCLE CHECK/UPDATE TASK
*********************************************************************************************************
*/
typedef struct
{  
    int iYear;
    int iMonth;
    int iDay;
}Date;

/******************************************************************************
 * FunctionName : IsLeap
 * Description  : 函数判断一个年份是否为闰年
 * Parameters   : year(年份)
 * Returns      : bool(true:闰年; false:非闰年)
*******************************************************************************/
bool IsLeap(int year)
{
    return (year % 4 ==0 || year % 400 ==0) && (year % 100 !=0);
}

/******************************************************************************
 * FunctionName : DayInYear
 * Description  : 根据给定的日期, 求出在该年的第几天
 * Parameters   : pDate(日期)
 * Returns      : int(该年的第几天)
*******************************************************************************/
int DayInYear(Date* pDate)
{  
    int i;
    int iRet = 0;
    
    int DAY[12]={31,28,31,30,31,30,31,31,30,31,30,31};
    if(IsLeap(pDate->iYear))
    {
        DAY[1] = 29;
    }
    
    for(i=0; i < pDate->iMonth - 1; ++i)
    {
        iRet += DAY[i];
    }
    iRet += pDate->iDay;
    
    return iRet;
}

/******************************************************************************
 * FunctionName : DaysBetween2Date
 * Description  : 根据给定的两个日期, 求出这两个日期的间隔天数
 * Parameters   : pDate1(日期1); pDate2(日期2)
 * Returns      : int(两个日期间隔天数)
*******************************************************************************/
int DaysBetween2Date(Date* pDate1, Date* pDate2)
{
    //取出日期中的年月日
    Date *pTmp;
    int year;
    int d1,d2,d3;
    
    //年份月份都相同
    if(pDate1->iYear == pDate2->iYear && pDate1->iMonth == pDate2->iMonth)
    {
        return abs(pDate1->iDay - pDate2->iDay);
    }
    //仅年份相同
    else if(pDate1->iYear == pDate2->iYear)
    {
        return abs(DayInYear(pDate1) - DayInYear(pDate2));
    }
    //年份月份都不同
    else
    {
        //确保pDate1->year1年份比pDate2->year2早
        if(pDate1->iYear > pDate2->iYear)
        {
            pTmp = pDate1;
            pDate1 = pDate2;
            pDate1 = pTmp;
        }
        
        if(IsLeap(pDate1->iYear))
            d1 = 366 - DayInYear(pDate1);   //取得这个日期在该年还于下多少天
        else
            d1 = 365 - DayInYear(pDate1);
        
        d2 = DayInYear(pDate2);             //取得在当年中的第几天
        
        d3 = 0;
        for(year = pDate1->iYear + 1; year < pDate2->iYear; year++)
        {  
            if(IsLeap(year))
                d3 += 366;
            else
                d3 += 365;
        }
        
        return d1 + d2 + d3;
    }
}

/******************************************************************************
 * FunctionName : day_diff
 * Description  : 根据给定的两个日期, 求出这两个日期的间隔天数
 * Parameters   : 起始年月日; 截至年月日
 * Returns      : int(起始与截至日期的间隔天数)
*******************************************************************************/
int day_diff(int year_start, int month_start, int day_start, 
        int year_end, int month_end, int day_end)
{
    int days;
    Date pDate1;
    Date pDate2;
    
    pDate1.iYear = 2000 + year_start;
    pDate1.iMonth = month_start;
    pDate1.iDay = day_start;
    
    pDate2.iYear = 2000 + year_end;
    pDate2.iMonth = month_end;
    pDate2.iDay = day_end;
    
    days = DaysBetween2Date(&pDate1, &pDate2);
    return days;
}

/******************************************************************************
 * FunctionName : get_interval_days
 * Description  : 根据间隔类型, 获取相应的间隔天数
 * Parameters   : interval_type(间隔类型)
 *                  ATOMIZER_INTERVAL_TYPE: 雾化器间隔时间
 *                  WASH_INTERVAL_TYPE:     清洗间隔时间
 *                  E7_CLOSE_INTERVAL_TYPE: 出现E7或关机间隔时间
 * Returns      : int(相应的间隔天数)
*******************************************************************************/
int get_interval_days(int interval_type)
{
    int interval_days = 0;
    
    int year_start;
    int month_start;
    int day_start;

    //get_show_time();      //屏幕刷新线程一直在更新时间数据
    int year_end = calendar.w_year;
    int month_end = calendar.w_month;
    int day_end = calendar.w_date;
    
    //WS_DEBUG("[get_interval_days] year_end = %d; month_end = %d, day_end = %d \r\n", year_end, month_end, day_end);
    
    switch(interval_type)
    {
        //1.雾化器间隔时间
        case ATOMIZER_INTERVAL_TYPE:
            year_start = air_clean_cfg.atomizer_start_time.start_year;
            month_start = air_clean_cfg.atomizer_start_time.start_month;
            day_start = air_clean_cfg.atomizer_start_time.start_day;
            #if TEST_AUTO_WASH
            //WS_DEBUG("[get_interval_days:ATOMIZER_INTERVAL_TYPE] year_start = %d; month_start = %d, day_start = %d \r\n", year_start, month_start, day_start);
            #endif
            interval_days = day_diff(year_start, month_start, day_start, year_end, month_end, day_end);
        break;
        
        //2.清洗间隔时间
        case WASH_INTERVAL_TYPE:
            year_start = air_clean_cfg.dev_work_time.start_year;
            month_start = air_clean_cfg.dev_work_time.start_month;
            day_start = air_clean_cfg.dev_work_time.start_day;
            #if TEST_AUTO_WASH
            //WS_DEBUG("[get_interval_days:WASH_INTERVAL_TYPE] year_start = %d; month_start = %d, day_start = %d \r\n", year_start, month_start, day_start);
            #endif
            interval_days = day_diff(year_start, month_start, day_start, year_end, month_end, day_end);
        break;
        
        //3.出现E7或关机时长(注水+排液)
        case E7_CLOSE_INTERVAL_TYPE:
            year_start = st_current_status.power_off_e7_time.start_year;
            month_start = st_current_status.power_off_e7_time.start_month;
            day_start = st_current_status.power_off_e7_time.start_day;
            if( (0 == year_start) && (0 == month_start) && (0 == day_start))
            {
                interval_days = 0;
            }
            else
            {
                interval_days = day_diff(year_start, month_start, day_start, year_end, month_end, day_end);
            }
        break;
        
        default:
            break;
    }
    
    return interval_days;
}


/******************************************************************************
 * FunctionName : e7_maintain_drain_liquid
 * Description  : 关机或E7状态下, 保养(清洗)过程中注水步骤
 * Parameters   : None
 * Returns      : 0排液正常退出; 排液非正常退出
*******************************************************************************/
int e7_maintain_supply_water()
{
    OS_ERR err;

    char timeout = 0;               //注水超时时间
    
    WS_DEBUG("+++++++++ [e7_maintain_supply_water] 开始执行 e7_maintain_supply_water 函数 \r\n");
    
    solenoid_onoff(SOLENOID_INTO_WORK, ON);
    while(1)
    {
        
        
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) == PULLDOWN)
        {
            break;
        }
        
        //电源键关闭 / 清洗标识被清除, 跳出
        //if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.power_off_e7_maintance_status))
        if(OFF == st_current_status.power_off_e7_maintance_status)
        {
            WS_DEBUG("[e7_maintain_supply_water] 清洗保养可能遇到溢出液位导致退出 return  \r\n");
            solenoid_onoff(SOLENOID_INTO_WORK, OFF);
            return 1;
        }
        
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
        
        //注水启动，150S内未到达最高液位点，出现E3
        timeout ++;
        if(timeout >= SUPPLY_WATER_TIMEOUT)
        {
            WS_DEBUG("+++++++++ [e7_maintain_supply_water] timeout = %d \r\n", timeout);
            solenoid_onoff(SOLENOID_INTO_WORK, OFF);
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) != PULLDOWN)     //工作仓未到达最高液位点
            {
                WS_DEBUG("+++++++++ [e7_maintain_supply_water] 清洗保养过程中出现注水故障，设置 ERR_WORK_SUPPLY_WATER 故障码\r\n");
                BIT_SET(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER);
                
                return -1;
            }
            return 0;
        }
    }
    solenoid_onoff(SOLENOID_INTO_WORK, OFF);
    
    return 0;
}


/******************************************************************************
 * FunctionName : e7_maintain_drain_liquid
 * Description  : 关机或E7状态下, 保养(清洗)过程中排液步骤
 * Parameters   : None
 * Returns      : 0排液正常退出; 排液非正常退出
*******************************************************************************/
int e7_maintain_drain_liquid()
{
    OS_ERR err;
    
    int out_time = 0;       //排液保护时间
    int delay_count = 0;    //延时计数
    
    char err_drain_liquid_flag = 0;     //第1次出现排液故障标识
    
    //正在执行清洗排液
    //st_current_status.wash_drain_liquid_status = ON;
    
    WS_DEBUG("+++++++++ [e7_maintain_drain_liquid] 开始执行 e7_maintain_drain_liquid 函数 \r\n");
    
    solenoid_onoff(SOLENOID_OUT_WORK, ON);
    while(1)
    {
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
        {
            break;
        }
        
        //排液停止情形: 
        //  清洗与一键排液功能关闭: 断电或者关闭电源键可取消
        //电源关闭 跳出
        //if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.power_off_e7_maintance_status))
        if(OFF == st_current_status.power_off_e7_maintance_status)
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            WS_DEBUG("[e7_maintain_drain_liquid] 001 清洗保养可能遇到溢出液位导致退出 return  \r\n");
            return 1;
        }

        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
        out_time ++;
        //当启动清洗程序 ** 秒后液位传感器未感应到最低液位点, 设备停止工作, 报E4排液故障
        if(out_time >= DRAIN_LIQUID_SAFE_TIME)
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            
            WS_DEBUG("[e7_maintain_drain_liquid] E4排液故障 第一次 \r\n");
            err_drain_liquid_flag = 1;
            
            break;
        }
    }
    
    WS_DEBUG("[e7_maintain_drain_liquid] flag 000001 \r\n");
    //第1次出现排液故障，报E4报警，继续第2次排液
    if(1 == err_drain_liquid_flag)
    {
        out_time = 0;
        solenoid_onoff(SOLENOID_OUT_WORK, ON);
        while(1)
        {
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) == PULLDOWN)
            {
                //若第2次排液正常，则消除E4
                //BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID);
                break;
            }
            
            //排液停止情形: 1.电源关闭; 2.清洗关闭; 3.一键排液关闭
            //电源关闭 跳出
            //if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.power_off_e7_maintance_status))
            if(OFF == st_current_status.power_off_e7_maintance_status)
            {
                solenoid_onoff(SOLENOID_OUT_WORK, OFF);
                WS_DEBUG("[e7_maintain_drain_liquid] 002 清洗保养可能遇到溢出液位导致退出 return  \r\n");
                return 1;
            }

            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
            out_time ++;
            //当启动清洗程序 ** 秒后液位传感器未感应到最低液位点, 设备停止工作, 报E4排液故障
            if(out_time >= DRAIN_LIQUID_SAFE_TIME)
            {
                solenoid_onoff(SOLENOID_OUT_WORK, OFF);
                //设置E4排液故障
                BIT_SET(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID);
                WS_DEBUG("[e7_maintain_drain_liquid] 保养(清洗)过程中出现排液故障，设置ERR_WORK_DRAIN_LIQUID故障码 \r\n");
                
                
                WS_DEBUG("[e7_maintain_drain_liquid] E4排液故障 第二次 return \r\n");
                
                return -1;
            }
        }
    }
    
    WS_DEBUG("[e7_maintain_drain_liquid] flag 000002 \r\n");

    //延时 DRAIN_LIQUID_DELAY_TIME 秒, 关闭排水电磁阀
    while(1)
    {
        //电源键关闭/清洗键关闭, 跳出
        //if((OFF == air_clean_cfg.power_onoff) || (OFF == st_current_status.power_off_e7_maintance_status))
        if(OFF == st_current_status.power_off_e7_maintance_status)
        {
            solenoid_onoff(SOLENOID_OUT_WORK, OFF);
            WS_DEBUG("[e7_maintain_drain_liquid] 003 清洗保养可能遇到溢出液位导致退出 return  \r\n");
            return 1;
        }
        
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); 
        delay_count++;
        if(delay_count >= DRAIN_LIQUID_DELAY_TIME)
        {
            WS_DEBUG("[e7_maintain_drain_liquid] 延时 DRAIN_LIQUID_DELAY_TIME 秒关闭排水电磁阀 \r\n");
            break;
        }
    }
    
    solenoid_onoff(SOLENOID_OUT_WORK, OFF);
    WS_DEBUG("[e7_maintain_drain_liquid] 正常退出 e7_maintain_drain_liquid 函数 \r\n");
    
    return 0;
}

/******************************************************************************
 * FunctionName : power_off_e7_maintance
 * Description  : 关机或E7状态下, 每隔三天执行保养(清洗)
 *                保养清洗: 注水-->排液-->注水
 *                    注水到高位开始排液, 排液到低点保持40S, 然后注满水到高位
 * Parameters   : None
 * Returns      : None
*******************************************************************************/
void power_off_e7_maintance()
{
    WS_DEBUG("[power_off_e7_maintance] 保养清洗：直接执行 e7_maintain_supply_water 注水到高液位即可 \r\n");
    //WS_DEBUG("[power_off_e7_maintance] 保养清洗第一步：执行 e7_maintain_supply_water 补水函数 \r\n");
    if(e7_maintain_supply_water() != 0)
    {
        st_current_status.power_off_e7_maintance_status = OFF; 
        return ;
    }
    
    /*
    WS_DEBUG("[power_off_e7_maintance] 保养清洗第二步：执行 e7_maintain_drain_liquid 排液函数 \r\n");
    if(e7_maintain_drain_liquid() != 0)
    {
        st_current_status.power_off_e7_maintance_status = OFF; 
        return ;
    }
    
    WS_DEBUG("[power_off_e7_maintance] 保养清洗第三步：执行 e7_maintain_supply_water 补水函数 \r\n");
    if(e7_maintain_supply_water() != 0)
    {
        st_current_status.power_off_e7_maintance_status = OFF; 
        return ;
    }
    */
}


/*
*********************************************************************************************************
*                                           相关逻辑备注
*********************************************************************************************************
*/
/*
    1.工作周期：周一至周日，工作时段：07:00至18:00，启停频率：开5停8
    2.工作周期：周一至周六，工作时段：07:00至18:00，启停频率：开5停8
    3.工作周期：周一至周五，工作时段：07:00至18:00，启停频率：开5停8
    4.工作周期：周一至周日，工作时段：00:00至23:59，启停频率：开5停8
*/
