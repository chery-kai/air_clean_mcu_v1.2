#include "appwifi.h"
#include "appfunc.h"

/*
*********************************************************************************************************
*                                           FOR WIFI TASK
*********************************************************************************************************
*/
uint8_t rxBuffer[100];
uint8_t idleFlag=0;
uint8_t rxCounter=0;

uint8_t GetDeviceSwitch()
{
    if(0x00==air_clean_cfg.power_onoff )
        return 0x01;
    else 
        return 0x02;
    //return 0x01;
}

uint8_t GetWorkingMode()
{
    return air_clean_cfg.work_mode;
    // 0x04;
}

uint8_t GetAtomizerStatus()
{
    return air_clean_cfg.which_atomizer;
    //return 0x01;
}

uint8_t GetAirVolumeStatus()
{
    return air_clean_cfg.air_fan_level;
    //return 0x02;
}

uint8_t GetConcentrationStatus()
{
    return air_clean_cfg.liquid_density;
    //return 0x03;
}

uint8_t GetWashingStatus()
{
    return air_clean_cfg.wash_mode;
    //return 0x01;
}

uint8_t GetDeviceRunningStatus()
{
    //这里若错误码超过 E7, 可能会越界
    //E7用于显示特征告知客户何时加液, 这里没有另外处理
    return air_clean_cfg.err_code_bitmap;
    //return 0x02;
}

uint8_t GetNH3Value()
{
    return st_current_status.current_nh3_ppm;
    //return 0x15;
}

uint8_t GetPlantLiquidStatus()
{
    return st_current_status.current_plant_liquid+1;
    //return 0x01;
}

BOOL IsFanAAbnormal()
    {
    return BIT_GET(air_clean_cfg.err_code_bitmap, ERR_FAN_ONE_FAULT);
    //return True;
}

BOOL IsFanBAbnormal()
{
    return BIT_GET(air_clean_cfg.err_code_bitmap, ERR_FAN_TWO_FAULT);
    //return False;
}

BOOL IsAtomizerAAbnormal()
{
    return False;
}

BOOL IsAtomizerBAbnormal()
{
    return False;
}

BOOL IsPumpAAbnormal()
{
	return BIT_GET(air_clean_cfg.err_code_bitmap, ERR_ORG_ADD_LIQUID);
	//return True;
}

BOOL IsPumpBAbnormal()
{
	return BIT_GET(air_clean_cfg.err_code_bitmap, ERR_WORK_SUPPLY_WATER);
	//return True;
}

BOOL IsValveAAbnormal()
{
    return BIT_GET(air_clean_cfg.err_code_bitmap, ERR_WORK_SUPPLY_WATER);
    //return True;
}

BOOL IsValveBAbnormal()
{
    return BIT_GET(air_clean_cfg.err_code_bitmap, ERR_WORK_DRAIN_LIQUID);
    //return True;
}

BOOL IsNH3SensorAbnormal()
{
    return st_current_status.nh3_err_flag;
    //return True;
}

BOOL IsLiquidBoxAbnormal()
{
    return False;
}

BOOL IsWorkBoxAbnormal()
{
    return False;
}

uint8_t GetCustomMode()
{
    uint8_t i,value=0x00;
    for(i=1;i<8;i++)
    {
        if(air_clean_cfg.st_custom_mode.custom_work_week[i]==0x01)
            BIT_SET(value, i-1);
    }
    return value;
}

/*int GetCustomTime1()
{
    int value;
    value = air_clean_cfg.st_custom_mode.custom_time_plan[0].start_hour<<24 \
                    + air_clean_cfg.st_custom_mode.custom_time_plan[0].start_minute<<16 \
                    +air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_hour<<8 \
                    +air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_minute ;

    return value;
}*/

uint8_t GetAtomizerRunningTime()
{
    return air_clean_cfg.st_custom_mode.custom_open_min;
}

uint8_t GetAtomizerStopTime()
{
    return air_clean_cfg.st_custom_mode.custom_close_min;
}

unsigned char gen_verify_code(char *buf, int len)
{
    u8 c = 0x00;
    int i;

    for (i=0; i<len; i++)
    {
        c += *buf++;
    }

    return ~c;
}

