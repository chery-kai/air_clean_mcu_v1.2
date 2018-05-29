#ifndef _APPFUNC_H_
#define _APPFUNC_H_

#include  <os.h>
#include  <stm32f10x.h>

#include "appfunc.h"
#include "includes.h"


#define DEBUG

#ifdef DEBUG
    #define WS_DEBUG printf
#else
    #define WS_DEBUG
#endif

//PRODUCT_TYPE(产品类型) 定义
#define AUTO_ALONE_8L       1   //8L自动单机通用版
#define AUTO_EDUCATE_8L     2   //8L自动教育版 


#define TEST_DS3231         0

#define TEST_AUTO_WASH      1       //测试自动清洗宏定义

#define NETWORK_VERSION     0       //区分"单机版"和"网络版"
//0: Standalone Version(单机版) ; 1: Network Version(网络版)

#define RELAY_EDITION       1        //继电器版本定义
//继电器版本: 使用同一个引脚控制风扇的开关以及风量大小(PWM波输出)


//**********************************************************************
#define MANUAL_EDITION      0       //区分手动版本

#if MANUAL_EDITION
    #define MANUAL_ADD_LIQUID_SAFE_TIME     270     //手动版加液保护时间
    #define MANUAL_ORG_TO_WORK_SAFE_TIME    30      //手动版配比保护时间
    #define MANUAL_LOW_ORG_TO_WORK_COUNT    1       //原液箱到低液位后抽液到工作仓的次数
#endif
//**********************************************************************







/*
*********************************************************************************************************
*                                         宏 定 义
*********************************************************************************************************
*/

//开关的状态
#define OFF         0
#define ON          1

#define PULLDOWN    0
#define PULLUP      1

//*******************************************************************

//1.雾化器标识
#define ATOMIZER_ONE                1
#define ATOMIZER_TWO                2
#define ATOMIZER_ALL                3

//2.风扇标识
#define AIR_FAN_ONE                 1
#define AIR_FAN_TWO                 2
#define AIR_FAN_ALL                 3

//3.液泵标识
#define PUMP_TO_ORG                 1           //加液泵
#define PUMP_TO_WORK                2           //补液泵(配比泵)
#define PUMP_ALL                    3

//4.电磁阀标识
#define SOLENOID_INTO_WORK          1           //进水电磁阀
#define SOLENOID_OUT_WORK           2           //排水电磁阀
#define SOLENOID_ALL                3

//设备保护时间
#define ADD_LIQUID_SAFE_TIME        360         //加液保护时间(6min)

//#define SUPPLY_WATER_SAFE_TIME      24          //进水电磁阀保护时间
#define SUPPLY_WATER_TIMEOUT        150         //注水保护时间

#define DRAIN_LIQUID_SAFE_TIME      150         //排水电磁阀保护时间150秒
#define DRAIN_LIQUID_DELAY_TIME     40          //排液排至低液位后延迟40秒关闭出水电磁阀

//*******************************************************************

//工作仓液体浓度
#define LIQUID_DENSITY_ONE                  1       //浓度低档 -- 抽液时间 ADD_LIQUID_LOW_TIME
#define LIQUID_DENSITY_TWO                  2       //浓度中档 -- 抽液时间 ADD_LIQUID_MID_TIME
#define LIQUID_DENSITY_THREE                3       //浓度高档 -- 抽液时间 ADD_LIQUID_HIGH_TIME

//抽液时间分3挡, 功能显示屏以浓度显示低中高三档
#define ADD_LIQUID_RESET_TIME               7000    //复位后第一次配比时间7秒
#define ADD_LIQUID_HIGH_TIME                2700    //高档 -- 抽液时间2.7秒
#define ADD_LIQUID_MID_TIME                 2300    //中档 -- 抽液时间2.3秒
#define ADD_LIQUID_LOW_TIME                 1800    //低档 -- 抽液时间1.8秒


//原液仓低液位时工作仓继续配比次数规定
#define DENSITY_ONE_ORGTOWORK_COUT          400     //低浓度时, 原液仓到工作仓的配比次数
#define DENSITY_TWO_ORGTOWORK_COUT          330     //中浓度时, 原液仓到工作仓的配比次数
#define DENSITY_THREE_ORGTOWORK_COUT        220     //高浓度时, 原液仓到工作仓的配比次数

//风扇转速等级
#define AIR_FAN_LEVEL_ONE                   1
#define AIR_FAN_LEVEL_TWO                   2
#define AIR_FAN_LEVEL_THREE                 3

