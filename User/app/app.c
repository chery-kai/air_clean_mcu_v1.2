#include  "app.h"

/*
*********************************************************************************************************
*                                           全局变量定义
*********************************************************************************************************
*/
char PRODUCT_TYPE;                          //产品类型(开机显示依据)

char is_running = 0;                        //运行状态
char g_current_net_status;                  //网络状态
air_clean_cfg_st air_clean_cfg;             //配置保存
current_status_st st_current_status;        //当前状态

time_setting_st st_time_setting;            //时间设置结构体
mode_setting_st st_mode_setting;            //模式设置结构体
setting_status_st st_setting_status;        //设置时间/模式状态


char global_fan_one_clear_flag = 0;         //风扇1故障清除标识
char global_fan_two_clear_flag = 0;         //风扇2故障清除标识


/*
*********************************************************************************************************
*                                           init_air_clean(AppTaskStart调用)
*********************************************************************************************************
*/
//错误码需求: 只有E7和E10断电后不消失, 其他断电后消失
void power_on_clear_err_code()
{
    //清除 E1
    if(BIT_GET(air_clean_cfg.err_code_bitmap,ERR_FAN_ONE_FAULT))
        BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_FAN_ONE_FAULT);
    
    //清除 E2
    if(BIT_GET(air_clean_cfg.err_code_bitmap,ERR_FAN_TWO_FAULT))
        BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_FAN_TWO_FAULT);
    
    //清除 E3
    if(BIT_GET(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER))
        BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER);
    
    //清除 E4
    if(BIT_GET(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID))
        BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID);
    
    //清除 E5
    if(BIT_GET(air_clean_cfg.err_code_bitmap,ERR_ORG_ADD_LIQUID))
        BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_ORG_ADD_LIQUID);
    
    //清除 E11
    if(BIT_GET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_FIRST))
        BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_FIRST);
    
    //清除 E12
    if(BIT_GET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_MANY_TIMES))
        BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_MANY_TIMES);
}


//初始化相关配置
int init_air_clean()
{
    WS_DEBUG("[init_air_clean] 开始执行init_air_clean(配置数据初始化)函数 \r\n");
    
    //如果配置数据不存在, 写初始值到Flash 
    if(!configure_data_exists())
    {
        WS_DEBUG("[init_air_clean] 检测到Flash中没有配置数据, 往Flash中写入 \r\n");
        
        cfg_data_init();
        memset(&st_current_status, 0, sizeof(st_current_status));
    }
    //如果配置数据存在, 从Flash读取配置值
    else
    {
        WS_DEBUG("[init_air_clean] 检测到Flash中存在配置数据, 从Flash中读取 \r\n");
        
        read_cfg_from_flash();
        //需求: 断电上电后E7和E10断电后不消失
        power_on_clear_err_code();
        //air_clean_cfg.err_code_bitmap = 0;
    }
    
    //对结构体 current_status_st 相关变量进行赋初值
    memset(&st_current_status, 0, sizeof(st_current_status));
    
    get_show_time();            //先获取时间, 否则 get_whole_dev_onoff 中无法比对时间
    get_whole_dev_onoff();      //确定 work_plan_flag 的值
    
    //"产品类型"(PRODUCT_TYPE)初始化 
    #if NETWORK_VERSION
        PRODUCT_TYPE = AUTO_EDUCATE_8L;
    #else
        PRODUCT_TYPE = AUTO_ALONE_8L;
    #endif
    
    WS_DEBUG("[init_air_clean] 执行完毕init_air_clean(配置数据初始化)函数 \r\n");
}


/*
*********************************************************************************************************
*                                           AIR CLEAN TASK
*********************************************************************************************************
*/
//检查设备是否可以工作
char check_if_dev_can_work()
{
    //出现以下错误码，设备停止工作
    //错误码为 ORG_MUST_ADD_PLANT 时, 设备也要求不工作
    if( (BIT_GET(air_clean_cfg.err_code_bitmap,ERR_FAN_ONE_FAULT)) ||
            (BIT_GET(air_clean_cfg.err_code_bitmap,ERR_FAN_TWO_FAULT)) ||
            (BIT_GET(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER)) ||
            (BIT_GET(air_clean_cfg.err_code_bitmap,ERR_WORK_DRAIN_LIQUID)) ||
            (BIT_GET(air_clean_cfg.err_code_bitmap,ERR_ORG_ADD_LIQUID)) ||
            (BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT)) ||
            (BIT_GET(air_clean_cfg.err_code_bitmap,FORCE_WASH_STOP_DEV)) )
    {
        return 0;
    }
    
    return 1;
}

//更新植物液液位状态(修复 bug001)
void update_plant_level()
{
    if(PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW))
    {
        st_current_status.current_plant_liquid = PLANT_LIQUID_LOW;
        if(0 == air_clean_cfg.org_low_to_work_count)
        {
            if(BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT) != 1)
            {
                BIT_SET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT);      //设置必须加液的错误码 E7
                write_cfg_to_flash();   //E7 需记录 Flash
            }
        }
    }
    //原液箱中液位时更新状态
    else if(PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_MID))
    {
        st_current_status.current_plant_liquid = PLANT_LIQUID_MID;
        //清除必须加液的错误码 E7
        BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);
        //更新"原液箱到低液位后还能抽液到工作仓的次数"
        update_org_low_to_work_count();
    }
    //原液箱高液位时更新状态
    else if(PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_HIGH))
    {
        st_current_status.current_plant_liquid = PLANT_LIQUID_HIGH;
        //清除必须加液的错误码 E7
        BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);
        //更新"原液箱到低液位后还能抽液到工作仓的次数"
        update_org_low_to_work_count();
    }
}