uint8_t GetDeviceVerifyCode(DevInfo devInfo, int num)
{
    u8 c = 0x00;
    int i;

    c += devInfo.head;
    c += devInfo.data_length;
    c += devInfo.mode;
    
    for (i=0; i<num; i++)
    {
        c += devInfo.data_info[i];
    }

    return ~c;
}

uint8_t GetFaultVerifyCode(FaultInfo faultInfo, int num)
{
    u8 c = 0x00;
    int i;

    c += faultInfo.head;
    c += faultInfo.data_length;
    c += faultInfo.mode;

    for (i=0; i<num; i++)
    {
        c += faultInfo.data_info[i];
    }

    return ~c;
}


uint8_t GetWifiState()
{
    uint8_t wifiStateData[5] = {0x55,0x00,0x03,0xFF},i=0;
    OS_ERR err;
    wifiStateData[4] = gen_verify_code(wifiStateData,4);
    rxCounter = 0;
    Usart_SendByte(WIFI_USARTx,wifiStateData[0]);
    Usart_SendHalfWord(WIFI_USARTx,wifiStateData[2]);
    Usart_SendByte(WIFI_USARTx,wifiStateData[3]);
    Usart_SendByte(WIFI_USARTx,wifiStateData[4]);
    /*while(1)
    {
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
        WS_DEBUG("idle Flag is 0x%x\r\n",idleFlag);		
        if(2==idleFlag)
        {
            for(i=0;i<rxCounter;i++)
            {
                WS_DEBUG("0x%x ",rxBuffer[i]);
            }
            if((rxBuffer[0]==0x55) && (gen_verify_code(rxBuffer,rxCounter-1) == rxBuffer[rxCounter-1]))
            {
                return rxBuffer[4];
            }else
            {
                return 0x04;
            }
            idleFlag = 0;
            rxCounter = 0;
            break;
        }else if(1==idleFlag)
        {
            rxCounter = 0;
        }
    }*/
}

void SendDevInfo(DevInfo devInfo)
{
    uint8_t i,wifiState;
    /*wifiState = GetWifiState();
    WS_DEBUG("wifiState is 0x%x\r\n",wifiState);*/

    Usart_SendByte(WIFI_USARTx,devInfo.head);
    Usart_SendHalfWord(WIFI_USARTx,devInfo.data_length);
    Usart_SendByte(WIFI_USARTx,devInfo.mode);
    for(i=0;i<16;i++)
    {
        Usart_SendByte(WIFI_USARTx,devInfo.data_info[i]);
    }
    Usart_SendByte(WIFI_USARTx,devInfo.check_data);
}

void SendFaultInfo(FaultInfo faultInfo)
{
    uint8_t i;
    Usart_SendByte(WIFI_USARTx,faultInfo.head);
    Usart_SendHalfWord(WIFI_USARTx,faultInfo.data_length);
    Usart_SendByte(WIFI_USARTx,faultInfo.mode);
    for(i=0;i<11;i++)
    {
        Usart_SendByte(WIFI_USARTx,faultInfo.data_info[i]);
    }
    Usart_SendByte(WIFI_USARTx,faultInfo.check_data);
}

void UploadFaultInfo()
{
    uint8_t i;
    FaultInfo faultInfo;
    faultInfo.head = 0x55;
    faultInfo.data_length = 0x0E;
    faultInfo.mode = 0xFD;
    faultInfo.data_info[0] = IsFanAAbnormal();
    faultInfo.data_info[1] = IsFanBAbnormal();
    faultInfo.data_info[2] = IsAtomizerAAbnormal();
    faultInfo.data_info[3] = IsAtomizerBAbnormal();
    faultInfo.data_info[4] = IsPumpAAbnormal();
    faultInfo.data_info[5] = IsPumpBAbnormal();
    faultInfo.data_info[6] = IsValveAAbnormal();
    faultInfo.data_info[7] = IsValveBAbnormal();
    faultInfo.data_info[8] = IsNH3SensorAbnormal();
    faultInfo.data_info[9] = IsLiquidBoxAbnormal();
    faultInfo.data_info[10] = IsWorkBoxAbnormal();
    faultInfo.check_data = GetFaultVerifyCode(faultInfo,11);

    /*
    WS_DEBUG("Fault Info is:");
    for(i=0;i<11;i++)
        WS_DEBUG("0x%x ",faultInfo.data_info[i]);
    WS_DEBUG("\r\nfaultInfo check_data is 0x%x\r\n",faultInfo.check_data);
    */
    
    SendFaultInfo(faultInfo);
}