//故障码 : E12权限最大，然后E1-E5,E10-E11
#define ERR_NONE                            0
#define ERR_FAN_ONE_FAULT                   1       //E1：风扇1异常
#define ERR_FAN_TWO_FAULT                   2       //E2：风扇2异常
#define ERR_WORK_SUPPLY_WATER               3       //E3：工作仓加液异常(实际即为进水异常)
#define ERR_WORK_DRAIN_LIQUID               4       //E4：工作仓排液异常
#define ERR_ORG_ADD_LIQUID                  5       //E5：液箱加液异常
#define ORG_MUST_ADD_PLANT                  7       //E7：告知客户液箱必须添加植物液
#define FORCE_WASH_STOP_DEV                 10      //E10：告知客户强制清洗导致设备停止

//首次出现报警溢液点位, 出现提示E11, 不影响设备正常工作
#define ERR_OVERFLOW_FIRST                  11      //E11：首次出现报警溢液点位

//若1小时内2次以上含2次出现, 就出现E12
#define ERR_OVERFLOW_MANY_TIMES             12      //E12：多次出现报警溢液点位

//数码管显示类型
#define SHOW_TIME           0       //显示时间
#define SHOW_NH3            1       //显示NH3值
#define SHOW_WORK_LIQUID_LEVEL  3   //显示工作仓液位

//植物液当前液位
#define PLANT_LIQUID_LOW    0       //植物液低液位
#define PLANT_LIQUID_MID    1       //植物液中液位
#define PLANT_LIQUID_HIGH   2       //植物液高液位

//强制清洗/雾化器切换标识
#define ATOMIZER_INTERVAL_TYPE      0       //查询雾化器工作时长(切换)
#define WASH_INTERVAL_TYPE          1       //查询设备工作时长(强制清洗)
#define E7_CLOSE_INTERVAL_TYPE      2       //出现E7或关机时长(注水+排液)

#define AUTO_WASH_MODE              0       //自动清洗模式
#define MANUAL_WASH_MODE            1       //手动清洗模式

#if TEST_AUTO_WASH
    #define ATOMIZER_SWITCH_DAY         4
    #define WASH_FORCE_DAY              4
    #define WASH_FORCE_HOUR             2
    
    #define E7_CLOSE_MAINTAIN_DAY       2
    #define E7_CLOSE_MAINTAIN_HOUR      3
#else
    #define ATOMIZER_SWITCH_DAY         7       //雾化器自动切换时间7天(1周)

    //清洗时间改为14天, 第15天的凌晨2点
    #define WASH_FORCE_DAY              14
    #define WASH_FORCE_HOUR             2
    
    //关机或E7状态保养第4天的凌晨2点
    #define E7_CLOSE_MAINTAIN_DAY       3
    #define E7_CLOSE_MAINTAIN_HOUR      2
#endif

/*
//遥控器原理图对应按键码
电源: 0x09; 音量: 0x13; 热点: 0x41; 
上:   0x08; 下:   0x06; 左:   0x11; 右:   0x3E;
确认: 0x07; 取消: 0x10; 风量: 0x0F; 浓度: 0x37;
加液: 0x0E; 时间: 0x32; 模式: 0x0D; 清洗: 0x2C;
氨气值: 0x02

//实际测试获取按键码
电源: 0x90    音量: 0xC8    热点: 0x82
向上: 0x10
向左: 0x88    确认: 0xE0    向右: 0x7C
取消: 0x08    向下: 0x60

风量: 0xF0    浓度: 0xEC
加液: 0x70    时间: 0x4C
模式: 0xB0    清洗: 0x34
氨气值: 0x40
*/
//根据实际测得的按键码定义

//新屏
#define IR_CODE_RESET               0xF4        //复位键长按
#define IR_CODE_POWER               0x10        //电源键

#define IR_CODE_SET_TIME            0x0F        //设置时间
#define IR_CODE_NET_LONG            0x68        //网络键长按

#define IR_CODE_UP                  0x40        //上
#define IR_CODE_DOWN                0x80        //下
#define IR_CODE_LEFT                0x78        //左
#define IR_CODE_RIGHT               0xC0        //右

#define IR_CODE_CONFIRM_SHORT       0xB4        //确认键短按
#define IR_CODE_CONFIRM_LONG        0xE0        //确认键长按

#define IR_CODE_SET_MODE            0xA2        //设置模式