void AirCleanTask(void *p_arg)
{
    OS_ERR err;
    
    char upload_count = 0;          //网络版上传错误信息时间计数
    char beep_alarm_finish = 0;     //蜂鸣器报警完成标识
    char beep_alarm_count = 0;      //蜂鸣器报警计数
    
    int ret;
    int stop_flag = 0;              //停止工作标识
    int loop_times = 0;             //循环计数
    
    
    char timeout;                   //注水超时计数
    char timeout_flag = 0;          //超时标志
    
    int pump2_time = 0;             //液泵2抽液时间
    
    char sensor_work_low_count = 0; //记录工作仓低液位的次数
    
    
    char peibi_supply_water_flag;
    
    
    (void)p_arg;
    
    WS_DEBUG("[AirCleanTask 主循环即将开始] 创建 AirCleanTask 成功... \r\n");
    
    //上电后检测一次植物液的液位
    update_plant_level();
    
    while(DEF_TRUE) 
    {
        WS_DEBUG("\r\n[@@@@@ AirCleanTask 主循环中 001 @@@@@] 需求：关机或E7状态下，每隔3天执行一次保养清洗 \r\n");
        /**********************************************************************
         * 需求：关机或E7状态下，每隔3天执行一次保养清洗
         * 1.根据开关机及E7状态，更新保养清洗的执行时间
         * 2.符合保养清洗条件时, 执行保养清洗
         *********************************************************************/
        //Step 1. 根据开关机及E7状态，更新保养清洗的执行时间
        if(ON == air_clean_cfg.power_onoff)     //开机
        {
            //开机 E7 状态
            if(BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT))  
            {
                if( (0 == st_current_status.power_off_e7_time.start_year) &&
                        (0 == st_current_status.power_off_e7_time.start_month) &&
                        (0 == st_current_status.power_off_e7_time.start_day) )
                {
                    WS_DEBUG("[AirCleanTask 主循环中 001] 开机 E7 状态, 更新保养时间 \r\n");
                    st_current_status.power_off_e7_time.start_year = calendar.w_year;
                    st_current_status.power_off_e7_time.start_month = calendar.w_month;
                    st_current_status.power_off_e7_time.start_day = calendar.w_date;
                }
            }
            //开机非 E7 状态, 清空 st_current_status.power_off_e7_time
            else
            {
                WS_DEBUG("[AirCleanTask 主循环中 001] 开机非 E7 状态, 清除保养时间 \r\n");
                st_current_status.power_off_e7_time.start_year = 0;
                st_current_status.power_off_e7_time.start_month = 0;
                st_current_status.power_off_e7_time.start_day = 0;
            }
        }
        else    //记录关机的时间, 用于保养清洗
        {
            if( (0 == st_current_status.power_off_e7_time.start_year) &&
                    (0 == st_current_status.power_off_e7_time.start_month) &&
                    (0 == st_current_status.power_off_e7_time.start_day) )
            {
                WS_DEBUG("[AirCleanTask 主循环中 001] 电源关闭, 更新保养时间 \r\n");
                st_current_status.power_off_e7_time.start_year = calendar.w_year;
                st_current_status.power_off_e7_time.start_month = calendar.w_month;
                st_current_status.power_off_e7_time.start_day = calendar.w_date;
            }
        }
        //Step 2. 符合保养清洗条件时, 执行保养清洗
        if( (ON == st_current_status.power_off_e7_maintance_status) &&
                (OFF == st_current_status.onekey_drain_status) &&
                (OFF == st_current_status.current_wash_status) &&
                (OFF == st_current_status.add_plant_liquid_flag) && 
                (OFF == st_current_status.poweron_error_code_flag) &&
                (OFF == st_current_status.overflow_drain_liquid_status) )
        {
            WS_DEBUG("[AirCleanTask 主循环中 001] 符合保养清洗条件时, 执行保养清洗 \r\n");
            power_off_e7_maintance();
            st_current_status.power_off_e7_maintance_status = OFF;
            
            st_current_status.power_off_e7_time.start_year = 0;
            st_current_status.power_off_e7_time.start_month = 0;
            st_current_status.power_off_e7_time.start_day = 0;
        }
        
        
        WS_DEBUG("\r\n[@@@@@ AirCleanTask 主循环中 002 @@@@@] 需求：出现相应错误码，所有设备停止工作，等待消除错误码 \r\n");
        /**********************************************************************
         * 需求：当出现相应错误码时, 所有设备停止工作, 等待消除错误码
         * 1.相应错误码导致设备不工作不包含以下错误码：
         *          ORG_MUST_ADD_PLANT, FORCE_WASH_STOP_DEV, ERR_OVERFLOW_FIRST
         *          //原液箱必须加液,强制清洗导致设备停止工作,第一次检测到工作仓溢出
         * 2.出现错误码导致设备不工作, 只有长按"确认"键消除错误码才能正常工作
         *********************************************************************/
        if( (ON == air_clean_cfg.power_onoff) && (0 != air_clean_cfg.err_code_bitmap) && \
                (get_err_num()!= ORG_MUST_ADD_PLANT) && (get_err_num()!= FORCE_WASH_STOP_DEV) &&
                (get_err_num()!= ERR_OVERFLOW_FIRST) )
        {
            WS_DEBUG("[AirCleanTask 主循环中 002] 出现错误码, 设备已经停止工作, 错误码 = %d \r\n", get_err_num());
            
            //1.检测到错误码导致设备不工作, 首先需设置以下标志参数
            stop_flag = 0;
            loop_times = 0;
            st_current_status.one_key_atomize_flag = 0;
            //开机状态下出现错误码标识(此时设备不工作), 用于溢出液位检测任务中
            st_current_status.poweron_error_code_flag = 1;
            
            dev_stop_work();
            
            //2.出现错误码, 蜂鸣每隔2秒滴一声, 3分钟后停止
            if(0 == beep_alarm_finish)
            {
                macBEEP_ON();
                OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err);
                macBEEP_OFF();
                
                beep_alarm_count ++;
                if(90 == beep_alarm_count)      //蜂鸣器响 3 分钟 (3*60/2=90)
                {
                    beep_alarm_count = 0;
                    beep_alarm_finish = 1;
                }
            }
            
            OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时2秒 
            
            #if NETWORK_VERSION
            //3.网络版本需每隔 30 秒上传一次错误信息
            upload_count ++;
            if(upload_count >= 15)
            {
                upload_count = 0;
                UploadFaultInfo();
            }
            #endif
            
            continue;
        }
        else
        {
            st_current_status.poweron_error_code_flag = 0;
            //错误码消除，清除蜂鸣器计数及蜂鸣器报警完成标识
            if(1 == beep_alarm_finish)
            {
                beep_alarm_count = 0;
                beep_alarm_finish = 0;
            }
            
            //正在执行溢出放液过程中，该任务停止往下执行
            if(ON == st_current_status.overflow_drain_liquid_status)
            {
                WS_DEBUG("[AirCleanTask 主循环中 002] 正在执行溢出排液操作，AirCleanTask停止往下执行 \r\n");
                OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时2秒 
                continue;
            }
        }
        
        
        WS_DEBUG("\r\n[@@@@@ AirCleanTask 主循环中 003 @@@@@] 需求：手动清洗到达强制清洗时间或一键排液完成后, 设备停止工作 \r\n");
        /**********************************************************************
         * 需求：手动清洗到达强制清洗时间或一键排液完成后, 设备停止工作
         *      1.手动清洗模式下, 达到清洗时间还未清洗, 设备停止工作
         *      2.一键排液执行完成正常退出后, 设备停止工作
         * 逻辑注明：
         *      1.满足"手动清洗到达强制清洗时间或一键排液完成", 则设备停止工作
         *      2.手动清洗到达强制清洗时间，可执行清洗操作
         *      3.手动清洗到达强制清洗时间或一键排液完成，可执行加液操作
         *********************************************************************/
        if((ON == air_clean_cfg.power_onoff) && 
                ( (1 == st_current_status.manual_wash_stopdev) || (1 == air_clean_cfg.onekey_drain_fished) ) )
        {
            /*
             * bug001: 植物液有液 --> 一键排液正常结束,清洗亮粉灯 --> 拔插电 --> 显示缺液
             * 原因:   st_current_status.current_plant_liquid 上电初始化为 0 
             */
            //update_plant_level();
            
            //1.手动强制清洗或一键排液完成后, 设备停止工作, 首先需设置以下标志参数
            stop_flag = 0;
            loop_times = 0;
            st_current_status.one_key_atomize_flag = 0;
            
            dev_stop_work();
            
            OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时2秒 
            
            //2.手动清洗到达强制清洗时间，可执行清洗操作
            if(1 == st_current_status.current_wash_status)
            {
                WS_DEBUG("[AirCleanTask 主循环中 003] 手动清洗到达强制清洗时间，可执行清洗操作，开始清洗 \r\n");
                do_current_wash();
            }
            
            //3.手动清洗到达强制清洗时间或一键排液完成，可执行加液操作
            //程序运行到此处, 可能出现 E7/E10/E11, 这里错误码不作判别
            if(ON == st_current_status.add_plant_liquid_flag)
            {
                WS_DEBUG("[AirCleanTask 主循环中 003] 手动清洗到达强制清洗时间或一键排液完成，可执行加液操作，开始加液 \r\n");
                add_plant_liquid();
            }
            
            continue;
        }
        
        
        WS_DEBUG("\r\n[@@@@@ AirCleanTask 主循环中 004 @@@@@] 需求：对工作仓低液位的检测需要多考虑, 因为到低液位点会出现来回跳动(1-->2-->1) \r\n");
        /**********************************************************************
         * 需求：对工作仓低液位的检测需要多考虑, 因为到低液位点会出现来回跳动(1-->2-->1)
         *      1.连续5次检测到工作仓低液位, 即工作仓低液位保持10秒, 认为工作仓真正处于低液位
         *      2.通过 st_current_status.sensor_work_low_confirm 来标识工作仓是否真正达到低液位
         *********************************************************************/
        if((ON == air_clean_cfg.power_onoff) && 
                (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW)))
        {
            sensor_work_low_count += 1;
            if(sensor_work_low_count == 5)      //工作仓低液位保持10秒
            {
                WS_DEBUG("[AirCleanTask 主循环中 004] 工作仓低液位保持10秒，设置 sensor_work_low_confirm \r\n");
                sensor_work_low_count = 0;
                st_current_status.sensor_work_low_confirm = 1;
            }
        }
        else
        {
            sensor_work_low_count = 0;
            st_current_status.sensor_work_low_confirm = 0;
        }
        
        
        WS_DEBUG("\r\n[@@@@@ AirCleanTask 主循环中 005 @@@@@] 需求：原液箱和工作仓都为空, 设置故障码E7用于提示用户 \r\n");
        /**********************************************************************
         * 需求：原液箱和工作仓都为空, 设置故障码E7用于提示用户
         *       两个均为空箱的情况下，可以执行加液操作
         *********************************************************************/
        if((ON == air_clean_cfg.power_onoff)
            && (OFF == st_current_status.current_wash_status)
            && (OFF == st_current_status.onekey_drain_status)
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW))
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW))
            && (1 == st_current_status.sensor_work_low_confirm)         //工作仓低液位确认
            //"工作仓低液位保持10秒"
            && (air_clean_cfg.org_low_to_work_count <= 0))              //原液箱低液位确认
            //"原液箱到低液位后抽液到工作仓的次数减为0"
        {
            WS_DEBUG("[AirCleanTask 主循环中 005] 原液箱和工作仓均确认为低液位\r\n");
            stop_flag = 0;
            loop_times = 0;
            st_current_status.one_key_atomize_flag = 0;
            
            dev_stop_work();
            
            //更新 st_current_status.current_plant_liquid, 为了在显示屏能够实时显示
            st_current_status.current_plant_liquid = PLANT_LIQUID_LOW;
            //若没有设置故障码 E7, 则需设置故障码 E7
            if( (0 == air_clean_cfg.org_low_to_work_count) &&
                    (BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT) != 1) )
            {
                BIT_SET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT);      //设置必须加液的错误码 E7
                write_cfg_to_flash();   //E7 需记录 Flash
            }
            
            //两箱都为空的情况下，程序不往下执行，因此当按了加液键，需执行加液功能
            if((ON == air_clean_cfg.power_onoff) && 
                    (1 == st_current_status.add_plant_liquid_flag))
            {
                WS_DEBUG("[AirCleanTask 主循环中 005] 按了加液键，即将执行加液功能\r\n");
                add_plant_liquid();
            }
            
            OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时2秒 
            continue;
        }
        
        
        WS_DEBUG("\r\n[@@@@@ AirCleanTask 主循环中 006 @@@@@] 需求：原液箱液位相关操作(高/中/低 分别更新为相应的状态) \r\n");
        /**********************************************************************
         * 需求：原液箱液位相关操作(高/中/低 分别更新为相应的状态)
         *********************************************************************/
        //原液箱低液位时更新状态
        if((ON == air_clean_cfg.power_onoff)
            && (OFF == st_current_status.current_wash_status)   //该条件可省略
            && (OFF == st_current_status.onekey_drain_status)   //该条件可省略
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW))) 
        {
            WS_DEBUG("[AirCleanTask 主循环中 006] 原液箱处于低液位，向工作仓剩余加液次数 = %d \r\n", air_clean_cfg.org_low_to_work_count);
            st_current_status.current_plant_liquid = PLANT_LIQUID_LOW;
            if(0 == air_clean_cfg.org_low_to_work_count)
            {
                if(BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT) != 1)
                {
                    BIT_SET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT);      //设置必须加液的错误码 E7
                    write_cfg_to_flash();   //E7 需记录 Flash
                }
            }
        }
        //原液箱中液位时更新状态
        else if((ON == air_clean_cfg.power_onoff)
            && (OFF == st_current_status.current_wash_status)   //该条件可省略
            && (OFF == st_current_status.onekey_drain_status)   //该条件可省略
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_MID))) 
        {
            WS_DEBUG("[AirCleanTask 主循环中 006] 原液箱处于低液位，向工作仓剩余加液次数 = %d \r\n", air_clean_cfg.org_low_to_work_count);
            st_current_status.current_plant_liquid = PLANT_LIQUID_MID;
            //清除必须加液的错误码 E7
            BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);
            //更新"原液箱到低液位后还能抽液到工作仓的次数"
            update_org_low_to_work_count();
        }
        //原液箱高液位时更新状态
        else if((ON == air_clean_cfg.power_onoff)
            && (OFF == st_current_status.current_wash_status)   //该条件可省略
            && (OFF == st_current_status.onekey_drain_status)   //该条件可省略
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_HIGH)))
        {
            WS_DEBUG("[AirCleanTask 主循环中 006] 原液箱处于中液位，更新原液箱可向工作仓剩余加液次数 \r\n");
            st_current_status.current_plant_liquid = PLANT_LIQUID_HIGH;
            //更新"原液箱到低液位后还能抽液到工作仓的次数"
            update_org_low_to_work_count();
            //清除必须加液的错误码 E7
            BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);
        }
        
        
        WS_DEBUG("\r\n[@@@@@ AirCleanTask 主循环中 007 @@@@@] 需求：工作仓液位相关操作(这里只针对低液位做自动配比操作) \r\n");
        /**********************************************************************
         * 需求：工作仓液位相关操作(这里只针对低液位做自动配比操作)
         *********************************************************************/
        if((ON == air_clean_cfg.power_onoff)
            && (OFF == st_current_status.overflow_drain_liquid_status)  //溢出排液不能执行自动配比！
            && (OFF == st_current_status.current_wash_status)
            && (OFF == st_current_status.onekey_drain_status)
            && (OFF == st_current_status.add_plant_liquid_flag)
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW))
            && (st_current_status.sensor_work_low_confirm == 1)         //工作仓低液位这里必须确认！
            && ((get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) != PULLDOWN)
                    || (air_clean_cfg.org_low_to_work_count > 0))
            )
        {
            WS_DEBUG("########## [AirCleanTask] 自动配比工作开始执行 ******************* \r\n");
            
            if(LIQUID_DENSITY_ONE == air_clean_cfg.liquid_density){
                pump2_time = ADD_LIQUID_LOW_TIME;       //低档
            }else if(LIQUID_DENSITY_TWO == air_clean_cfg.liquid_density){
                pump2_time = ADD_LIQUID_MID_TIME;       //中档
            }else if(LIQUID_DENSITY_THREE == air_clean_cfg.liquid_density){
                pump2_time = ADD_LIQUID_HIGH_TIME;      //高档
            }else{
                pump2_time = ADD_LIQUID_HIGH_TIME;      //默认高档
            }
            //检测是否为复位后第一次配比，复位后第一次配比时间要长，防止抽液不能到达工作仓
            if(1 == air_clean_cfg.cfg_data_init_flag)
            {
                WS_DEBUG("########## [AirCleanTask 自动配比] 这是复位后第一次配比，抽液时间设定为 ADD_LIQUID_RESET_TIME \r\n");
                pump2_time = ADD_LIQUID_RESET_TIME;
                air_clean_cfg.cfg_data_init_flag = 0;
                write_cfg_to_flash();       //记录一下 Flash
            }
            
            
            /** 自动配比过程相关说明:
             *  1.配比步骤: 设备停止工作之后, 这里先加水, 后加植物液, 防止每次抽液只是抽的植物液
             *  2.加水过程中一到报警溢液点位就启动工作仓放液，放液到低液位点，启动新一轮注水配比程序
             *  3.电磁阀1需设置保护时间, 一次加水过程中如到达保护时间, 停止加水程序, 关闭电磁阀1, 并报警
             */
            //Step 1. 设备停止工作
            WS_DEBUG("########## [AirCleanTask 自动配比] Step 1. 设备停止工作 \r\n");
            stop_flag = 0;
            loop_times = 0;
            st_current_status.one_key_atomize_flag = 0;
            
            dev_stop_work();
            
            
            //Step 2. 启动电磁阀1 加水到最高液位
            WS_DEBUG("########## [AirCleanTask 自动配比] Step 2. 打开电磁阀1开始加水 \r\n");
            timeout = 0;
            timeout_flag = 0;
            peibi_supply_water_flag = 0;
            
            solenoid_onoff(SOLENOID_INTO_WORK, ON);     //打开电磁阀1开始加水
            while(1)
            {
                //检测是否达到溢出液位, 优先级最高
                if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_OVERFLOW) == PULLDOWN)
                {
                    WS_DEBUG("########## [AirCleanTask 自动配比-->加水] 加水过程中检测到达到溢出液位，异常退出加水过程 \r\n");
                    break;
                }
                
                //关闭电源/执行清洗/一键排液/添加原液, 则停止配比
                if((OFF == air_clean_cfg.power_onoff)
                    || (ON == st_current_status.current_wash_status)
                    || (ON == st_current_status.onekey_drain_status)
                    || (ON == st_current_status.add_plant_liquid_flag)) 
                {
                    WS_DEBUG("########## [AirCleanTask 自动配比-->加水] 关闭电源/执行清洗/一键排液/添加原液等操作，异常退出加水过程 \r\n");
                    break;
                }
                
                if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) == PULLDOWN)
                {
                    peibi_supply_water_flag = 1;
                    WS_DEBUG("########## [AirCleanTask 自动配比-->加水] 加水成功(到达高液位)，正常退出加水过程 \r\n");
                    break;
                }
                
                OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
                
                timeout++;
                if(timeout >= SUPPLY_WATER_TIMEOUT)
                {
                    timeout_flag = 1;
                    WS_DEBUG("########## [AirCleanTask 自动配比-->加水] 加水达到超时保护时间，异常退出加水过程 \r\n");
                    break;
                }
            }
            solenoid_onoff(SOLENOID_INTO_WORK, OFF);    //关闭电磁阀1停止加水
            if(timeout_flag)
            {
                //工作仓未到达最高液位点, 设置E3进水故障
                if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) != PULLDOWN)
                {
                    BIT_SET(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER);
                    WS_DEBUG("########## [AirCleanTask 自动配比-->加水] 加水超时，工作仓没有达到高液位，设置 ERR_WORK_SUPPLY_WATER 故障码 \r\n");
                }
            }
            
            
            //Step 3. 启动液泵2进行抽液 (当工作仓液位离开低液位时再抽取植物液到工作仓)
            WS_DEBUG("########## [AirCleanTask 自动配比] Step 3. 启动液泵2进行抽液 \r\n");
            if(peibi_supply_water_flag == 1)
            {
                WS_DEBUG("########## [AirCleanTask 自动配比-->抽液] 液泵2进行抽液延时相应的时间 \r\n");
                liquid_pump_onoff(PUMP_TO_WORK, ON);        //液泵2开始抽液
                OSTimeDlyHMSM(0,0,0,pump2_time,OS_OPT_TIME_HMSM_STRICT,&err);   //延时相应的时间
                liquid_pump_onoff(PUMP_TO_WORK, OFF);       //液泵2停止抽液
                
                //出现E11, 配比注水正常后第一次 peibi_overflow_num 设置为 0, 第二次正常再自动清除E11
                if(1 == st_current_status.peibi_overflow_num)
                {
                    WS_DEBUG("########## [AirCleanTask 自动配比] E11 后第一次配比成功，peibi_overflow_num 设置为 0！ \r\n");
                    st_current_status.peibi_overflow_num = 0;
                }
                else if(0 == st_current_status.peibi_overflow_num)
                {
                    if(BIT_GET(air_clean_cfg.err_code_bitmap, ERR_OVERFLOW_FIRST))
                    {
                        WS_DEBUG("########## [AirCleanTask 自动配比] E11 后第二次配比成功，清除 ERR_OVERFLOW_FIRST！ \r\n");
                        clear_first_overflow_time();
                        BIT_CLEAR(air_clean_cfg.err_code_bitmap, ERR_OVERFLOW_FIRST);
                    }
                }
                
                if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) == PULLDOWN)
                {
                    WS_DEBUG("########## [AirCleanTask 自动配比] 自动配比正常完成，液箱已经位于低液位，更新 org_low_to_work_count \r\n");
                    air_clean_cfg.org_low_to_work_count -= 1;
                    write_cfg_to_flash();
                    WS_DEBUG("########## [AirCleanTask 自动配比] 当前 air_clean_cfg.org_low_to_work_count 值 = %d \r\n", air_clean_cfg.org_low_to_work_count);
                    if(0 == air_clean_cfg.org_low_to_work_count)
                    {
                        WS_DEBUG("########## [AirCleanTask 自动配比] 液箱位于低液位，抽液达到指定次数，设置加液的错误码 E7 ！！！ \r\n");
                        
                        if(BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT) != 1)
                        {
                            BIT_SET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT);      //设置必须加液的错误码 E7
                            write_cfg_to_flash();   //E7 需记录 Flash
                        }
                    }
                }
            }
            else
            {
                WS_DEBUG("########## [AirCleanTask 自动配比-->抽液] 由于加水过程异常退出，并未执行抽液操作 \r\n");
            }
            
            WS_DEBUG("########## [AirCleanTask] 自动配比工作执行结束 ******************* \r\n");
        }
        
        
        /**********************************************************************
         * 需求：错误码相关操作
         *********************************************************************/
        //if( (ERR_NONE != air_clean_cfg.err_code_bitmap) \
        //        && (get_err_num()!= ORG_MUST_ADD_PLANT) && (get_err_num()!= FORCE_WASH_STOP_DEV) )
        //{
        //    //把故障码通知给服务器，
        //    //把优先级最高的故障码显示在屏幕上。
        //}
        
        
        WS_DEBUG("\r\n[@@@@@ AirCleanTask 主循环中 008 @@@@@] 需求：处理遥控器按键相关操作 \r\n");
        /**********************************************************************
         * 需求：处理遥控器按键相关操作
         *********************************************************************/
        //处理"雾化器A/B切换"
        if(ON == st_current_status.siwtch_atomizer_flag)
        {
            WS_DEBUG("[AirCleanTask 主循环中 008] 处理雾化器A/B切换 \r\n");
            if(is_running == ON) 
            {
                if(air_clean_cfg.which_atomizer == ATOMIZER_ONE) {
                    atomizer_onoff(ATOMIZER_TWO,OFF);
                    atomizer_onoff(ATOMIZER_ONE,ON);
                } else {
                    atomizer_onoff(ATOMIZER_ONE,OFF);
                    atomizer_onoff(ATOMIZER_TWO,ON);
                }
                
                //更新雾化器使用起始时间 -- 在接收到"雾化器切换"按键时处理
                //update_cycle_time(ATOMIZER_INTERVAL_TYPE);
            }
            st_current_status.siwtch_atomizer_flag = OFF;
            
            //loop_times 清空为 0, 重新计算"开几停几"状态
            loop_times=0;
            st_current_status.one_key_atomize_flag = 0;
        }
        
        //处理 "风量调节" 
        if(st_current_status.siwtch_fan_level_flag == ON) 
        {
            WS_DEBUG("[AirCleanTask 主循环中 008] 处理风量调节 \r\n");
            if(is_running == ON) 
            { 
                set_air_fan_level();
            }
            st_current_status.siwtch_fan_level_flag = OFF;
        }
        
        
        WS_DEBUG("\r\n[AirCleanTask] ======================= 调试信息分割标识 ======================= \r\n\r\n");
        
        //关闭电源/执行清洗/一键排液/添加原液: 必须关闭所有设备
        if((OFF == air_clean_cfg.power_onoff) || 
                (ON == st_current_status.current_wash_status)|| 
                (ON == st_current_status.onekey_drain_status) || 
                (ON == st_current_status.add_plant_liquid_flag) ||
                (0 == check_if_dev_can_work()))     //不符合正常工作条件
        {
            stop_flag = 0;
            loop_times = 0;
            st_current_status.one_key_atomize_flag = 0;
            
            dev_stop_work();
        }
        
        //执行清洗操作
        if( (ON == air_clean_cfg.power_onoff) && 
                (ON == st_current_status.current_wash_status) && 
                (OFF == st_current_status.overflow_drain_liquid_status) )
        {
            WS_DEBUG("[AirCleanTask] 开始执行 do_current_wash(清洗) 函数 ...\r\n");
            do_current_wash();
            WS_DEBUG("[AirCleanTask] 执行完毕 do_current_wash(清洗) 函数 ...\r\n");
        }
        
        //执行一键排液
        if( (ON == air_clean_cfg.power_onoff) && 
                (ON == st_current_status.onekey_drain_status) && 
                (OFF == st_current_status.overflow_drain_liquid_status) )
        {
            WS_DEBUG("[AirCleanTask] 开始执行 do_onekey_drain(一键排液) 函数 ...\r\n");
            do_onekey_drain();
            WS_DEBUG("[AirCleanTask] 执行完毕 do_onekey_drain(一键排液) 函数 ...\r\n");
        }
        
        //添加原液操作
        if( (ON == air_clean_cfg.power_onoff) && 
                (ON == st_current_status.add_plant_liquid_flag) && 
                (OFF == st_current_status.overflow_drain_liquid_status) )
        {
            WS_DEBUG("[AirCleanTask] 开始执行 add_plant_liquid(加液) 函数 ...\r\n");
            add_plant_liquid();
            WS_DEBUG("[AirCleanTask] 执行完毕 add_plant_liquid(加液) 函数 ...\r\n");
        }
        
        //设置模式或设置时间完成, 需要检查是否在运行时间段, 更新设备的运行状态
        //这里用的 mt_setting_finished 来标识"时间或模式"设置完成, 没有分开处理
        if(1 == st_setting_status.mt_setting_finished)
        {
            WS_DEBUG("[AirCleanTask] 检测到'设置模式/时间完成标识'被置位, 更新设备的运行状态 \r\n");
            
            //设置时间/模式后, 不在工作时间范围内, 不考虑设备当前状态, 设备停止运行
            if(!get_whole_dev_onoff())
            {
                WS_DEBUG("[AirCleanTask] 设置时间/模式后, 不在工作时间范围内, 设备停止运行 \r\n");
                stop_flag = 0;
                loop_times = 0;
                dev_stop_work();
            }
            //设置时间/模式后, 在工作时间范围内:
            //1.若设备当前状态正在运行, 继续设备运行, 重新计时
            //2.若设备当前状态不在运行, 需要开启设备运行
            else
            {
                WS_DEBUG("[AirCleanTask] 设置时间/模式后, 处在工作时间范围内, 更新设备状态 \r\n");
                stop_flag = 0;
                loop_times = 0;
            }
            st_current_status.one_key_atomize_flag = 0;
            st_setting_status.mt_setting_finished = 0;
        }
        
        
        WS_DEBUG("\r\n[@@@@@ AirCleanTask 主循环中 111 @@@@@] 需求：当所有故障解除，符合正常工作(出雾)的条件后所需做的工作 \r\n");
        /**********************************************************************
         * 需求：当所有故障解除，符合正常工作(出雾)的条件后所需做的工作
         *********************************************************************/
        if( (ON == air_clean_cfg.power_onoff) && 
                (OFF == st_current_status.current_wash_status) && 
                (OFF == st_current_status.onekey_drain_status) && 
                (OFF == st_current_status.add_plant_liquid_flag) && 
                (OFF == st_current_status.overflow_drain_liquid_status) && 
                (PULLDOWN != get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW)) &&
                (0 == st_current_status.sensor_work_low_confirm) &&     //工作仓不在低液位
                (1 == check_if_dev_can_work()) )                        //设备没有导致不能正常工作的故障码
                //&& ( (ERR_NONE == air_clean_cfg.err_code_bitmap) || (get_err_num() == ERR_OVERFLOW_FIRST) 
                //        || (get_err_num() == ORG_MUST_ADD_PLANT) )
        {
            WS_DEBUG("[AirCleanTask 主循环中 111] 符合正常工作(出雾)的条件 \r\n");
            
            //1.不在工作时间范围内
            if(!get_whole_dev_onoff())
            {
                WS_DEBUG("[AirCleanTask 主循环中 111 --> 001] 不在工作时间范围内 \r\n");
                
                //一键雾化标识为 1, 则执行出雾两分钟
                if(1 == st_current_status.one_key_atomize_flag)
                {
                    WS_DEBUG("[AirCleanTask 主循环中 111 --> 001] 一键雾化标识为 1, 则执行出雾两分钟 \r\n");
                    if(0 == is_running)     //设备开始工作的时候, loop_times 清零
                        loop_times = 0;
                    
                    atomizer_start_work();
                }
                else
                {
                    WS_DEBUG("[AirCleanTask 主循环中 111 --> 001] 一键雾化标识为 0, 则执行设备停止工作 \r\n");
                    if(1 == is_running)
                    {
                        stop_flag = 0;
                        loop_times = 0;
                        st_current_status.one_key_atomize_flag = 0;
                        
                        dev_stop_work();
                    }
                }
            }
            //2.正在工作时间范围内
            else
            {
                //注意: 正在工作时间范围内, 一键出雾功能不做处理！直接清除一键出雾标识
                WS_DEBUG("[AirCleanTask 主循环中 111 --> 002] 正在工作时间范围内 \r\n");
                st_current_status.one_key_atomize_flag = 0;
                
                if(stop_flag == 0)
                {
                    WS_DEBUG("[AirCleanTask 主循环中 111 --> 002] 启动设备正常出雾 \r\n");
                    dev_start_work();
                }
                
                //设备正在工作, 检查是否在运行时间段(这块逻辑应该是多余的)
                if((1 == is_running) && (!get_whole_dev_onoff()))
                {
                    stop_flag = 0;
                    loop_times = 0;
                    dev_stop_work();
                }
            }
        }
        
        loop_times++;
        
        //这里是否与 "所有故障解除可以正常工作"  的条件完全一致？？？
        //if((ON == air_clean_cfg.power_onoff)
        //        && (OFF == st_current_status.overflow_drain_liquid_status)
        //        && (ERR_NONE == air_clean_cfg.err_code_bitmap))
        //{
        if( (ON == air_clean_cfg.power_onoff) && 
                (OFF == st_current_status.current_wash_status) && 
                (OFF == st_current_status.onekey_drain_status) && 
                (OFF == st_current_status.add_plant_liquid_flag) && 
                (OFF == st_current_status.overflow_drain_liquid_status) && 
                (PULLDOWN != get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW)) &&
                (0 == st_current_status.sensor_work_low_confirm) &&     //工作仓不在低液位
                (1 == check_if_dev_can_work()) )                        //设备没有导致不能正常工作的故障码
        {
            //*****************************************************************
            //一键出雾功能
            if(1 == st_current_status.one_key_atomize_flag)
            {
                //WS_DEBUG("one_key_atomize_flag --> loop_times = %d \r\n", loop_times);
                if( (1 == is_running) && (loop_times >= 30*2) )     //出雾两分钟
                {
                    WS_DEBUG("[AirCleanTask 一键出雾检测 001] 一键出雾达到出雾2分钟，设备停止工作 \r\n");
                    loop_times = 0;
                    st_current_status.one_key_atomize_flag = 0;
                    
                    dev_stop_work();
                }
            }
            
            //非一键出雾功能
            else
            {
                //自定义模式
                if(MODE_CUSTOM == air_clean_cfg.work_mode)
                {
                    if( (1 == is_running) && (loop_times >= 30*(air_clean_cfg.st_custom_mode.custom_open_min)) )
                    {
                        WS_DEBUG("[AirCleanTask 正常工作检测 001] 自定义模式下达到开几分，设备停止工作 \r\n");
                        stop_flag = 1;
                        loop_times = 0;
                        
                        dev_stop_work();
                    }
                    else if( (0 == is_running) && (loop_times >= 30*(air_clean_cfg.st_custom_mode.custom_close_min)) )
                    {
                        WS_DEBUG("[AirCleanTask 正常工作检测 001] 自定义模式下达到关几分，设备开始工作 \r\n");
                        stop_flag = 0;
                        loop_times = 0;
                        
                        dev_start_work();
                    }
                }
                //标准模式
                else
                {
                    if( (1 == is_running) && (loop_times >= 30*(air_clean_cfg.standard_mode_open_min)) )
                    {
                        WS_DEBUG("[AirCleanTask 正常工作检测 002] 标准模式下达到开几分，设备停止工作 \r\n");
                        stop_flag = 1;
                        loop_times = 0;
                        
                        dev_stop_work();
                    }
                    else if( (0 == is_running) && (loop_times >= 30*(air_clean_cfg.standard_mode_close_min)) )
                    {
                        WS_DEBUG("[AirCleanTask 正常工作检测 002] 标准模式下达到关几分，设备开始工作 \r\n");
                        stop_flag = 0;
                        loop_times = 0;
                        
                        dev_start_work();
                    }
                }
            }
        }
        
        OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时2秒 
    }
}