void TriggleWifi()
{
    OS_ERR err;
    char num[4]={0x55,0x00,0x03,0xFE};

    GPIO_SetBits(GPIOE, GPIO_Pin_12);

    OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err); 

    GPIO_ResetBits(GPIOE, GPIO_Pin_12);

    //WS_DEBUG("HHK --> TriggleWifi ... Usart_SendByte begin \r\n");
    
    USART_Config();
    Usart_SendByte(WIFI_USARTx,num[0]);
    Usart_SendHalfWord(WIFI_USARTx,num[2]);
    Usart_SendByte(WIFI_USARTx,num[3]);
    Usart_SendByte(WIFI_USARTx,gen_verify_code(num,4));
    
    //WS_DEBUG("HHK --> TriggleWifi ... Usart_SendByte end \r\n");

    WS_DEBUG("EndIO2 get wifi Status\r\n");
}


/*void UploadParaToServer()
{
    uint8_t i;
    uint8_t sendData[20]={0x55,0x00,0x13,0x02,0x01,0x04,0x01,0x02,0x03,0x01,0x02,0x15,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
                            
    for(i=0;i<20;i++)
    {
        Usart_SendByte(WIFI_USARTx,sendData[i]);
    }
    Usart_SendByte(WIFI_USARTx,gen_verify_code(sendData,20));
    WS_DEBUG("UploadParaToServer 0x%x\r\n",gen_verify_code(sendData,20));
}*/


