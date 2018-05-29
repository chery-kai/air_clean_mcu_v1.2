#ifndef __LIQUID_SENSOR_H
#define __LIQUID_SENSOR_H


#include "stm32f10x.h"


/*
//相关引脚对应的液位都是低电平有效
原液箱液位传感器:
LOW_Y: PD8          //原液箱低液位
MID_Y: PD9          //原液箱中液位
HIGH_Y: PD10        //原液箱高液位

工作仓液位传感器:
LOW_W: PD11         //工作箱低液位
HIGH_W: PD12        //工作箱高液位
*/

//液箱液位传感器(三挡)
#define LIQUID_LEVEL_SENSOR_ORG_LOW         0
#define LIQUID_LEVEL_SENSOR_ORG_MID         1
#define LIQUID_LEVEL_SENSOR_ORG_HIGH        2

//工作仓液位传感器(三挡)
//#define LIQUID_LEVEL_SENSOR_WORK_LOW        3
//#define LIQUID_LEVEL_SENSOR_WORK_MID        4
//#define LIQUID_LEVEL_SENSOR_WORK_HIGH       5

#define LIQUID_LEVEL_SENSOR_WORK_LOW        3
#define LIQUID_LEVEL_SENSOR_WORK_HIGH       4
#define LIQUID_LEVEL_SENSOR_WORK_OVERFLOW   5


/* 定义liquid_sensor连接的GPIO端口, 用户只需要修改下面的代码即可改变控制液位传感器的引脚 */
#define     macLIQUID_SENSOR_GPIO_PORT      GPIOD                   /* GPIO端口 */
#define     macLIQUID_SENSOR_GPIO_CLK       RCC_APB2Periph_GPIOD    /* GPIO端口时钟 */

//原液箱液位对应引脚号
//手动版
//#define     macSENSOR_LOW_Y_GPIO_PIN        GPIO_Pin_11              /* LOW_Y引脚号 */
//#define     macSENSOR_MID_Y_GPIO_PIN        GPIO_Pin_12              /* MID_Y引脚号 */
//#define     macSENSOR_HIGH_Y_GPIO_PIN       GPIO_Pin_13             /* HIGH_Y引脚号 */

//自动版
#define     macSENSOR_LOW_Y_GPIO_PIN        GPIO_Pin_11              /* LOW_Y引脚号 */
#define     macSENSOR_HIGH_Y_GPIO_PIN       GPIO_Pin_13              /* HIGH_Y引脚号 */


//工作仓液位对应引脚号
#define     macSENSOR_LOW_W_GPIO_PIN        GPIO_Pin_14             /* LOW_W引脚号 */
#define     macSENSOR_HIGH_W_GPIO_PIN       GPIO_Pin_15             /* HIGH_W引脚号 */

//工作仓溢出液位点
#define     macSENSOR_MID_W_GPIO_PIN        GPIO_Pin_12             /* MID_W引脚号 */


//手动版
//#define     macSENSOR_LOW_Y_Read()          GPIO_ReadInputDataBit(macLIQUID_SENSOR_GPIO_PORT, macSENSOR_LOW_Y_GPIO_PIN)
//#define     macSENSOR_MID_Y_Read()          GPIO_ReadInputDataBit(macLIQUID_SENSOR_GPIO_PORT, macSENSOR_MID_Y_GPIO_PIN)
//#define     macSENSOR_HIGH_Y_Read()         GPIO_ReadInputDataBit(macLIQUID_SENSOR_GPIO_PORT, macSENSOR_HIGH_Y_GPIO_PIN)

//自动版
#define     macSENSOR_LOW_Y_Read()          GPIO_ReadInputDataBit(macLIQUID_SENSOR_GPIO_PORT, macSENSOR_LOW_Y_GPIO_PIN)
#define     macSENSOR_HIGH_Y_Read()         GPIO_ReadInputDataBit(macLIQUID_SENSOR_GPIO_PORT, macSENSOR_HIGH_Y_GPIO_PIN)

#define     macSENSOR_LOW_W_Read()          GPIO_ReadInputDataBit(macLIQUID_SENSOR_GPIO_PORT, macSENSOR_LOW_W_GPIO_PIN)
#define     macSENSOR_HIGH_W_Read()         GPIO_ReadInputDataBit(macLIQUID_SENSOR_GPIO_PORT, macSENSOR_HIGH_W_GPIO_PIN)
/*
//工作仓中液位
#define     macSENSOR_MID_W_Read()          GPIO_ReadInputDataBit(macLIQUID_SENSOR_GPIO_PORT, macSENSOR_MID_W_GPIO_PIN)
*/
//工作仓溢出液位 (overflow)
#define     macSENSOR_OFL_W_Read()          GPIO_ReadInputDataBit(macLIQUID_SENSOR_GPIO_PORT, macSENSOR_MID_W_GPIO_PIN)

#define     HIGH_LEV        1
#define     LOW_LEV         0

void LIQUID_SENSOR_GPIO_Config(void);
char get_liquid_level_sensor_status(char sensor);

#endif /* __LIQUID_SENSOR_H */