/*
*********************************************************************************************************
*                                           KEY CONTROL TASK
*********************************************************************************************************
*/
//按键控制线程
unsigned char tmp_key;
void KeyControlTask(void * p_arg)
{
    int cnt;
    unsigned char key;
    OS_ERR err;
    
    (void)p_arg;

    WS_DEBUG("[KeyControlTask 主循环即将开始] 创建 KeyControlTask 成功... \r\n");

    while(1) 
    {
        key = Remote_Scan();                    //获取按键值(bsp/bsp_remote.c 文件中定义)
        //WS_DEBUG("[KeyControlTask func] Remote_Scan return key value = 0x%02x\r\n", key);
        
        if(key)
        {
            //WS_DEBUG("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& KeyControlTask &&&&&&&&&&&&&&&&&&&&&&&&&&&&& \r\n\r\n");
            
            OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err);
            
            /*
            while((tmp_key = Remote_Scan()))
            {
                if(tmp_key == key)
                    continue; 
            }
            */

            WS_DEBUG("[KeyControlTask func] Remote_Scan return key value = 0x%02x\r\n", key);
            
            #if MANUAL_EDITION
                if(IR_CODE_DENSITY == key)      //手动版本浓度按键不作处理
                {
                    continue;
                }
            #endif
            
            //电源 按键独立, 当电源为关闭状态时, 不更新状态
            if((ON == air_clean_cfg.power_onoff) || (key == IR_CODE_POWER))
            {
                //溢出液位排液过程中按键不处理？？？？
                if(ON == st_current_status.overflow_drain_liquid_status)
                {
                    //如果这里不作处理，那么即使正在执行溢出排液，那么也有按键(清洗+确认)也可能触发清洗
                    //在每个具体操作中添加检测溢出液位？  加液，清洗，一键排液   应该没必要
                    //在执行每个动作之前，检查是否正在执行溢出排液操作？
                    
                    
                    //这里暂定为只处理 电源键和复位键
                    if((IR_CODE_POWER != key) && (IR_CODE_RESET != key) && (IR_CODE_CONFIRM_LONG != key))
                    {
                        //WS_DEBUG("[KeyControlTask func] setting_time: key=0x%X not handle.\r\n", key);
                        continue;
                    }
                    
                }
                
                //1.正在"设置时间"情形
                //      可处理的按键: 电源, 上下左右, 确认(长短)/返回
                if(1 == st_setting_status.setting_time_flag)
                {
                    if((IR_CODE_POWER != key) && (IR_CODE_UP != key) && (IR_CODE_DOWN != key)
                            && (IR_CODE_LEFT != key) && (IR_CODE_RIGHT != key) && (IR_CODE_CANCEL != key)
                            && (IR_CODE_CONFIRM_SHORT != key) && (IR_CODE_CONFIRM_LONG != key))
                    {
                        //WS_DEBUG("[KeyControlTask func] setting_time: key=0x%X not handle.\r\n", key);
                        continue;
                    }
                }
                
                //2.正在"设置模式"情形
                //      1.正在选择设置的"模式类型"
                //      2.本地模式选择
                //      3.自定义模式设置
                //      //可处理的按键: 电源, 上下左右, 确认(长短)/返回
                else if(1 == st_setting_status.setting_mode_flag)
                {
                    //正在选择设置的"模式类型"
                    if(0 == st_mode_setting.set_mode_type_flag)
                    {
                        //只能处理: 电源, 上下, 确认(长短)/返回
                        if((IR_CODE_POWER != key) && (IR_CODE_UP != key) && (IR_CODE_DOWN != key) && (IR_CODE_CANCEL != key)
                                && (IR_CODE_CONFIRM_SHORT != key) && (IR_CODE_CONFIRM_LONG != key))
                        {
                            //WS_DEBUG("[KeyControlTask func] setting_mode-->select_mode_type: key=0x%X not handle.\r\n", key);
                            continue;
                        }
                    }
                    //已经选择好设置的"模式类型"
                    else
                    {
                        //标准模式相关设置, 只设置开几停几
                        if(1 == st_mode_setting.mode_type_select)
                        {
                            //"标准模式下:正在设置开几停几" -- 只能处理的按键: 上下,确认,取消,电源
                            if(st_mode_setting.std_set_open_close_flag>0 && st_mode_setting.std_set_open_close_flag<3)
                            {
                                if((IR_CODE_POWER != key) && (IR_CODE_UP != key) && (IR_CODE_DOWN != key) && (IR_CODE_CANCEL != key)
                                        && (IR_CODE_CONFIRM_SHORT != key) && (IR_CODE_CONFIRM_LONG != key))
                                {
                                    //WS_DEBUG("[KeyControlTask func] setting_mode-->std_set_open_close_flag: key=0x%X not handle.\r\n", key);
                                    continue;
                                }
                            }
                        }
                        
                        //自定义模式相关设置, 并选择该自定义模式
                        if(2 == st_mode_setting.mode_type_select)
                        {
                            //A."自定义模式下:正在设置周开关" -- 只能处理的按键: 确认(长短),取消,电源
                            if(st_mode_setting.set_work_week_flag>0 && st_mode_setting.set_work_week_flag<8)
                            {
                                if((IR_CODE_POWER != key) && (IR_CODE_CANCEL != key)
                                        && (IR_CODE_CONFIRM_SHORT != key) && (IR_CODE_CONFIRM_LONG != key))
                                {
                                    //WS_DEBUG("[KeyControlTask func] setting_mode-->set_work_week: key=0x%X not handle.\r\n", key);
                                    continue;
                                }
                            }
                            
                            //B."自定义模式下:正在设置时间段" -- 只能处理的按键: 上下,确认,取消,电源
                            if(8 == st_mode_setting.set_work_week_flag && st_mode_setting.set_work_time_flag>0 && st_mode_setting.set_work_time_flag<5)
                            {
                                if((IR_CODE_POWER != key) && (IR_CODE_UP != key) && (IR_CODE_DOWN != key)
                                        && (IR_CODE_LEFT != key) && (IR_CODE_RIGHT != key) && (IR_CODE_CANCEL != key)
                                        && (IR_CODE_CONFIRM_SHORT != key) && (IR_CODE_CONFIRM_LONG != key))
                                {
                                    //WS_DEBUG("[KeyControlTask func] setting_mode-->set_work_time: key=0x%X not handle.\r\n", key);
                                    continue;
                                }
                            }
                            
                            //C."自定义模式下:正在设置开几停几" -- 只能处理的按键: 上下,确认,取消,电源
                            if(8 == st_mode_setting.set_work_week_flag && 5 == st_mode_setting.set_work_time_flag && st_mode_setting.set_open_close_flag>0 && st_mode_setting.set_open_close_flag<3)
                            {
                                if((IR_CODE_POWER != key) && (IR_CODE_UP != key) && (IR_CODE_DOWN != key) && (IR_CODE_CANCEL != key)
                                        && (IR_CODE_CONFIRM_SHORT != key) && (IR_CODE_CONFIRM_LONG != key))
                                {
                                    //WS_DEBUG("[KeyControlTask func] setting_mode-->set_open_close: key=0x%X not handle.\r\n", key);
                                    continue;
                                }
                            }
                        }
                    }
                }
                
                //3.按下"清洗键", 等待确认键或取消键
                if(1 == st_current_status.wait_wash_confirm)
                {
                    //如果不是按的"确认键"或者"取消键", wait_wash_confirm("等待清洗确认"标志)
                    if((IR_CODE_CONFIRM_SHORT != key) && (IR_CODE_CANCEL != key) && (IR_CODE_WASH_SHORT != key))
                    {
                        st_current_status.wait_wash_confirm = 0;    //清除"等待清洗确认"标志
                    }
                }
                
                //4."错误码"情形(应该放在设置项后, 如果正在进行设置, 设置优先处理)
                //      1.这里只可处理: 电源键(开关设备), 长按确认键(清除错误码)
                //      2.若在"设置时间/设置模式..."过程中, 出现错误码, 不应该停止设置 
                //      3.显示屏那, 也需要注意不能显现错误码, 需要等待设置完成之后再显示错误码 
                if( (ERR_NONE != air_clean_cfg.err_code_bitmap) && (get_err_num()!= ORG_MUST_ADD_PLANT) \
                        && (get_err_num()!= FORCE_WASH_STOP_DEV) && (get_err_num()!= ERR_OVERFLOW_FIRST) )
                {
                    if((IR_CODE_POWER != key) && (IR_CODE_CONFIRM_LONG != key))
                    {
                        //WS_DEBUG("[KeyControlTask func] err_code_bitmap: key=0x%X not handle.\r\n", key);
                        continue;
                    }
                }
                
                //5.正在清洗或正在一键排液, 以下按键不作处理: 复位, 清洗(长按+短按), 加液
                //如何取消清洗 -- 断电或者关闭电源键可取消清洗
                if((1 == st_current_status.current_wash_status) || (1 == st_current_status.onekey_drain_status))
                {
                    if((IR_CODE_WASH_SHORT == key) || (IR_CODE_RESET == key) || 
                            (IR_CODE_ADD_LIQUID_SHORT == key) || (IR_CODE_WASH_LONG == key))
                        continue;
                }
                
                //6.正在加液, 以下按键不作处理: 复位, 清洗(长按+短按)
                if(1 == st_current_status.add_plant_liquid_flag)
                {
                    if((IR_CODE_WASH_SHORT == key) || (IR_CODE_WASH_LONG == key) 
                            || (IR_CODE_RESET == key))
                        continue;
                }
                
                //WS_DEBUG("[KeyControlTask func] macBEEP_ON && macBEEP_OFF\r\n");
                macBEEP_ON();
                OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err);
                macBEEP_OFF();
                
                
                //WS_DEBUG("[KeyControlTask func] ############## before update_status_by_data_code #################\r\n");
                update_status_by_data_code(key);    //根据按键值更新状态(app/appfunc.c 文件中定义)
            }
        }
        //tmp_key = key;
        
        OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err);                      //延时 1ms
        //延时时间应该长一点，否则该任务的优先级较高时，其他任务将不会被执行到
    }
}
/*
01: 复位键; 02: 电源键; 03: 时间键; 04: 网络键;
05: 方向键; 06: 确认键; 07: 模式键; 08: 返回键;
09: 清洗键; 10: 浓度键; 11: 风量键; 12: 加液键; 
13: 雾化器; 14: 氨气值;
*/