void SetupDevice()
{
    switch(rxBuffer[4])
    {
        case 0x00:
            WS_DEBUG("Device Status: Keep in current status\r\n");
            break;
        case 0x01:
            air_clean_cfg.power_onoff = 0x01;
            /*write_cfg_to_flash(); 
            
            st_setting_status.setting_time_flag = 0;
            st_setting_status.setting_mode_flag = 0;
            
            st_time_setting.hour_set_value = 0;     
            st_time_setting.minu_set_value = 0;     
            st_time_setting.week_set_value = 0;     
            st_time_setting.time_set_location = 0;  
            
            st_mode_setting.set_mode_type_flag = 0;
            st_mode_setting.set_work_week_flag = 0;
            st_mode_setting.set_work_time_flag = 0;
            st_mode_setting.set_open_close_flag = 0;
            
            st_current_status.wait_confirm_wash = 0; 
            st_current_status.current_wash_status = 0;*/
            WS_DEBUG("Device Status: Set On\r\n");
            break;
        case 0x02:
            air_clean_cfg.power_onoff = 0x00;
            WS_DEBUG("Device Status: Set Off\r\n");
            break;
        default:
            WS_DEBUG("Device Status: Invalid Cmd\r\n");
            break;
    }

    switch(rxBuffer[5])
    {
        case 0x00:
            WS_DEBUG("Working Mode: Keep in current status\r\n");
            break;
        case 0x01:
            air_clean_cfg.work_mode = MODE_LOCAL_ONE;
            WS_DEBUG("Working Mode:Set A\r\n");
            break;
        case 0x02:
            air_clean_cfg.work_mode = MODE_LOCAL_TWO;
            WS_DEBUG("Working Mode:Set B\r\n");
            break;
        case 0x03:
            air_clean_cfg.work_mode = MODE_LOCAL_THREE;
            WS_DEBUG("Working Mode:Set C\r\n");
            break;
        case 0x04:
            air_clean_cfg.work_mode = MODE_LOCAL_FOURE;
            WS_DEBUG("Working Mode:Set D\r\n");
            break;
        case 0x05:
            air_clean_cfg.work_mode = MODE_CUSTOM;
            WS_DEBUG("Working Mode:Set Customize\r\n");
            break;
        default:
            WS_DEBUG("Working Mode: Invalid Cmd\r\n");
            break;
    }

    switch(rxBuffer[6])
    {
        case 0x00:
            WS_DEBUG("Atomizer Status: Keep in current status\r\n");
            break;
        case 0x01:
            air_clean_cfg.which_atomizer =  ATOMIZER_ONE;
            WS_DEBUG("Atomizer Status: Set A\r\n");
            break;
        case 0x02:
            air_clean_cfg.which_atomizer =  ATOMIZER_TWO;
            WS_DEBUG("Atomizer Status: Set B\r\n");
            break;
        case 0x03:
            air_clean_cfg.atomizer_lock =  0;
            WS_DEBUG("Atomizer Status: Auto\r\n");
            break;
        default:
            WS_DEBUG("Atomizer Status: Invalid Cmd\r\n");
            break;
    }

    switch(rxBuffer[7])
    {
        case 0x00:
            WS_DEBUG("Air Volume Status: Keep in current status\r\n");
            break;
        case 0x01:
            air_clean_cfg.air_fan_level = AIR_FAN_LEVEL_ONE;
            st_current_status.siwtch_fan_level_flag = ON;
            write_cfg_to_flash();
            WS_DEBUG("Air Volume Status: 1st stage\r\n");
            break;
        case 0x02:
            air_clean_cfg.air_fan_level = AIR_FAN_LEVEL_TWO;
            st_current_status.siwtch_fan_level_flag = ON;
            write_cfg_to_flash();
            WS_DEBUG("Air Volume Status: 2nd stage\r\n");
            break;
        case 0x03:
            air_clean_cfg.air_fan_level = AIR_FAN_LEVEL_THREE;
            st_current_status.siwtch_fan_level_flag = ON;
            write_cfg_to_flash();
            WS_DEBUG("Air Volume Status: 3rd stage\r\n");
            break;
        default:
            WS_DEBUG("Air Volume Status: Invalid Cmd\r\n");
            break;
    }

    switch(rxBuffer[8])
    {
        case 0x00:
            WS_DEBUG("Concentration Status: Keep in current status\r\n");
            break;
        case 0x01:
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) != PULLDOWN)
            {
                air_clean_cfg.liquid_density = LIQUID_DENSITY_ONE;
                write_cfg_to_flash(); 
                WS_DEBUG("Concentration Status: 1st stage\r\n");
            }
            break;
        case 0x02:
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) != PULLDOWN)
            {
                air_clean_cfg.liquid_density = LIQUID_DENSITY_TWO;
                write_cfg_to_flash(); 
                WS_DEBUG("Concentration Status: 2nd stage\r\n");
            }
            break;
        case 0x03:
            if(get_liquid_level_sensor_status(LIQUID_LEVEL_SENSOR_ORG_LOW) != PULLDOWN)
            {
                air_clean_cfg.liquid_density = LIQUID_DENSITY_THREE;
                write_cfg_to_flash(); 
                WS_DEBUG("Concentration Status: 3rd stage\r\n");
            }
            break;
        default:
            WS_DEBUG("Concentration Status: Invalid Cmd\r\n");
            break;
    }

    switch(rxBuffer[9])
    {
        case 0x00:
            WS_DEBUG("Washing Status: Keep in current status\r\n");
            break;
        case 0x01:
            WS_DEBUG("Washing Status: idle\r\n");
            break;
        case 0x02:
            WS_DEBUG("Washing Status: Working\r\n");
            break;
        default:
            WS_DEBUG("Concentration Status: Invalid Cmd\r\n");
            break;
    }

    switch(rxBuffer[10])
    {
        case 0x00:
            WS_DEBUG("Device running status: Keep in current status\r\n");
            break;
        case 0x01:
            WS_DEBUG("Device running Status: Abnormal\r\n");
            break;
        case 0x02:
            WS_DEBUG("Device running Status: Replacing filter\r\n");
            break;
        default:
            WS_DEBUG("Device running Status: Invalid Cmd\r\n");
            break;
    }
    }

    void UploadDeviceInfo(uint8_t insCode)
    {
    uint8_t i;
    DevInfo devInfo;
    devInfo.head = 0x55;
    devInfo.data_length = 0x13;
    devInfo.mode = insCode;
    devInfo.data_info[0] = GetDeviceSwitch();
    devInfo.data_info[1] = GetWorkingMode();
    devInfo.data_info[2] = GetAtomizerStatus();
    devInfo.data_info[3] = GetAirVolumeStatus();
    devInfo.data_info[4] = GetConcentrationStatus();
    devInfo.data_info[5] = GetWashingStatus();
    devInfo.data_info[6] = GetDeviceRunningStatus();
    devInfo.data_info[7] = GetNH3Value();
    devInfo.data_info[8] = GetPlantLiquidStatus();
    for(i=9;i<16;i++)
    {
        devInfo.data_info[i] = 0x00;
    }
    devInfo.check_data = GetDeviceVerifyCode(devInfo,16);

    /*
    WS_DEBUG("In upload info,check_data is 0x%x, data info:",devInfo.check_data);
    for(i=0;i<16;i++)
    {
        WS_DEBUG("0x%x ",devInfo.data_info[i]);
    }
    WS_DEBUG("\r\n");
    */

    SendDevInfo(devInfo);
}