#define IR_CODE_CANCEL              0xE2        //取消键
#define IR_CODE_CANCEL_LONG         0x38        //取消键长按

#define IR_CODE_WASH_SHORT          0x22        //清洗键短按
#define IR_CODE_WASH_LONG           0x20        //清洗键长按

#define IR_CODE_DENSITY             0x12        //浓度键
#define IR_CODE_FAN_LEVEL           0x52        //风量键

#define IR_CODE_ADD_LIQUID_SHORT    0x49        //加液键短按
#define IR_CODE_ADD_LIQUID_LONG     0xA0        //加液键长按

#define IR_CODE_ATOMIZER_SHORT      0x62        //雾化器短按
#define IR_CODE_ATOMIZER_LONG       0xB8        //雾化器长按

#define IR_CODE_SHOW_NH3            0x58        //氨气键

//*******************************************************************

#define MODE_STANDARD       0       //标准模式

//2018.02.22 : 本地模式舍弃, 使用标准模式
#define MODE_LOCAL_ONE      1       //本地模式1
#define MODE_LOCAL_TWO      2       //本地模式2
#define MODE_LOCAL_THREE    3       //本地模式3
#define MODE_LOCAL_FOURE    4       //本地模式4
#define MODE_CUSTOM         5       //自定义(智能)模式

#define DEFAULT_WORK_MODE   1       //默认工作模式:本地模式1
#define DEFAULT_OPEN_MIN    3       //默认启动时长:3分钟
#define DEFAULT_CLOSE_MIN   7       //默认暂停时长:7分钟

//智能模式默认
#define DEFAULT_START_HOUR  7
#define DEFAULT_START_MIN   30
#define DEFAULT_STOP_HOUR   17
#define DEFAULT_STOP_MIN    0

//*******************************************************************

//bitmap相关操作
#define BIT_GET(bitmap, x)      ((bitmap) >> (x) & 1U)
#define BIT_SET(bitmap, x)      ((bitmap) |= (1<<(x)))
#define BIT_CLEAR(bitmap, x)    ((bitmap) &= ~(1<<(x)))
#define     SET     1
#define     CLEAR   0

//配置数据所在 Flash 路径
//#define AIR_CLEAN_CFG_FILE  "/tffs0/air_clean_rel.config"


/*
*********************************************************************************************************
*                                         结构体定义
*********************************************************************************************************
*/
typedef enum {
    SUNDAY      =0,
    MONDAY      =1,
    TUESDAY     =2,
    WEDNESDAY   =3,
    THURSDAY    =4,
    FRIDAY      =5,
    SATURDAY    =6
    //SUNDAY    =7
}week_day_enu;

typedef struct {
    char start_hour;
    char start_minute;
    char stop_hour;
    char stop_minute;
}time_plan_st;

typedef struct {
    char today_on_off;
    time_plan_st time_plan[2];  //每天分为"上下午"两个时段
}day_plan_st;

typedef struct {
    char mode;
    day_plan_st week_plan[7];   //0bit代表周日, 1bit代表周一
}air_clean_mode_st;
//64 Byte

typedef struct {
    int start_year;
    int start_month;
    int start_day;
}date_record_st;
//12 Byte


typedef struct {
    char custom_work_week[8];           //数组1-7记录"周一到周日开关状态"  8字节
    
    /*
    typedef struct {
        char start_hour;
        char start_minute;
        char stop_hour;
        char stop_minute;
    }time_plan_st;
    */
    //根据学校需求, 自定义模式: 遥控器可设置 1 个工作时间段; 后台可设置 10 个工作时间段
    time_plan_st custom_time_plan[10];         //每天可设置10个时段     4*10字节
    
    int custom_open_min;           //自定义模式启动时长     4字节
    int custom_close_min;          //自定义模式暂停时长     4字节
}custom_mode_st;
//8+4*10+4+4 = 56 字节