/*
*********************************************************************************************************
*                                          SCREEN REFRESH TASK
*********************************************************************************************************
*/
/*
显示屏显示内容如下:
     PIC   数码显示(00:00) PIC  PIC  PIC
    植物液   NH3  氨气值  时间  音量  周

    标准模式   A/B  PIC  PIC  PIC  PIC  PIC
    智能模式 雾化器 风量 浓度 清洗 网络 电源
*/
#define ROW_NUM     11
void ScreenRefreshTask(void * p_arg)
{
    OS_ERR err;
    
    char i;
    int count;
    unsigned char row_data[ROW_NUM] = {0};
    
    (void)p_arg;
    
    WS_DEBUG("[ScreenRefreshTask 主循环即将开始] 创建 ScreenRefreshTask 成功... \r\n");
    
    //初始设置"外置RTC"
    DS3231_Init();
    
    /*
    get_show_time();
    OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
    
    if((0 == air_clean_cfg.atomizer_start_time.start_year) && (0 == air_clean_cfg.atomizer_start_time.start_month)
            && (0 == air_clean_cfg.atomizer_start_time.start_day))
    {
        update_cycle_time(ATOMIZER_INTERVAL_TYPE);
    }
    if((0 == air_clean_cfg.dev_work_time.start_year) && (0 == air_clean_cfg.dev_work_time.start_month)
            && (0 == air_clean_cfg.dev_work_time.start_day))
    {
        update_cycle_time(WASH_INTERVAL_TYPE);
    }
    */
    if((0 == air_clean_cfg.atomizer_start_time.start_year) || (0 == air_clean_cfg.dev_work_time.start_year))
    {
        get_show_time();
        if(0 == air_clean_cfg.atomizer_start_time.start_year) 
        {
            update_cycle_time(ATOMIZER_INTERVAL_TYPE);
        }
        if(0 == air_clean_cfg.dev_work_time.start_year) 
        {
            update_cycle_time(WASH_INTERVAL_TYPE);
        }
        OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
    }
    
    count = 0;
    st_current_status.start_up_flag = 1;                //开机启动标识置为1
    while(1)
    {
        //WS_DEBUG("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& ScreenRefreshTask &&&&&&&&&&&&&&&&&&&&&&&&&&&&& \r\n\r\n");
        
        get_show_time();
        update_display_data(row_data);
        
        /*
        for(i=0; i<4; i++)
        {
            memset(string, 0, sizeof(string));
            itoa((int)(row_data[i]),string,2);
            WS_DEBUG("row_data[%d] = %d \r\n", i, row_data[i]);
        }
        */
        
        TM1640_Display(row_data);       //显示
        
        //每隔 500 ms 刷新一次
        OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
        
        
        if(1 == st_current_status.start_up_flag)
        {
            count ++;
            if(count >= 5)
            {
                count = 0;
                st_current_status.start_up_flag = 0;    //开机启动标识置为0
            }
        }
    }
}