void SetCustomeMode(uint8_t value)
{
    uint8_t i;
    for(i=1;i<8;i++)
    {
        if(BIT_GET(value,i-1))
            air_clean_cfg.st_custom_mode.custom_work_week[i]=0x01;
        else
            air_clean_cfg.st_custom_mode.custom_work_week[i]=0x00;
    }
}

void SetAtomizerRunningTime(uint8_t value)
{
    air_clean_cfg.st_custom_mode.custom_open_min = value;
}

void SetAtomizerStopTime(uint8_t value)
{
    air_clean_cfg.st_custom_mode.custom_close_min = value;
}

void SetCustomTime(uint8_t n,uint8_t value1,uint8_t value2,uint8_t value3,uint8_t value4)
{
    air_clean_cfg.st_custom_mode.custom_time_plan[n].start_hour = value1;
    air_clean_cfg.st_custom_mode.custom_time_plan[n].start_minute = value2;
    air_clean_cfg.st_custom_mode.custom_time_plan[n].stop_hour = value3;
    air_clean_cfg.st_custom_mode.custom_time_plan[n].stop_minute = value4;
}

void CustomizeSetup()
{
    SetCustomeMode(rxBuffer[4]);
    SetCustomTime(0,rxBuffer[5],rxBuffer[6],rxBuffer[7],rxBuffer[8]);
    SetCustomTime(1,rxBuffer[9],rxBuffer[10],rxBuffer[11],rxBuffer[12]);
    SetCustomTime(2,rxBuffer[13],rxBuffer[14],rxBuffer[15],rxBuffer[16]);
    SetCustomTime(3,rxBuffer[17],rxBuffer[18],rxBuffer[19],rxBuffer[20]);
    SetCustomTime(4,rxBuffer[21],rxBuffer[22],rxBuffer[23],rxBuffer[24]);
    SetCustomTime(5,rxBuffer[25],rxBuffer[26],rxBuffer[27],rxBuffer[28]);
    SetCustomTime(6,rxBuffer[29],rxBuffer[30],rxBuffer[31],rxBuffer[32]);
    SetCustomTime(7,rxBuffer[33],rxBuffer[34],rxBuffer[35],rxBuffer[36]);
    SetCustomTime(8,rxBuffer[37],rxBuffer[38],rxBuffer[39],rxBuffer[40]);
    SetCustomTime(9,rxBuffer[41],rxBuffer[42],rxBuffer[43],rxBuffer[44]);

    SetAtomizerRunningTime(rxBuffer[45]);
    SetAtomizerStopTime(rxBuffer[46]);

    /*
    WS_DEBUG("working day mode is:%d\r\n",rxBuffer[4]);
    WS_DEBUG("Working time1:%d:%d-%d:%d\r\n",rxBuffer[5],rxBuffer[6],rxBuffer[7],rxBuffer[8]);
    WS_DEBUG("Working time2:%d:%d-%d:%d\r\n",rxBuffer[9],rxBuffer[10],rxBuffer[11],rxBuffer[12]);
    WS_DEBUG("Working time3:%d:%d-%d:%d\r\n",rxBuffer[13],rxBuffer[14],rxBuffer[15],rxBuffer[16]);
    WS_DEBUG("Working time4:%d:%d-%d:%d\r\n",rxBuffer[17],rxBuffer[18],rxBuffer[19],rxBuffer[20]);
    WS_DEBUG("Working time5:%d:%d-%d:%d\r\n",rxBuffer[21],rxBuffer[22],rxBuffer[23],rxBuffer[24]);
    WS_DEBUG("Working time6:%d:%d-%d:%d\r\n",rxBuffer[25],rxBuffer[26],rxBuffer[27],rxBuffer[28]);
    WS_DEBUG("Working time7:%d:%d-%d:%d\r\n",rxBuffer[29],rxBuffer[30],rxBuffer[31],rxBuffer[32]);
    WS_DEBUG("Working time8:%d:%d-%d:%d\r\n",rxBuffer[33],rxBuffer[34],rxBuffer[35],rxBuffer[36]);
    WS_DEBUG("Working time9:%d:%d-%d:%d\r\n",rxBuffer[37],rxBuffer[38],rxBuffer[39],rxBuffer[40]);
    WS_DEBUG("Working time10:%d:%d-%d:%d\r\n",rxBuffer[41],rxBuffer[42],rxBuffer[43],rxBuffer[44]);
    WS_DEBUG("Atomizer running time %d mins\r\n",rxBuffer[45]);
    WS_DEBUG("Atomizer stop time %d mins\r\n",rxBuffer[46]);
    */
}