//1.需要记录到  Flash 中的数据记录到 air_clean_cfg_st 结构体中
typedef struct 
{
    //标准模式定义, air_clean_mode_st结构: 64Byte
    air_clean_mode_st st_standard_mode;
    
    //启停时长针对标准模式, 记录到Flash中
    int standard_mode_open_min;             //标准模式启动时长
    int standard_mode_close_min;            //标准模式暂停时长
    //*************************************************************************
    
    //自定义模式数据定义
    //*************************************************************************
    custom_mode_st st_custom_mode;
    
    
    char power_onoff;                       //电源状态
    char work_mode;                         //工作模式: 智能模式/本地模式1-4
    char which_atomizer;                    //使用的雾化器(A/B)
    char air_fan_level;                     //风量(调节风轮的速度等级)
    
    int liquid_density;                     //浓度(配比浓度:加液时长不同实现)
    int err_code_bitmap;                    //错误码定义
    int org_low_to_work_count;              //原液仓低液位时继续抽液到工作仓计数
    
    date_record_st dev_work_time;           //设备工作时间
    date_record_st atomizer_start_time;     //雾化器工作时间
    
    char wash_mode;                         //清洗模式: 0为自动清洗; 1为手动清洗
    char atomizer_lock;                     //雾化器锁定: 1为锁定
    char cfg_flash_flag;                    //设置为固定值: 0x5A
    
    char onekey_drain_fished;               //一键排液完成标识，完成后，清洗灯显示粉色
    char cfg_data_init_flag;            //复位标识(第一次初始化写Flash标识)
    //2018.02.22 : 在默认模式情况按复位后第一次配比时间为3.2S第二次恢复浓度2时间2.7秒
		
    //保证上面的数据 4 字节对齐，从而保证写入Flash的正确性
    //结构体中数据的顺序切勿改变！！！
} air_clean_cfg_st;


typedef struct {
    int st_hour;
    int st_min;
    int st_sec;
}time_record_st;


//2.用于实时状态显示的数据记录到 current_status_st 结构体中
typedef struct {
    
    //原液箱液位状态
    char current_plant_liquid;          //植物液当前液位
    
    //氨气浓度
    //float current_nh3_value;            //当前氨气浓度值(上传至服务器的平均值)
    float nh3_show_real_value;          //液晶屏显示的实时获取到的氨气浓度值(ppm转换值)
    
    // 氨气ppm值
    int current_nh3_ppm;                //当前氨气浓度值ppm (上传至服务器的平均值)

    //清洗状态
    char current_wash_status;           //当前清洗状态
    char wait_wash_confirm;             //等待清洗确认(清洗+确认,等待'确认'按键)
    char manual_wash_stopdev;           //手动清洗模式下,20天未清洗,强制设备停止工作
    
    char auto_wash_prompt;              //自动清洗提示, 自动清洗完成24小时以内亮黄灯
    char auto_wash_past_hours;          //自动清洗完成的小时数

    //液晶屏显示相关
    char current_number_show;           //标识数码管显示类型(时间/NH3)
    
    //相关标志
    char add_plant_liquid_flag;         //加液标志
    char siwtch_atomizer_flag;          //切换雾化器标志
    char siwtch_fan_level_flag;         //调节风量标志
    char siwtch_work_mode_flag;         //切换工作模式标志
    char close_fan_flag;                //关闭风扇标识(需延时5秒关闭)

    char current_atomizer_status[2];    //正在运行设为1，不在运行就设置为0.
    char nh3_err_flag;                  //氨气传感器故障标识
    
    char net_trigger_flag;              //触发连接网络标识
    char sensor_work_low_confirm;       //工作仓低液位确认标识

    //char auto_addto_work_flag;          //工作仓自动配比标识(工作仓低液位,原液箱非低液位), 用于标识工作仓液位显示
    
    char start_up_flag;                 //开机启动标识
    
    #if MANUAL_EDITION
    char manual_need_auto_wash;
    #endif
    
    
    char one_key_atomize_flag;          //一键出雾功能标识
    char onekey_drain_status;           //一键排液功能标识
    
    char first_overflow_flag;               //工作仓第一次溢出标识
    date_record_st  first_overflow_date;
    time_record_st  first_overflow_time;    //工作仓第一次溢出时间点
    time_record_st  second_overflow_time;   //工作仓第二次溢出时间点
    
    char poweron_error_code_flag;           //开机状态下出现错误码标识(此时设备不工作)
    
    char wash_drain_liquid_status;          //清洗排液状态(1:正在清洗排液;0:未执行清洗排液)
    
    char overflow_drain_liquid_status;
    
    char peibi_overflow_num;       //出现E11后, 第一次成功配比不操作, 第二次成功配比之后清除e11
    
    char power_off_e7_maintance_status;     //关机E7状态下保养状态
    date_record_st power_off_e7_time;       //关机或者出现E7的时间
    
} current_status_st;

//*****************************************************************************

//设置状态结构体
typedef struct {
    char setting_time_flag;                 //设置时间标志
    char setting_mode_flag;                 //设置模式标志
    
    char mt_setting_finished;               //设置时间/模式结束
} setting_status_st;