/*
*********************************************************************************************************
*                                           CYCLE CHECK/UPDATE TASK
*********************************************************************************************************
*/
//检测"关闭风扇"任务
void CloseFanTask(void * p_arg) 
{
    OS_ERR err; 
    
    (void)p_arg;
    
    WS_DEBUG("[CloseFanTask 主循环即将开始] 创建 CloseFanTask 成功... \r\n");
    
    while(1)
    {
        //WS_DEBUG("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& CloseFanTask &&&&&&&&&&&&&&&&&&&&&&&&&&&&& \r\n\r\n");
        if(1 == st_current_status.close_fan_flag)
        {
            //延时 5 秒关闭风扇
            OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时 5 S
            //OS_CRITICAL_ENTER();
            if(1 == st_current_status.close_fan_flag)
            {
                air_fan_onoff(OFF);
                st_current_status.close_fan_flag = 0;
            }
            //OS_CRITICAL_EXIT();
        }
        
        /* 这里是否可以改进使用"ucos中信号量"来实现 */
        //每隔 200 ms 检查一次
        OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
    }
}



#define SEC_30_COUNT    6   //每次睡眠 5 秒, 30秒定义为6
#define SEC_60_COUNT    12
#define SEC_150_COUNT   18
/**
 *  定时任务：()
 *      1.每隔30秒上传一次服务器数据
 *      2.每隔1分钟获取一次氨气浓度值
 *      3.检测 "网络触发闪烁时间", 闪烁1分30秒
 *      4.风扇正在工作时, 每隔1分钟检测风扇的状态
 */
void ScheduledTask(void * p_arg)
//void UpdateNH3Task(void * p_arg)
{
    OS_ERR err; 
    
    char fan_check_count = 0;
    char fan_one_check_error_count = 0;       //
    char fan_two_check_error_count = 0;
    
    #if NETWORK_VERSION
    char net_upload_count = 0;
    
    
    int ret = -1;
    char nh3_set_count = 0;         //
    char net_trigger_count = 0;     //网络触发闪烁时间计时
    #endif
    
    (void)p_arg;
    
    WS_DEBUG("[ScheduledTask 主循环即将开始] 创建 ScheduledTask 成功... \r\n");
    
    #if NETWORK_VERSION
        //初始设置氨气传感器工作模式: "问答式", 返回0为成功
        NH3_Sensor_Init_Request();
        OSTimeDlyHMSM(0,0,0,600,OS_OPT_TIME_HMSM_STRICT,&err);
        ret = NH3_Sensor_Init_Check();

        WS_DEBUG("[ScheduledTask] NH3_Sensor_Init return : %d\r\n",ret);
    #endif
    
    while(1) 
    {
        //WS_DEBUG("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& ScheduledTask &&&&&&&&&&&&&&&&&&&&&&&&&&&&& \r\n\r\n");
        
        //每隔5秒更新一次
        OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
        
        #if NETWORK_VERSION
        //1.每隔30秒上传服务器一次数据(能联网状态下即上传数据)
        net_upload_count ++;
        if(net_upload_count == SEC_30_COUNT)
        {
            net_upload_count = 0;
            UploadDeviceInfo(0x03);
        }
        #endif
        
        
        if(ON == air_clean_cfg.power_onoff)
        {
            #if NETWORK_VERSION
                //1.每隔30秒上传服务器一次数据
                //UploadDeviceInfo(0x03);
                
                /*
                //2.每隔1分钟获取一次NH3浓度值 -- 修改为: 每隔5秒获取一次NH3浓度值
                nh3_set_count ++;
                if(nh3_set_count == SEC_60_COUNT)
                {
                    nh3_set_count = 0;
                    set_nh3_concentration();        //设置当前NH3平均浓度值
                }
                */
                //2.每隔5秒获取一次NH3浓度值
                set_nh3_concentration();        //设置当前NH3平均浓度值
                
                //3.检测 "网络触发闪烁时间", 闪烁1分30秒
                if(1 == st_current_status.net_trigger_flag)
                {
                    net_trigger_count++;
                    if(net_trigger_count >= SEC_150_COUNT)
                    {
                        st_current_status.net_trigger_flag = 0;
                        net_trigger_count = 0;
                    }
                }
            #endif
            
            
            //风扇正在工作时, 每隔1分钟检测风扇的状态
            if(1 == is_running)
            {
                fan_check_count ++;
                if(fan_check_count >= SEC_60_COUNT)        //间隔1分钟检测
                {
                    fan_check_count = 0;
                    
                    //检测风扇 1
                    global_fan_one_clear_flag = 1;
                    ADVANCE_TIM_Init();             //开始定时器检测
                    OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
                    //WS_DEBUG("[ScheduledTask] global_fan_one_clear_flag = %d \r\n", global_fan_one_clear_flag);
                    
                    if(1 == global_fan_one_clear_flag)            //
                    {
                        DISABLE_ADVANCE_TIM();          //禁用定时器检测
                        fan_one_check_error_count ++;
                        if(fan_one_check_error_count >= 2)  //连续 2 次检测有问题
                        {
                            fan_one_check_error_count = 0;
                            BIT_SET(air_clean_cfg.err_code_bitmap,ERR_FAN_ONE_FAULT);
                            dev_stop_work();
                        }
                        global_fan_one_clear_flag = 0;
                    }
                    
                    /*-----------------------------------------------------------*/
                    
                    //检测风扇 2
                    global_fan_two_clear_flag = 1;
                    ADVANCE2_TIM_Init();
                    OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
                    //WS_DEBUG("[ScheduledTask] global_fan_two_clear_flag = %d \r\n", global_fan_two_clear_flag);
                    
                    if(1 == global_fan_two_clear_flag)            //
                    {
                        DISABLE_ADVANCE2_TIM();          //禁用定时器检测
                        fan_two_check_error_count ++;
                        if(fan_two_check_error_count >= 2)
                        {
                            fan_two_check_error_count = 0;
                            BIT_SET(air_clean_cfg.err_code_bitmap,ERR_FAN_TWO_FAULT);
                            dev_stop_work();
                        }
                        global_fan_two_clear_flag = 0;
                    }
                }
            }
        }
    }
}


//检测"雾化器切换/强制清洗"任务
#define  CYCLE_CHECK_TIME           10
#define  TEST_CYCLE_CHECK_TIME      2
void CycleCheckTask(void * p_arg) 
{
    OS_ERR err; 
    
    int e7_close_days;      //设备出现关机(不是断电)或E7状态的天数
    
    int wash_days;
    int atomizer_days;
    
    (void)p_arg;
    
    WS_DEBUG("[CycleCheckTask 主循环即将开始] 创建 CycleCheckTask 成功... \r\n");
    
    
    //需要等屏幕刷新线程中外部时钟模块初始化且正常运行, 并判别了是否需要执行 update_cycle_time(**)
    OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
    
    while(1) 
    {
        //WS_DEBUG("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& CycleCheckTask &&&&&&&&&&&&&&&&&&&&&&&&&&&&& \r\n\r\n");
        
        //设备出现关机（不是断电）或E7状态，第4天开始的凌晨2点开始注水
        /*此刻没有正在执行保养清洗/一键清洗/自动清洗/溢出排液  &&  出现关机或E7状态  
        --> 检测关机或E7状态的天数*/
        if( (OFF == st_current_status.power_off_e7_maintance_status) && 
            (OFF == st_current_status.onekey_drain_status) &&
            (OFF == st_current_status.current_wash_status) &&
            (OFF == st_current_status.overflow_drain_liquid_status) &&
            ((OFF == air_clean_cfg.power_onoff) 
                    || BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT)) 
        )
        {
            e7_close_days = get_interval_days(E7_CLOSE_INTERVAL_TYPE);
            WS_DEBUG("[CycleCheckTask] 检测到关机或E7状态的天数为 %d 天 !!!!!!!!!! \r\n", e7_close_days);
            if( (e7_close_days >= (E7_CLOSE_MAINTAIN_DAY+1)) && (E7_CLOSE_MAINTAIN_HOUR == calendar.hour) )
            {
                //设备出现关机（不是断电）或E7状态，第4天开始的凌晨2点开始注水，
                //注水到高位开始排液，排液到低点保持40S,然后注满水到高位。
                //若注水不能到高位，则不继续后续程序。每间隔3天循环此动作
                //power_off_e7_maintance();
                WS_DEBUG("[CycleCheckTask] 达到关机或E7状态执行保养清洗的天数，即将执行保养清洗%%%%%% \r\n");
                st_current_status.power_off_e7_maintance_status = 1;
            }
        }
        
        //**************************************************************************************
        
        //添加断水E3报警触发后，每天23点定时自动清除E3，清除效果与长按确认键效果一致
        //备注：关机状态下也需要自动清除E3
        if( BIT_GET(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER) )
        {
            
            #if TEST_AUTO_WASH
            //23:00 ~ 23:02 之间执行
            if( (23==calendar.hour) && (calendar.min>=0 && calendar.min<TEST_CYCLE_CHECK_TIME) )
            #else
            //23:00 ~ 23:10 之间执行
            if( (23==calendar.hour) && (calendar.min>=0 && calendar.min<CYCLE_CHECK_TIME) )
            #endif
            {
                BIT_CLEAR(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER);
            }
        }
        
        //**************************************************************************************
        
        if(OFF == air_clean_cfg.power_onoff)
        {
            #if TEST_AUTO_WASH
                //每隔2分钟检查一次
                OSTimeDlyHMSM(0,TEST_CYCLE_CHECK_TIME,0,0,OS_OPT_TIME_HMSM_STRICT,&err);
            #else
                OSTimeDlyHMSM(0,5,0,0,OS_OPT_TIME_HMSM_STRICT,&err);
            #endif
        }
        else        //电源键打开才作检测
        {
            //=====================================================================================
            
            wash_days = 0;
            atomizer_days = 0;
            
            wash_days = get_interval_days(WASH_INTERVAL_TYPE);
            WS_DEBUG("[CycleCheckTask] wash_days = %d ...\r\n", wash_days);
            
            //手动模式清洗
            if(MANUAL_WASH_MODE == air_clean_cfg.wash_mode)
            {
                if(wash_days >= WASH_FORCE_DAY)
                {
                    //手动模式达到强制清洗执行时间: 强制设备停止工作(取消告警)
                    st_current_status.manual_wash_stopdev = 1;
                    BIT_SET(air_clean_cfg.err_code_bitmap, FORCE_WASH_STOP_DEV);
                    write_cfg_to_flash();   //E10 需记录 Flash
                }
                else//wash_days < WASH_FORCE_DAY
                {
                    //未达到强制清洗执行时间
                    st_current_status.manual_wash_stopdev = 0;
                    BIT_CLEAR(air_clean_cfg.err_code_bitmap, FORCE_WASH_STOP_DEV);
                }
                
            }
            //自动模式清洗
            else if(AUTO_WASH_MODE == air_clean_cfg.wash_mode)
            {
                //清洗时间改为14天, 第15天的凌晨2点
                if( (wash_days >= (WASH_FORCE_DAY+1)) && (WASH_FORCE_HOUR == calendar.hour) )
                {
                    //强制清洗在 23:00 左右执行(这里修改为 10 分钟检测一次)
                    st_current_status.current_wash_status = 1;
                    #if MANUAL_EDITION
                    st_current_status.manual_need_auto_wash = 1;
                    #endif
                }
                
                if(1 == st_current_status.auto_wash_prompt)
                {
                    //自动清洗完成标志置为1时
                    st_current_status.auto_wash_past_hours ++;
                    if(st_current_status.auto_wash_past_hours >= 24)
                    {
                        st_current_status.auto_wash_prompt = 0;
                    }
                }
            }
            
            //=====================================================================================
            
            //雾化器只有在未锁定情形下, 才做自动切换检测
            if(0 == air_clean_cfg.atomizer_lock)
            {
                atomizer_days = get_interval_days(ATOMIZER_INTERVAL_TYPE);
                WS_DEBUG("[CycleCheckTask] atomizer_days = %d ...\r\n", atomizer_days);
                
                if(atomizer_days >= ATOMIZER_SWITCH_DAY)
                {
                    //这里相当于按键"雾化器切换", 在 AirCleanTask 中处理
                    if(air_clean_cfg.which_atomizer == ATOMIZER_ONE)
                        air_clean_cfg.which_atomizer = ATOMIZER_TWO;
                    else
                        air_clean_cfg.which_atomizer = ATOMIZER_ONE;
                    
                    //更新雾化器使用起始时间
                    update_cycle_time(ATOMIZER_INTERVAL_TYPE);
                    
                    st_current_status.siwtch_atomizer_flag = ON;
                    
                    
                    
                    //雾化器切换之后, 写入Flash
                    //write_cfg_to_flash(); 
                    
                    /*
                    WS_DEBUG("\r\n***********************************************\r\n");
                    WS_DEBUG("***********************************************\r\n");
                    WS_DEBUG("##### atomizer_days >= ATOMIZER_SWITCH_DAY#####\r\n");
                    WS_DEBUG("ATOMIZER_SWITCH_DAY --> calendar.w_year = %d \r\n",calendar.w_year);
                    WS_DEBUG("ATOMIZER_SWITCH_DAY --> calendar.w_month = %d \r\n",calendar.w_month);
                    WS_DEBUG("ATOMIZER_SWITCH_DAY --> calendar.w_date = %d \r\n",calendar.w_date);
                    WS_DEBUG("ATOMIZER_SWITCH_DAY --> calendar.hour = %d \r\n",calendar.hour);
                    WS_DEBUG("ATOMIZER_SWITCH_DAY --> calendar.min = %d \r\n",calendar.min);
                    WS_DEBUG("ATOMIZER_SWITCH_DAY --> calendar.sec = %d \r\n",calendar.sec);
                    WS_DEBUG("ATOMIZER_SWITCH_DAY --> calendar.week = %d \r\n",calendar.week);
                    WS_DEBUG("***********************************************\r\n");
                    WS_DEBUG("***********************************************\r\n\r\n");
                    */
                    
                    //WS_DEBUG("1111111111111111111111111111111111111111111111111111111\r\n\r\n");
                }
            }
            //WS_DEBUG("22222222222222222222222222222222222222222222222222222222\r\n\r\n");
            //=====================================================================================
            
            #if TEST_AUTO_WASH
                //每隔2分钟检查一次
                OSTimeDlyHMSM(0,TEST_CYCLE_CHECK_TIME,0,0,OS_OPT_TIME_HMSM_STRICT,&err);
            #else
                //每隔一小时检查一次
                //OSTimeDlyHMSM(1,0,0,0,OS_OPT_TIME_HMSM_STRICT,&err);
                
                //1 小时间隔太长, 这里修改为 10 分钟
                OSTimeDlyHMSM(0,CYCLE_CHECK_TIME,0,0,OS_OPT_TIME_HMSM_STRICT,&err);
            #endif
            
            //WS_DEBUG("333333333333333333333333333333333333333333333333333333333333333\r\n\r\n");
        }
    }
}