void CustomizeQuery()
{
    uint8_t customizeData[48],i;
    customizeData[0] = 0x55;
    customizeData[1] = 0x00;
    customizeData[2] = 0x2E;
    customizeData[3] = 0x05;

    customizeData[4] = GetCustomMode();
    customizeData[5] = air_clean_cfg.st_custom_mode.custom_time_plan[0].start_hour;
    customizeData[6] = air_clean_cfg.st_custom_mode.custom_time_plan[0].start_minute;
    customizeData[7] = air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_hour;
    customizeData[8] = air_clean_cfg.st_custom_mode.custom_time_plan[0].stop_minute;

    customizeData[9] = air_clean_cfg.st_custom_mode.custom_time_plan[1].start_hour;
    customizeData[10] = air_clean_cfg.st_custom_mode.custom_time_plan[1].start_minute;
    customizeData[11] = air_clean_cfg.st_custom_mode.custom_time_plan[1].stop_hour;
    customizeData[12] = air_clean_cfg.st_custom_mode.custom_time_plan[1].stop_minute;

    customizeData[13] = air_clean_cfg.st_custom_mode.custom_time_plan[2].start_hour;
    customizeData[14] = air_clean_cfg.st_custom_mode.custom_time_plan[2].start_minute;
    customizeData[15] = air_clean_cfg.st_custom_mode.custom_time_plan[2].stop_hour;
    customizeData[16] = air_clean_cfg.st_custom_mode.custom_time_plan[2].stop_minute;

    customizeData[17] = air_clean_cfg.st_custom_mode.custom_time_plan[3].start_hour;
    customizeData[18] = air_clean_cfg.st_custom_mode.custom_time_plan[3].start_minute;
    customizeData[19] = air_clean_cfg.st_custom_mode.custom_time_plan[3].stop_hour;
    customizeData[20] = air_clean_cfg.st_custom_mode.custom_time_plan[3].stop_minute;

    customizeData[21] = air_clean_cfg.st_custom_mode.custom_time_plan[4].start_hour;
    customizeData[22] = air_clean_cfg.st_custom_mode.custom_time_plan[4].start_minute;
    customizeData[23] = air_clean_cfg.st_custom_mode.custom_time_plan[4].stop_hour;
    customizeData[24] = air_clean_cfg.st_custom_mode.custom_time_plan[4].stop_minute;

    customizeData[25] = air_clean_cfg.st_custom_mode.custom_time_plan[5].start_hour;
    customizeData[26] = air_clean_cfg.st_custom_mode.custom_time_plan[5].start_minute;
    customizeData[27] = air_clean_cfg.st_custom_mode.custom_time_plan[5].stop_hour;
    customizeData[28] = air_clean_cfg.st_custom_mode.custom_time_plan[5].stop_minute;

    customizeData[29] = air_clean_cfg.st_custom_mode.custom_time_plan[6].start_hour;
    customizeData[30] = air_clean_cfg.st_custom_mode.custom_time_plan[6].start_minute;
    customizeData[31] = air_clean_cfg.st_custom_mode.custom_time_plan[6].stop_hour;
    customizeData[32] = air_clean_cfg.st_custom_mode.custom_time_plan[6].stop_minute;

    customizeData[33] = air_clean_cfg.st_custom_mode.custom_time_plan[7].start_hour;
    customizeData[34] = air_clean_cfg.st_custom_mode.custom_time_plan[7].start_minute;
    customizeData[35] = air_clean_cfg.st_custom_mode.custom_time_plan[7].stop_hour;
    customizeData[36] = air_clean_cfg.st_custom_mode.custom_time_plan[7].stop_minute;

    customizeData[37] = air_clean_cfg.st_custom_mode.custom_time_plan[8].start_hour;
    customizeData[38] = air_clean_cfg.st_custom_mode.custom_time_plan[8].start_minute;
    customizeData[39] = air_clean_cfg.st_custom_mode.custom_time_plan[8].stop_hour;
    customizeData[40] = air_clean_cfg.st_custom_mode.custom_time_plan[8].stop_minute;

    customizeData[41] = air_clean_cfg.st_custom_mode.custom_time_plan[9].start_hour;
    customizeData[42] = air_clean_cfg.st_custom_mode.custom_time_plan[9].start_minute;
    customizeData[43] = air_clean_cfg.st_custom_mode.custom_time_plan[9].stop_hour;
    customizeData[44] = air_clean_cfg.st_custom_mode.custom_time_plan[9].stop_minute;

    customizeData[45] = GetAtomizerRunningTime();
    customizeData[46] = GetAtomizerStopTime();
    customizeData[47] = gen_verify_code(customizeData,47);

    /*
    //WS_DEBUG("customize query data is :",customizeData[i]);
    for(i=0;i<48;i++)
    {
        WS_DEBUG("0x%x ",customizeData[i]);
        Usart_SendByte(WIFI_USARTx,customizeData[i]);
    }
    WS_DEBUG("\r\n");
    */
}