//时间设置结构体
typedef struct {
    unsigned char hour_set_value;           //小时设置值:0-23
    unsigned char minu_set_value;           //分钟设置值:0-59
    unsigned char week_set_value;           //周设置值  :1-7
    unsigned char time_set_location;        //设置位置  :1-3 -- 1:小时; 2:分钟; 3:周
} time_setting_st;


//模式设置结构体
typedef struct 
{
    //设置模式类型
    char mode_type_select;                          //1:本地(标准)模式 2:自定义模式
    //设置模式类型标识(是否已选择模式类型)
    char set_mode_type_flag;                        //0:未设置 1:已设置
    
    //标准模式设置开几停几, 取消选择本地模式
    unsigned char standard_open_min;                //开几分钟(标准模式)
    unsigned char standard_close_min;               //停几分钟(标准模式)
    char std_set_open_close_flag;                   //0:不处于"设置开几停几"状态; 1:正在"设置开几"状态; 2:正在"设置停几"状态
    
    //设置周开关
    char custom_work_week[8];                       //数组1-7记录开关状态
    //设置周开关标识(标识设置到周几)
    char set_work_week_flag;                        //0:不处于"设置周开关标识"状态; 1-7:正在"设置周1-7开关标识"状态
    
    //设置工作时间段
    unsigned char custom_start_work_hour;           //开始工作时间段:小时    1
    unsigned char custom_start_work_min;            //开始工作时间段:分钟    2
    unsigned char custom_stop_work_hour;            //停止工作时间段:小时    3
    unsigned char custom_stop_work_min;             //停止工作时间段:分钟    4
    char set_work_time_flag;                        //0:不处于"设置工作时间段"状态; 1-4:正在"设置工作时间段"状态
    
    //设置开几停几
    unsigned char custom_open_min;                  //开几分钟(自定义模式)
    unsigned char custom_close_min;                 //停几分钟(自定义模式)
    char set_open_close_flag;                       //0:不处于"设置开几停几"状态; 1:正在"设置开几"状态; 
} mode_setting_st;
//模式设置步骤: 1.选择设置的"模式类型"(标准模式/自定义模式)
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
//
void init_standard_mode();          //初始化标准模式
char configure_data_exists();       //检查Flash中是否有配置数据
void read_cfg_from_flash();         //从Flash读配置
void write_cfg_to_flash();          //写配置到Flash
void test_flash();                  //测试 Flash 项

//AIR CLEAN TASK
void cfg_data_init();                                   //配置数据初始化
void update_org_low_to_work_count();                    //更新"原液箱到低液位后还能抽液到工作仓的次数"
void set_nh3_concentration();                           //设置当前NH3平均浓度值
void atomizer_onoff(char which_atomizer, char onoff);   //控制雾化器开关
void liquid_pump_onoff(char which_pump, char onoff);    //控制液泵开关
void solenoid_onoff(char which_solenoid, char onoff);   //控制电磁阀开关 
void air_fan_onoff(char onoff);                         //控制风扇开关
void set_air_fan_level();                               //设置风扇的风量等级

int get_whole_dev_onoff();                              //确定当前模式设备是否运行
int close_all_device();                                 //关闭所有外部设备
int dev_start_work();                                    //设备开始工作
int atomizer_start_work();                              //一键出雾功能对应的设备开始工作(不判断是否在工作时间段内)
int add_plant_liquid();                                 //添加植物液(自动版)
int manual_add_plant_liquid();                          //添加植物液(手动版)
void do_current_wash();                                 //执行清洗

//KEY CONTROL TASK
char update_status_by_data_code(unsigned char data_code);

//SCREEN REFRESH TASK
void update_display_data(unsigned char * row_data);

//CYCLE CHECK/UPDATE TASK
int get_interval_days(int interval_type);
int get_err_num();
int wash_drain_liquid();
void overflow_drain_liquid();   //溢出放液

void overflow_setting();
void clear_first_overflow_time();


void power_off_e7_maintance();

extern air_clean_cfg_st air_clean_cfg;
extern current_status_st st_current_status;
extern char is_running;
extern char PRODUCT_TYPE;

extern setting_status_st st_setting_status;    //设置状态
extern time_setting_st st_time_setting;        //时间设置结构体
extern mode_setting_st st_mode_setting;        //模式设置结构体

//网络状态(复位之后, 网络状态不影响)
extern char g_current_net_status;               //当前网络状态

#endif  //_APPFUNC_H_