/*
*********************************************************************************************************
*                                           WIFI NET TASK
*********************************************************************************************************
*/
//网络数据处理任务
void WifiNetHandleTask(void * p_arg) 
{
    OS_ERR err;
    uint8_t i;
    char num[4]={0x55,0x00,0x01,0xFE};

    //TriggleWifi();
    OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
    //WS_DEBUG("beginaaaa get wifi Status\r\n");

    RefreshWifiStatus(0x04);
    while(1)
    {
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
        if(idleFlag==1)
        {
            idleFlag = 0;
            /*
            WS_DEBUG("\r\n HHK --> In this time, %d receive data are: ",rxCounter);
            for(i=0;i<rxCounter;i++)
            {
                WS_DEBUG("0x%x ",rxBuffer[i]);
            }
            WS_DEBUG("check rxBuffer jiaoyan is 0x%x, rxBuffer[rxCounter-1] is 0x%x, rxBuffer3 is 0x%x\r\n",gen_verify_code(rxBuffer,rxCounter-1),rxBuffer[rxCounter-1],rxBuffer[3]);
            */
            
            if( (rxBuffer[0]==0x55) && (gen_verify_code(rxBuffer,rxCounter-1) == rxBuffer[rxCounter-1]))
            {
                switch(rxBuffer[3])
                {
                    case 0x01:
                        SetupDevice();
                        break;
                    case 0x02:
                        UploadDeviceInfo(0x02);
                        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
                        UploadFaultInfo();
                        break;
                    case 0x04:
                        CustomizeSetup();
                        break;
                    case 0x05:
                        CustomizeQuery();
                        break;
                    case 0xFF:
                        RefreshWifiStatus(rxBuffer[4]);
                        break;
                    default:
                        break;
                }
            }
            //WS_DEBUG("\r\n");
            
            rxCounter = 0;
            
        }
    }

    WS_DEBUG("End\r\n");

    
    /*
    if(5 == st_current_status.nh3_err_flag)
    {
        //连续 5 次检测到氨气传感器故障, 上传"风扇故障"至服务器
    }
    
    */
    
//---------------------------------------------------------------------------------------------------------
//                                           各模块测试函数
//---------------------------------------------------------------------------------------------------------
    /* OK 
    //****************************** 测试"串口打印" ******************************
    OS_ERR err;
    (void)p_arg;
    OS_INFO("[Test Module Task] test debug USART start **********\n\r");
    
    while (DEF_TRUE) 
    {
        WS_DEBUG("[USART TEST] flag 001... \n\r");
        OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);   //延时10S
    }
    //****************************** 测试"串口打印" ******************************
    */
    

    /* OK
    //****************************** 测试"内部Flash" ******************************
    OS_ERR err;
    (void)p_arg;
    OS_INFO("[Test Module Task] test internal Flash start **********\n\r");
    
    while (DEF_TRUE) 
    {
        //先测试 tets_flash_func, 再测试 test_flash
        //tets_flash_func();        //bsp_internal_flash.c 中定义测试函数
        test_flash();               //appfunc.c 中定义测试函数
        OSTimeDlyHMSM(0,1,0,0,OS_OPT_TIME_HMSM_STRICT,&err);   //延时1min
    }
    //****************************** 测试"内部Flash" ******************************
    */
    

    // OK 
    //****************************** 测试"外置RTC" ******************************
    /*
    //1.测试 "外置 RTC" 模块的相关引脚 
    OS_ERR err;  
    (void)p_arg;
    
    OS_INFO("[Test Module Task] Test extern RTC Pin Start **********\n\r");
    while(1)
    {
            WS_DEBUG("flag 001 \r\n");
            macI2C_RTC_SCL_0();
            macI2C_RTC_SDA_0();
            OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时 1 S 
            
            WS_DEBUG("11111111111 \r\n");
            macI2C_RTC_SCL_1();
            macI2C_RTC_SDA_1();
            OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时 1 S 
    }
    */
    //**************************************************************************
    /*
    //2.测试 "外置 RTC" 模块的功能 
    OS_ERR err;  
    (void)p_arg;
    
    OS_INFO("[Test Module Task] Test extern RTC Function start **********\n\r");
    DS3231_Init();
    
    get_show_time();
    WS_DEBUG("******************************************** \r\n");
    WS_DEBUG("calendar.w_year = %d \r\n",calendar.w_year);
    WS_DEBUG("calendar.w_month = %d \r\n",calendar.w_month);
    WS_DEBUG("calendar.w_date = %d \r\n",calendar.w_date);
    WS_DEBUG("calendar.hour = %d \r\n",calendar.hour);
    WS_DEBUG("calendar.min = %d \r\n",calendar.min);
    WS_DEBUG("calendar.sec = %d \r\n",calendar.sec);
    WS_DEBUG("calendar.week = %d \r\n",calendar.week);
    WS_DEBUG("******************************************** \r\n");
    
    //设置时间
    //calendar.w_year=17;  
    //calendar.w_month=9;  
    //calendar.w_date=22; 
    //calendar.hour=19;  
    //calendar.min=0;
    //calendar.sec=0;
    //calendar.week=5;
    //DS3231_Set(calendar.w_year,calendar.w_month,calendar.w_date,
    //        calendar.week,calendar.hour,calendar.min,calendar.sec);
    while(1)
    {
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); 
        WS_DEBUG("[RTC TEST] flag 001... \r\n");
        get_show_time();
        
        WS_DEBUG("calendar.w_year = %d \r\n",calendar.w_year);
        WS_DEBUG("calendar.w_month = %d \r\n",calendar.w_month);
        WS_DEBUG("calendar.w_date = %d \r\n",calendar.w_date);
        WS_DEBUG("calendar.hour = %d \r\n",calendar.hour);
        WS_DEBUG("calendar.min = %d \r\n",calendar.min);
        WS_DEBUG("calendar.sec = %d \r\n",calendar.sec);
        WS_DEBUG("calendar.week = %d \r\n",calendar.week);
        
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时 1 S 
    }
    */
    //****************************** 测试"外置RTC" ******************************
    
    
    
    /*
    ///* OK
    //****************************** 测试"NH3传感器" ******************************
    int ret=0;
    int nh3_value=0;
    
    OS_ERR err;
    (void)p_arg;
    OS_INFO("[Test Module Task] Test NH3 sensor start **********\n\r");
    
    ret = NH3_Sensor_Init();
    WS_DEBUG("[Test Module Task] NH3_Sensor_Init return : %d\r\n",ret);
    OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);        //延时5s
    while (1) 
    {
        nh3_value = NH3_Sensor_Request();
        WS_DEBUG("[Test Module Task] nh3_value = %d \r\n",nh3_value);
        OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时5s
    }
    //****************************** 测试"NH3传感器" ******************************
    */
}