void RefreshWifiStatus(uint8_t statusValue)
{
    //WS_DEBUG("+++++++++++++++++++++++++++ 888888 +++++++++++++++++++++++++\r\n");
    if(0x00 == statusValue)
    {
        WS_DEBUG("********************\r\n");
        WS_DEBUG("Wifi Connect success\r\n");
        WS_DEBUG("********************\r\n");
        
        st_current_status.net_trigger_flag = 0;
        //st_current_status.current_net_status = 2;
        g_current_net_status = 2;
        
    }
    else if(0x01 == statusValue)
    {
        WS_DEBUG("********************\r\n");
        WS_DEBUG("Wifi Connect failed\r\n");
        WS_DEBUG("********************\r\n");
        
        //st_current_status.current_net_status = 0;
        g_current_net_status = 0;
    }
    else if(0x02 == statusValue)
    {
        WS_DEBUG("********************\r\n");
        WS_DEBUG("Connect Server failed \r\n");
        WS_DEBUG("********************\r\n");
        st_current_status.net_trigger_flag = 0;
        //st_current_status.current_net_status = 1;
        g_current_net_status = 1;
    }else if(0x03 == statusValue)
    {
        WS_DEBUG("********************\r\n");
        WS_DEBUG("Waiting for peiwang\r\n");
        WS_DEBUG("********************\r\n");
        //st_current_status.current_net_status = 0;
        g_current_net_status = 0;
    }else if(0x04 == statusValue)
    {
        WS_DEBUG("********************\r\n");
        WS_DEBUG("Wifi not triggerd\r\n");
        WS_DEBUG("********************\r\n");
    }
}