/*
*********************************************************************************************************
*                                           MANUAL AIR CLEAN TASK
*********************************************************************************************************
*/
#if MANUAL_EDITION
void ManualAirCleanTask(void *p_arg)
{
    OS_ERR err;
    
    int stop_flag = 0;              //停止工作标识
    int loop_times = 0;             //循环计数
    
    int upload_count = 0;           //临时计数
    
    char timeout;                   //注水超时计数
    char timeout_flag = 0;          //超时标志
    
    char beep_alarm_count = 0;      //蜂鸣器报警计数
    char beep_alarm_finish = 0;     //蜂鸣器报警完成标识
    
    char org_low_flag = 0;          //原液箱是否为低液位标识
    
    (void)p_arg;
    
    WS_DEBUG("[ManualAirCleanTask] ManualAirCleanTask create success. \r\n");
    
    while(DEF_TRUE) 
    {
        /**********************************************************************
         * 当出现错误码时, 所有设备停止工作, 不再继续执行 (E7 除外)
         * 只有长按"确认"键才能消除错误码
         *********************************************************************/
        if( (ON == air_clean_cfg.power_onoff) && (0 != air_clean_cfg.err_code_bitmap) && \
                (get_err_num()!= ORG_MUST_ADD_PLANT) && (get_err_num()!= FORCE_WASH_STOP_DEV) &&
                (get_err_num()!= ERR_OVERFLOW_FIRST) )
        {
            stop_flag = 0;
            loop_times = 0;
            st_current_status.one_key_atomize_flag = 0;
            
            //出现错误码, 蜂鸣每隔2秒滴一声, 3分钟后停止, 所有报警需加此设置除E6
            if(0 == beep_alarm_finish)
            {
                macBEEP_ON();
                OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err);
                macBEEP_OFF();
                
                beep_alarm_count ++;
                if(90 == beep_alarm_count)      //蜂鸣器响 3 分钟 (3*60/2=90)
                {
                    beep_alarm_count = 0;
                    beep_alarm_finish = 1;
                }
            }
            
            OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时2秒 
            
            #if NETWORK_VERSION
            //单机版本未注释掉的时候, 导致程序死机
            //这里 30 秒上传一次
            upload_count ++;
            if(upload_count >= 15)
            {
                upload_count = 0;
                UploadFaultInfo();
            }
            #endif
            
            continue;
        }
        else
        {
            //清除蜂鸣器报警完成标识
            if(1 == beep_alarm_finish)
            {
                beep_alarm_finish = 0;
            }
        }
        
        /**********************************************************************
         * 手动清洗模式下: 达到清洗时间(5周)还未清洗, 设备暂停工作
         *********************************************************************/
        //手动版本的清洗需要液箱有液, 因此在强制停止工作期间要能够执行加液
        if((ON == air_clean_cfg.power_onoff) && (1 == st_current_status.manual_wash_stopdev))
        {
            //if(1 == is_running)
            //{
                stop_flag = 0;
                loop_times = 0;
                st_current_status.one_key_atomize_flag = 0;
                
                dev_stop_work();
            //}
            
            //这里是手动版特有的, 需要能执行加液
            if(1 == st_current_status.add_plant_liquid_flag)
            {
                manual_add_plant_liquid();
            }
            
            OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时2秒 
            
            if(1 == st_current_status.current_wash_status)
            {
                WS_DEBUG("[ManualAirCleanTask] MANUAL_WASH_MODE: do_current_wash .... \r\n");
                //注意: 手动版执行清洗前需要保证原液箱有液
                //manual_do_wash();
                do_current_wash();
            }
            
            if(ON == st_current_status.onekey_drain_status)
            {
                do_onekey_drain();
            }
            
            continue;
        }
        
        /**********************************************************************
         * 原液箱和工作仓都为空, 不设置故障码, 显示屏显示植物液为空即可
         *********************************************************************/
        //无需考虑水位波动问题，液箱至最低液位，再补液2次，无需写入FLASH
        if((ON == air_clean_cfg.power_onoff)
            && (OFF == st_current_status.current_wash_status)
            && (OFF == st_current_status.onekey_drain_status)
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW))
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW))
            //"原液箱到低液位后抽液到工作仓的次数减为0"
            && (air_clean_cfg.org_low_to_work_count <= 0))
        {
            //WS_DEBUG("[ManualAirCleanTask] LIQUID_LEVEL_SENSOR_ORG_LOW && LIQUID_LEVEL_SENSOR_WORK_LOW ...\r\n");
            stop_flag = 0;
            loop_times = 0;
            st_current_status.one_key_atomize_flag = 0;
            
            dev_stop_work();
            
            st_current_status.current_plant_liquid = PLANT_LIQUID_LOW;
            if(0 == air_clean_cfg.org_low_to_work_count)
            {
                if(BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT) != 1)
                {
                    BIT_SET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT);      //设置必须加液的错误码 E7
                    write_cfg_to_flash();   //E7 需记录 Flash
                }
            }
            
            //点击了加液键
            if((ON == air_clean_cfg.power_onoff)
                    && (1 == st_current_status.add_plant_liquid_flag))
            {
                manual_add_plant_liquid();
            }
            
            OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时2秒 
            continue;
        }
        
        /**********************************************************************
         * 原液箱液位相关操作(高/中/低 分别更新为相应的状态)
         *********************************************************************/
        //原液箱低液位时更新状态
        if((ON == air_clean_cfg.power_onoff)
            && (OFF == st_current_status.current_wash_status)
            && (OFF == st_current_status.onekey_drain_status)
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW))) 
        {
            //WS_DEBUG("[ManualAirCleanTask] LIQUID_LEVEL_SENSOR_ORG_LOW ...\r\n");
            st_current_status.current_plant_liquid = PLANT_LIQUID_LOW;
            //WS_DEBUG("air_clean_cfg.org_low_to_work_count = %d \r\n", air_clean_cfg.org_low_to_work_count);
            if(0 == air_clean_cfg.org_low_to_work_count)
            {
                if(BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT) != 1)
                {
                    BIT_SET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT);      //设置必须加液的错误码 E7
                    write_cfg_to_flash();   //E7 需记录 Flash
                }
            }
        }
        //原液箱中液位时更新状态
        else if((ON == air_clean_cfg.power_onoff)
            && (OFF == st_current_status.current_wash_status)
            && (OFF == st_current_status.onekey_drain_status)
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_MID))) 
        {
            //WS_DEBUG("[ManualAirCleanTask] LIQUID_LEVEL_SENSOR_ORG_MID ...\r\n");
            st_current_status.current_plant_liquid = PLANT_LIQUID_MID;
            //更新"原液箱到低液位后还能抽液到工作仓的次数"
            //update_org_low_to_work_count();           //自动版本更新
            air_clean_cfg.org_low_to_work_count = MANUAL_LOW_ORG_TO_WORK_COUNT; //手动版本固定次数更新为 MANUAL_LOW_ORG_TO_WORK_COUNT
            //清除必须加液的错误码 E7
            BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);
        }
        //原液箱高液位时更新状态
        else if((ON == air_clean_cfg.power_onoff)
            && (OFF == st_current_status.current_wash_status)
            && (OFF == st_current_status.onekey_drain_status)
            && (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_HIGH)))
        {
            //WS_DEBUG("[ManualAirCleanTask] LIQUID_LEVEL_SENSOR_ORG_HIGH ...\r\n");
            st_current_status.current_plant_liquid = PLANT_LIQUID_HIGH;
            //更新"原液箱到低液位后还能抽液到工作仓的次数"
            //update_org_low_to_work_count();           //自动版本更新
            air_clean_cfg.org_low_to_work_count = MANUAL_LOW_ORG_TO_WORK_COUNT; //手动版本固定次数更新为 MANUAL_LOW_ORG_TO_WORK_COUNT
            //清除必须加液的错误码 E7
            BIT_CLEAR(air_clean_cfg.err_code_bitmap, ORG_MUST_ADD_PLANT);
        }

        /**********************************************************************
         * 工作仓液位相关操作(这里只针对低液位做自动配比操作)
         *********************************************************************/
        /**********************************************************************
         * 手动版配比条件:
         *      1.工作仓低液位, 
         *      2.原液箱不为低液位或者液箱至最低液位补液次数还没有达到2次
         * 手动版配比过程:
         *      从液箱补液至工作仓无需打开电磁阀（不装进水电磁阀），把补液泵的
         *      抽液时间由原来的1、1.2、1.5秒统一为保护时间32秒
         *********************************************************************/
        if((ON == air_clean_cfg.power_onoff) && 
            (OFF == st_current_status.current_wash_status) && 
            (OFF == st_current_status.onekey_drain_status) && 
            (OFF == st_current_status.add_plant_liquid_flag) && 
            (PULLDOWN == get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW)) &&    //工作仓低液位
            ((get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) != PULLDOWN) &&    //原液箱不为低液位
                    || (air_clean_cfg.org_low_to_work_count > 0))       //液箱至最低液位补液次数还没有达到指定次数(1次)
            )
        {
            WS_DEBUG("++++++++++++++++++++++++ work auto +++++++++++++++++++++++ \r\n");
            
            //原液箱为低液位设置标志, 用作是否更新 air_clean_cfg.org_low_to_work_count 的依据
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) == PULLDOWN)
            {
                org_low_flag = 1;
            }
            else
            {
                org_low_flag = 0;
            }

            //Step 1. 关闭所有设备
            stop_flag = 0;
            loop_times = 0;
            st_current_status.one_key_atomize_flag = 0;
            
            dev_stop_work();
            
            //Step 2. 启动液泵2(补液泵)加液到最高液位, 保护时间为32秒
            
            timeout = 0;
            timeout_flag = 0;
            liquid_pump_onoff(PUMP_TO_WORK, ON);        //液泵2开始工作进行配比
            while(1)
            {
                if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) == PULLDOWN)
                {
                    break;
                }
                
                //关闭电源/执行清洗/添加原液, 则停止配比
                if((OFF == air_clean_cfg.power_onoff) ||
                    (ON == st_current_status.current_wash_status) ||
                    (ON == st_current_status.onekey_drain_status) || 
                    (ON == st_current_status.add_plant_liquid_flag)) 
                {
                    break;
                }
                
                OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时1秒 
                
                timeout++;
                if(timeout >= MANUAL_ORG_TO_WORK_SAFE_TIME)
                {
                    timeout_flag = 1;
                    break;
                }
            }

            liquid_pump_onoff(PUMP_TO_WORK, OFF);       //液泵2停止工作
            
            if(timeout_flag)
            {
                if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_HIGH) != PULLDOWN){
                    BIT_SET(air_clean_cfg.err_code_bitmap,ERR_WORK_SUPPLY_WATER);
                    WS_DEBUG("Set ERR_WORK_SUPPLY_WATER Error Code --> Func:%s @Line:%d\r\n",__func__,__LINE__);
                }
            }
            
            //Step 3. 若原液箱为低液位, 则补液成功后更新 air_clean_cfg.org_low_to_work_count
            if(1 == org_low_flag)       //原液箱低液位
            {
                //这里可能存在BUG, 液位波动, 是否需要工作仓延时工作一段时间在执行加液
                //如何判断补液成功？？？
                if((get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW) != PULLDOWN))
                {
                    air_clean_cfg.org_low_to_work_count -= 1;
                    if(0 == air_clean_cfg.org_low_to_work_count)
                    {
                        if(BIT_GET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT) != 1)
                        {
                            BIT_SET(air_clean_cfg.err_code_bitmap,ORG_MUST_ADD_PLANT);      //设置必须加液的错误码 E7
                            write_cfg_to_flash();   //E7 需记录 Flash
                        }
                    }
                }
            }
            
            WS_DEBUG("[ManualAirCleanTask] air_clean_cfg.org_low_to_work_count = %d \r\n", air_clean_cfg.org_low_to_work_count);
        }
        
        /**********************************************************************
         * 错误码相关操作
         *********************************************************************/
        if((ERR_NONE != air_clean_cfg.err_code_bitmap) && \
                 && (get_err_num()!= ORG_MUST_ADD_PLANT) && (get_err_num()!= FORCE_WASH_STOP_DEV) )
        {
            //把故障码通知给服务器，
            //把优先级最高的故障码显示在屏幕上。
        }
        
        /**********************************************************************
         * 处理遥控器按键相关操作
         *********************************************************************/
        //处理"A/B切换"
        if(ON == st_current_status.siwtch_atomizer_flag)
        {
            if(is_running == ON) 
            {
                if(air_clean_cfg.which_atomizer == ATOMIZER_ONE) 
                {
                    atomizer_onoff(ATOMIZER_TWO,OFF);
                    atomizer_onoff(ATOMIZER_ONE,ON);
                } 
                else 
                {
                    atomizer_onoff(ATOMIZER_ONE,OFF);
                    atomizer_onoff(ATOMIZER_TWO,ON);
                }
                
                //更新雾化器使用起始时间
                //update_cycle_time(ATOMIZER_INTERVAL_TYPE);
            }
            st_current_status.siwtch_atomizer_flag = OFF;
            
            //这里清空为 0
            loop_times=0;
            st_current_status.one_key_atomize_flag = 0;
        }
        
        
        //处理 "风量调节" 按键
        if(st_current_status.siwtch_fan_level_flag == ON) 
        {
            if(is_running == ON) 
            { 
                //WS_DEBUG("[ManualAirCleanTask] do set_air_fan_level func ...\r\n");
                set_air_fan_level();
            }
            st_current_status.siwtch_fan_level_flag = OFF;
            //WS_DEBUG("[ManualAirCleanTask] st_current_status.siwtch_fan_level_flag = %d \r\n",st_current_status.siwtch_fan_level_flag);
        }
        
        //关闭电源/执行清洗/添加原液: 必须关闭所有设备
        if((OFF == air_clean_cfg.power_onoff) || 
                (ON == st_current_status.current_wash_status) || 
                (ON == st_current_status.onekey_drain_status) || 
                (ON == st_current_status.add_plant_liquid_flag))
        {
            stop_flag = 0;
            loop_times = 0;
            st_current_status.one_key_atomize_flag = 0;
            
            dev_stop_work();
        }
        
        //执行清洗
        if((ON == air_clean_cfg.power_onoff)
            && (ON == st_current_status.current_wash_status))
        {
            WS_DEBUG("[ManualAirCleanTask] do_current_wash func start ...\r\n");
            
            //manual_do_wash();
            do_current_wash();
            
            WS_DEBUG("[ManualAirCleanTask] do_current_wash func finish ...\r\n");
        }
        
        //执行一键排液
        if((ON == air_clean_cfg.power_onoff)
            && (ON == st_current_status.onekey_drain_status))
        {
            //manual_do_wash();
            do_onekey_drain();
        }
        
        //添加原液
        if((ON == air_clean_cfg.power_onoff)
                && (ON == st_current_status.add_plant_liquid_flag))
        {
            manual_add_plant_liquid();
        }
        
        if(1 == st_setting_status.mt_setting_finished)
        {
            //设置时间/模式后, 不在工作时间范围内, 不考虑设备当前状态, 停止设备运行
            if(!get_whole_dev_onoff())
            {
                stop_flag = 0;
                loop_times = 0;
                dev_stop_work();
            }
            //设置时间/模式后, 在工作时间范围内:
            //1.若设备当前状态正在运行, 继续设备运行, 重新计时
            //2.若设备当前状态不在运行, 需要开启设备运行
            else
            {
                stop_flag = 0;
                loop_times = 0;
            }
            
            st_current_status.one_key_atomize_flag = 0;
            st_setting_status.mt_setting_finished = 0;
        }
        
        /**********************************************************************
         * 所有故障解除可以正常工作
         *********************************************************************/
        if((ON == air_clean_cfg.power_onoff)
            && (ERR_NONE == air_clean_cfg.err_code_bitmap)
            && (OFF == st_current_status.current_wash_status)
            && (OFF == st_current_status.onekey_drain_status)
            && (PULLDOWN != get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_LOW)))
        {
            //不在工作时间范围内
            if(!get_whole_dev_onoff())
            {
                //一键雾化标识为 1, 则执行出雾两分钟
                if(1 == st_current_status.one_key_atomize_flag)
                {
                    if(0 == is_running)     //设备开始工作的时候, loop_times 清零
                        loop_times = 0;
                        
                     atomizer_start_work();
                }
                else
                {
                    if(1 == is_running)
                    {
                        stop_flag = 0;
                        loop_times = 0;
                        st_current_status.one_key_atomize_flag = 0;
                        
                        dev_stop_work();
                    }
                }
            }
            else
            {
                st_current_status.one_key_atomize_flag = 0;
            
                if(stop_flag == 0)
                {
                    //WS_DEBUG("[ManualAirCleanTask] 00000 Everything is OK dev_start_work \r\n");
                    dev_start_work();
                }
                
                //设备正在工作, 检查是否在运行时间段
                if((1 == is_running) && (!get_whole_dev_onoff()))
                {
                    stop_flag = 0;
                    loop_times = 0;
                    dev_stop_work();
                }
            }
        }
        
        loop_times++;
        
        if((ON == air_clean_cfg.power_onoff)
                && (ERR_NONE == air_clean_cfg.err_code_bitmap))
        {
            //*****************************************************************
            //一键出雾功能
            if(1 == st_current_status.one_key_atomize_flag)
            {
                if( (1 == is_running) && (loop_times >= 30*2) )     //出雾两分钟
                {
                    loop_times = 0;
                    st_current_status.one_key_atomize_flag = 0;
                    
                    dev_stop_work();
                }
            }
            
            //非一键出雾功能
            else
            {
                //自定义模式
                if(MODE_CUSTOM == air_clean_cfg.work_mode)
                {
                    if( (1 == is_running) && (loop_times >= 30*(air_clean_cfg.st_custom_mode.custom_open_min)) )
                    {
                        //WS_DEBUG("[ManualAirCleanTask->MODE_CUSTOM]loop_times = %d \r\n", loop_times);
                        
                        stop_flag = 1;
                        loop_times = 0;
                        
                        //WS_DEBUG("[ManualAirCleanTask->MODE_CUSTOM]loop_times dev_stop_work \r\n");
                        dev_stop_work();
                    }
                    else if( (0 == is_running) && (loop_times >= 30*(air_clean_cfg.st_custom_mode.custom_close_min)) )
                    {
                        stop_flag = 0;
                        loop_times = 0;
                        
                        //WS_DEBUG("[ManualAirCleanTask->MODE_CUSTOM] 00001 loop_times dev_start_work \r\n");
                        
                        dev_start_work();
                    }
                }
                //本地模式(标准模式)
                else
                {
                    if( (1 == is_running) && (loop_times >= 30*(air_clean_cfg.standard_mode_open_min)) )
                    {
                        //WS_DEBUG("[ManualAirCleanTask->MODE_LOCAL]loop_times = %d \r\n", loop_times);
                        
                        stop_flag = 1;
                        loop_times = 0;
                        
                        //WS_DEBUG("[ManualAirCleanTask->MODE_LOCAL]loop_times dev_stop_work \r\n");
                        dev_stop_work();
                    }
                    else if( (0 == is_running) && (loop_times >= 30*(air_clean_cfg.standard_mode_close_min)) )
                    {
                        stop_flag = 0;
                        loop_times = 0;
                        
                        //WS_DEBUG("[ManualAirCleanTask->MODE_LOCAL] 00002 loop_times dev_start_work \r\n");
                        
                        dev_start_work();
                    }
                }
            }
        }
        
        OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);    //延时2秒 
    }
}
#endif





void DetectOverflowTask(void * p_arg)
{
    OS_ERR err;
    (void)p_arg;
    
    WS_DEBUG("[DetectOverflowTask] DetectOverflowTask create success. \r\n");
    
    while(1) 
    {
        //WS_DEBUG("[DetectOverflowTask] +++++++++++++++++++++++++++++. \r\n");
        //WS_DEBUG("[DetectOverflowTask] DetectOverflowTask flag print. \r\n");
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
        
        /*
          检测到溢出液位:
            1.关机不断电状态检测到溢出液位, 出水电磁阀打开
        */
        if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_WORK_OVERFLOW) == PULLDOWN)
        {
            
            WS_DEBUG("+++++++++++++++ 检测到溢出液位 +++++++++++++++\r\n");
            
            //1.关机不断电状态触发溢出液位, 出水电磁阀需打开
            if(OFF == air_clean_cfg.power_onoff)
            {
                WS_DEBUG("[DetectOverflowTask] 关机不断电状态下检测到溢出液位\r\n");
                st_current_status.overflow_drain_liquid_status = 1;
                overflow_setting();
                
                
                WS_DEBUG("999999999999999999999999999999999\r\n");
                overflow_drain_liquid();
                //注意这里与开机状态下排液的不同之处
            }
            
            else
            {
                //st_current_status.add_plant_liquid_flag = 0;
                //st_current_status.current_wash_status = OFF;
                //st_current_status.onekey_drain_status = OFF;
                
                
                //2.开机状态下出现错误码标识(此时设备不工作)
                if(1 == st_current_status.poweron_error_code_flag)
                {
                    WS_DEBUG("[DetectOverflowTask] 开机状态下出现错误码时检测到溢出液位，并执行 overflow_setting\r\n");
                    
                    st_current_status.overflow_drain_liquid_status = 1;
                    //出现错误码, 但不是错误码E12导致的设备工作, 需进行相关设置:
                    //   1.没有设置 E11, 设置错误码 E11, 并记录工作仓第一次溢出时间点
                    //   2.已经设置 E11, 时间间隔超过1小时, 重新记录溢出的时间, 否则设置错误码 E12
                    if( BIT_GET(air_clean_cfg.err_code_bitmap,ERR_OVERFLOW_MANY_TIMES) != 1 )
                    {
                        overflow_setting();
                    }
                    WS_DEBUG("rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\r\n");
                    overflow_drain_liquid();
                }
                
                //3.手动清洗模式下达到清洗时间(5周)还未清洗, 设备暂停工作
                //强制清洗设备停止过程中，可以执行"加液"和"清洗"功能


                
                //加液过程中检测到溢出液位：停止加液-->延时1秒-->排液
                else if(1 == st_current_status.add_plant_liquid_flag)
                {
                    WS_DEBUG("[DetectOverflowTask] 加液过程中检测到溢出液位, 并执行 overflow_setting \r\n");
                    
                    //清除加液标识add_plant_liquid_flag，在函数add_plant_liquid中将停止加液
                    st_current_status.overflow_drain_liquid_status = 1;
                    st_current_status.add_plant_liquid_flag = 0;    //停止加液
                    OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
                    
                    overflow_setting();
                    WS_DEBUG("7777777777777777777777777777\r\n");
                    overflow_drain_liquid();
                }
                
                
                //注意!!!!
                /*在清洗和一键排液的过程中(这里举例一键排液)：
                    当检测到溢出液位，先将 st_current_status.onekey_drain_status 置为 OFF，然后延时2秒，
                    这时在主任务中 AirCleanTask 中检测到达到工作的条件，打开了雾化器工作，因此这里先将 
                    st_current_status.overflow_drain_liquid_status 置为ON，保证了不能达到正常工作的条件
                日志：
                所有故障解除可以正常工作, 应该可以正常工作
                正在工作时间范围内 
                设备开始工作 
                [KeyControlTask func] Remote_Scan return key value = 0xe2       //执行一键清洗
                [AirCleanTask] do_onekey_drain func start ...
                [do_onekey_drain] 执行 onekey_drain_liquid  
                +++++++++ [onekey_drain_liquid] 开始执行 onekey_drain_liquid 函数 
                +++++++++++++++ 检测到溢出液位 +++++++++++++++
                [DetectOverflowTask] 清洗过程中检测到溢出液位                   //DetectOverflowTask 函数此时睡眠两秒
                [onekey_drain_liquid] 001 电源关闭 return                       //一键清洗函数停止
                [do_onekey_drain] onekey_drain_liquid 函数返回非0 
                [AirCleanTask] do_onekey_drain func finish ...
                所有故障解除可以正常工作, 应该可以正常工作                      //由于onekey_drain_status在DetectOverflowTask被置为OFF,这里又达到工作的条件了
                正在工作时间范围内 
                设备开始工作 
                [dev_start_work] 执行启动雾化器操作: atomizer_onoff ON 
                
                */
                //清洗过程中检测到溢出液位(1.正在进水; 2.正在排液)
                else if(ON == st_current_status.current_wash_status)
                {
                    WS_DEBUG("[DetectOverflowTask] 清洗过程中检测到溢出液位 \r\n");
                    
                    //无论是在进水还是排液，都将清洗停止，然后执行"溢出放液"操作
                    //优点：清洗过程中的放液可以由开关键停止，而溢出放液不受控制
                    st_current_status.overflow_drain_liquid_status = 1;
                    st_current_status.current_wash_status = OFF;
                    OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
                    
                    overflow_setting();
                    WS_DEBUG("66666666666666666666\r\n");
                    overflow_drain_liquid();
                }
                
                //一键排液过程中检测到溢出液位
                else if(ON == st_current_status.onekey_drain_status)
                {
                    WS_DEBUG("[DetectOverflowTask] 清洗过程中检测到溢出液位 \r\n");
                    
                    //无论是在进水还是排液，都将清洗停止，然后执行"溢出放液"操作
                    //优点：清洗过程中的放液可以由开关键停止，而溢出放液不受控制
                    st_current_status.overflow_drain_liquid_status = 1;
                    st_current_status.onekey_drain_status = OFF;
                    OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
                    
                    WS_DEBUG("000000000000000000000000000000000000000\r\n");
                    overflow_setting();
                    overflow_drain_liquid();
                }
                
                else if(ON == st_current_status.power_off_e7_maintance_status)
                {
                    WS_DEBUG("[DetectOverflowTask] 关机或E7状态保养清洗过程中检测到溢出液位 \r\n");
                    
                    //无论是在进水还是排液，都将清洗停止，然后执行"溢出放液"操作
                    //优点：清洗过程中的放液可以由开关键停止，而溢出放液不受控制
                    st_current_status.overflow_drain_liquid_status = 1;
                    st_current_status.power_off_e7_maintance_status = OFF;
                    OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
                    
                    WS_DEBUG("tttttttttttttttTTTTTTTTTTTTTTTTTTTTTTTTTT\r\n");
                    overflow_setting();
                    overflow_drain_liquid();
                }
                ///*
                //3.配比过程中触发溢出液位   ---   是否能包括 ？？？
                else
                {
                    if( (OFF == st_current_status.onekey_drain_status) &&
                            (OFF == st_current_status.current_wash_status) &&
                            (OFF == st_current_status.add_plant_liquid_flag) && 
                            (OFF == st_current_status.poweron_error_code_flag) &&
                            (OFF == st_current_status.power_off_e7_maintance_status)
                    )
                    {
                        st_current_status.overflow_drain_liquid_status = 1;
                        
                        overflow_setting();
                    
                        WS_DEBUG("88888888888888888888888888888888\r\n");
                        overflow_drain_liquid();
                    }

                }
                //*/
                
            }
            
        }
        

    }
}




/*
相关说明：
1.AirCleanTask 任务中出现故障码时，需要蜂鸣器立即响，则相关操作立即停止
    例如：配比过程中，加水到达溢出液位执行排液，不能在AirCleanTask中执行
          否则只有执行完排液，蜂鸣器才会响，应将排液功能放到单独任务中

2.自动配比流程：
    Step 1. 关闭所有设备  
    Step 2. 启动电磁阀1，加水到最高液位
        2-1.工作仓未到达最高液位点，设置E3进水故障
        2-2.加水过程中检测到溢出液位的处理：
            A.第一次检测到溢出液位，设置E11，DetectOverflowTask 任务中执行排液
                若正常排液，需要继续执行自动配比；若排液出现故障，设置E4，设备停止工作
            B.第二次检测到溢出液位，设置E12，DetectOverflowTask 任务中执行排液
                不论排液是否正常，设备都停止工作；
        //DetectOverflowTask 任务中执行排液过程中，不能执行自动配比
        //排液的优先级最高，因此在排液过程中添加标识是否执行完成
        
    Step 3. 启动液泵2进行抽液 (当加水到最高位正常执行后再抽取植物液到工作仓)

3.清洗流程：设备停止工作-->排液-->进水-->排液-->进水-->排液-->更新清洗时间
    清洗过程中只有：排液和进水两种操作
    3-1.清洗过程中正在"排液"检测到溢出液位
        已经正在排液了，可以不做处理
    3-2.清洗过程中正在"进水"检测到溢出液位
        停止清洗，DetectOverflowTask 任务中执行排液
        
        
执行排液的情形：
    1.清洗功能
        1-1.正常排液
        1-2.若进水过程中检测到溢出液位，清洗功能继续 OR 停止？
    
    2.一键排液功能
        若排液异常，一键排液功能作为执行失败
        
    3.检测到溢出液位
    
    //正常排液时，开关机可取消排液；溢出液位导致的排液，开关机无法取消
    
    

要求逻辑变更：
    1.排液过程中：
        原先：第一次排液故障设置E4，第二次排液成功清除E4 
        修改：两次排液都故障再设置E4，否则不设置E4 
    
    
    2.清洗过程中：检测到溢出液位，停止清洗，与原液箱加液保持一致     (确认通过)
    
    //这里建议修改: 正常配比后就消除E11   何必要等到第二次？？？
*/


/*
手动版(ManualAirCleanTask)说明:
    1.从外部把植物配比液收入液箱储存，我们称之为加液。从液箱抽液至工作仓，我们称之为补液。
        1-1.手动加液至液箱的时间是根据测算的，时间比自动少了是因为加液泵功率大了
            手动加液版本理解为：从外部加液至液箱与原程序一样，只是保护时间不一样
        1-2.从液箱补液至工作仓无需打开电磁阀（不装进水电磁阀），把补液泵的抽液时
            间由原来的1、1.2、1.5秒统一为保护时间32秒，同时浓度功能失效。
        1-3.功能屏浓度灯不亮并且浓度按键失效无声音
    2.手动与自动的区别就只有补液至工作仓的机制不同，自动款借助外部自来水，而手动款只抽取液
        箱的配比液，同时浓度功能失效。
    3.手动版本无需考虑水位波动问题，液箱至最低液位，再补液2次，无需写入FLASH。
    
手动版启动清洗程序:
    1.打开排液电磁阀，保护时间个机制和自动版一样，排液结束关闭排位电磁阀。
    2.启动补液泵，加液至最高点或32秒时间结束。
    3.打开排位电磁阀，排液完成关闭电磁阀。

*/

/*
自动版本(AirCleanTask)逻辑需求
    1.三档浓度隔膜泵抽植物液时间为最淡1:240浓度为0.7秒，中间1:180浓度为1秒，最浓1:120浓度为1.4秒。
    2.液箱液位传感器到达最低液位，不作为控制设备停止的条件，到达最低液位开始计数，最淡1:240浓度为450次，中间1:180浓度为330次，最浓1:120浓度为220次。
    3.当补液次数到达设定值次数，设备停止工作。
    4.当液箱加植物液并离开液箱液位传感器最低液位点，设备正常工作，补液计数失效。
*/