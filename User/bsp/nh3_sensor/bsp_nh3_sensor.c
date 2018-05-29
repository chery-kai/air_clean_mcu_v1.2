/**
  ******************************************************************************
  * @file    bsp_nh3_sensor.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   串口2用于氨气传感器通讯
  ******************************************************************************
  */ 
  
#include "bsp_nh3_sensor.h"

 /**
  * @brief  USART1 GPIO 配置,工作模式配置 9600 8-N-1
  * @param  无
  * @retval 无
  */

  //参考: 40.STM32-ISO-MINI_UCOSIII_emWin_DEMO\User\bsp\sim900a

 /**
  * @brief  配置嵌套向量中断控制器NVIC
  * @param  无
  * @retval 无
  */
static void NH3_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 嵌套向量中断控制器组选择 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 配置USART为中断源 */
    NVIC_InitStructure.NVIC_IRQChannel = NH3_USART_IRQ;
    /* 抢断优先级*/
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    /* 子优先级 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    /* 使能中断 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    /* 初始化配置NVIC */
    NVIC_Init(&NVIC_InitStructure);
}

 /**
  * @brief  USART GPIO 配置,工作参数配置
  * @param  无
  * @retval 无
  */
void NH3_USART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // 打开串口GPIO的时钟
    NH3_USART_GPIO_APBxClkCmd(NH3_USART_GPIO_CLK, ENABLE);

    // 打开串口外设的时钟
    NH3_USART_APBxClkCmd(NH3_USART_CLK, ENABLE);

    // 将USART Tx的GPIO配置为推挽复用模式
    GPIO_InitStructure.GPIO_Pin = NH3_USART_TX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(NH3_USART_TX_GPIO_PORT, &GPIO_InitStructure);

    // 将USART Rx的GPIO配置为浮空输入模式
    GPIO_InitStructure.GPIO_Pin = NH3_USART_RX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(NH3_USART_RX_GPIO_PORT, &GPIO_InitStructure);

    // 配置串口的工作参数
    // 配置波特率
    USART_InitStructure.USART_BaudRate = NH3_USART_BAUDRATE;
    // 配置 针数据字长
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    // 配置停止位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    // 配置校验位
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    // 配置硬件流控制
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    // 配置工作模式，收发一起
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    // 完成串口的初始化配置
    USART_Init(NH3_USARTx, &USART_InitStructure);
    
    // 串口中断优先级配置
    //NVIC_Configuration();	//跳到 bsp_exti.c
    NH3_NVIC_Configuration();

    // 使能串口接收中断
    USART_ITConfig(NH3_USARTx, USART_IT_RXNE, ENABLE);
    
    // 使能串口
    USART_Cmd(NH3_USARTx, ENABLE);
    
    USART_ClearFlag(NH3_USARTx, USART_FLAG_TC);
}


/*****************  发送一个字符 **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
    /* 发送一个字节数据到USART */
    USART_SendData(pUSARTx,ch);
        
    /* 等待发送数据寄存器为空 */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}

/*****************  发送字符串 **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
    unsigned int k=0;
    do 
    {
        Usart_SendByte( pUSARTx, *(str + k) );
        k++;
    } while(*(str + k)!='\0');

    /* 等待发送完成 */
    while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
    {}
}

/*****************  发送一个16位数 **********************/
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)
{
    uint8_t temp_h, temp_l;

    /* 取出高八位 */
    temp_h = (ch&0XFF00)>>8;
    /* 取出低八位 */
    temp_l = ch&0XFF;

    /* 发送高八位 */
    USART_SendData(pUSARTx,temp_h);	
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);

    /* 发送低八位 */
    USART_SendData(pUSARTx,temp_l);	
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}

/****************** 发送8位的数组 ************************/
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num)
{
    uint8_t i;

    for(i=0; i<num; i++)
    {
        //printf("[Usart_SendArray]array[%d] = 0x%x \r\n",i, array[i]);
        /* 发送一个字节数据到USART */
        Usart_SendByte(pUSARTx,array[i]);
    }
    /* 等待发送完成 */
    while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);
}


#if 1
//中断缓存串口数据
#define UART_BUFF_SIZE      512
volatile    uint16_t uart_p = 0;
uint8_t     uart_buff[UART_BUFF_SIZE];

void bsp_USART2_IRQHandler(void)
{
    if(uart_p<UART_BUFF_SIZE)
    {
        if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
        {
            uart_buff[uart_p] = USART_ReceiveData(USART2);
            uart_p++;
        }
    }
}


//获取接收到的数据和长度
char *get_rebuff(uint8_t *len)
{
    *len = uart_p;
    return (char *)&uart_buff;
}
/*
uint8_t *get_rebuff(uint8_t *len)
{
    *len = uart_p;
    printf("[get_rebuff]uart_p = %d \r\n",uart_p);
    printf("[get_rebuff]*len = %d \r\n",*len);
    return (uint8_t *)&uart_buff;
}
*/

void clean_rebuff(void)
{
    uint16_t i=UART_BUFF_SIZE;
    uart_p = 0;
    while(i)
        uart_buff[--i]=0;
}
#endif


//*****************************************************************************
//*****************************************************************************

/**********************************************************************
* 函数名: ucharFucCheckSum(uchar *i,ucharln)
* 功能描述:求和校验（取发送、接收协议的1\2\3\4\5\6\7的和取反+1）
* 函数说明:将数组的元素1-倒数第二个元素相加后取反+1（元素个数必须大于2）
**********************************************************************/
unsigned char FucCheckSum(unsigned char *i,unsigned char ln)
{
    unsigned char j,tempq=0;
    i+=1;
    for(j=0;j<(ln-2);j++)
    {
        tempq+=*i;
        i++;
    }
    tempq=(~tempq)+1;
    
    return(tempq);
}


#define     CMDLEN      9
/*氨气浓度传感器初始化
 *
 *串口通过发送 0x78 指令, 把通讯模式更改为 0x04(问答式):
 *    起始位 地址 命令 通讯模式            校验值
 *     FF     01   78     04    00 00 00 00  83 
 * 
 *串口接收氨气浓度传感器的返回数据判定是否修改成功
 *    起始位 命令 返回标定             校验值
 *    FF      78     01   00 00 00 00 00 87           //01 代表成功
*/

//初始化并检测模块: 0表示成功, 1表示失败
/*
int NH3_Sensor_Init(void)
{
    uint8_t i,len;
    uint8_t check_sum;
    uint8_t *ret_data;
    
    uint8_t mode_array[CMDLEN] = 
            {0xFF, 0x01, 0x78, 0x04, 0x00, 0x00, 0x00, 0x00, 0x83};
            
    uint8_t reply_array[CMDLEN] = 
            {0xFF, 0x78, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87};
    
    //NH3_USART_Config();             //初始化串口
    
    clean_rebuff();                 //清空了接收缓冲区数据
    
    //将氨气传感器的通讯模式更改为问答式
    Usart_SendArray(NH3_USARTx, mode_array, CMDLEN);

    //这里需要延时的时间长一点, 否则从串口读取到"氨气传感器发送的值"有问题, 中间的 0x00 数据全部丢失
    //延时1ms -- 不能只延时 1ms
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    //延时 时间不够  会导致 uart_p = 0
    //printf("[NH3_Sensor_Init]uart_p = %d \r\n",uart_p);
    
    //读取串口数据, 判断模式是否修改成功
    ret_data = get_rebuff(&(len));
    
    for(i=0; i<len; i++)
    {
        printf("[NH3_Sensor_Init] ret_data[%d] = %X \r\n", i, ret_data[i]);
    }
    
    //clean_rebuff();                 //清空了接收缓冲区数据
    //这里不能清空接收缓冲区, 否则会影响下面使用 ret_data
    
    if(len == 0)
    {
        //printf("[NH3_Sensor_Init] len=0 \r\n");
        return NH3_FALSE;
    }
    
    check_sum = FucCheckSum(ret_data, CMDLEN);
    
    //clean_rebuff();                 //清空了接收缓冲区数据
    //这里才能清空接收缓冲区, 下面不再使用 ret_data
    
    printf("[NH3_Sensor_Init] check_sum = %X \r\n", check_sum);
    
    if(check_sum == reply_array[CMDLEN-1])
    {
        return NH3_TRUE;
    }
    else
    {
        return NH3_FALSE;
    }
    
    //if (strstr(ret_data,reply_array) != 0)
    //    return NH3_TRUE;
    //else
    //    return NH3_FALSE;
}
*/

/*氨气浓度传感器请求浓度值
 *
 *串口通过发送 0x86 指令, 向氨气传感器请求浓度值:
 *    起始位 地址 命令              校验值
 *    FF      01   86 00 00 00 00 00  79
 *氨气传感器发送当前浓度值, 串口读取传感器浓度
 *    起始位 命令 传感器浓度值               校验值
 *               高字节 低字节
 *    FF      86    00     00      00 00 00 00 7A
*/
/*
int NH3_Sensor_Request()
{
    int NH3_Ret = 0;
    char hig_byte = 0, low_byte = 0;
    
    uint8_t i,len;
    uint8_t check_sum;
    uint8_t *ret_data;
    
    uint8_t request_array[CMDLEN] = 
            {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    
    //这里需要清空缓存
    clean_rebuff();                 //清空了接收缓冲区数据
    
    //向氨气传感器请求浓度值
    Usart_SendArray(NH3_USARTx, request_array, CMDLEN);
    
    //延时1ms -- 这里需要延迟较长时间
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    bsp_DelayUS(100000);
    
    //读取氨气传感器返回的浓度值
    ret_data = get_rebuff(&(len));
    
    for(i=0; i<len; i++)
    {
        printf("[NH3_Sensor_Request] ret_data[%d] = %X \r\n", i, ret_data[i]);
    }
    
    if(len != 0)
    {
        //检查返回数据的正确性
        check_sum = FucCheckSum(ret_data, CMDLEN);
        if(check_sum == ret_data[CMDLEN-1])
        {
            hig_byte = ret_data[2];
            low_byte = ret_data[3];
            
            //气体浓度值=气体浓度高位*256+气体浓度低位
            NH3_Ret = hig_byte*256 + low_byte;
            return NH3_Ret;
        }
    }
    
    return 0;
}
*/


//初始化并检测模块: 0表示成功, 1表示失败
//初始化请求
void NH3_Sensor_Init_Request(void)
{
    uint8_t mode_array[CMDLEN] = 
            {0xFF, 0x01, 0x78, 0x04, 0x00, 0x00, 0x00, 0x00, 0x83};
    
    //NH3_USART_Config();             //初始化串口
    
    clean_rebuff();                 //清空了接收缓冲区数据
    
    //将氨气传感器的通讯模式更改为问答式
    Usart_SendArray(NH3_USARTx, mode_array, CMDLEN);
}

int NH3_Sensor_Init_Check(void)
{
    uint8_t i,len;
    uint8_t check_sum;
    uint8_t *ret_data;
    
    uint8_t reply_array[CMDLEN] = 
            {0xFF, 0x78, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87};
    
    //延时 时间不够  会导致 uart_p = 0
    //printf("[NH3_Sensor_Init]uart_p = %d \r\n",uart_p);
    //读取串口数据, 判断模式是否修改成功
    ret_data = get_rebuff(&(len));
    
    /*
    for(i=0; i<len; i++)
    {
        printf("[NH3_Sensor_Init_Check] ret_data[%d] = %X \r\n", i, ret_data[i]);
    }
    */
    
    if(len == 0)
    {
        clean_rebuff();
        return NH3_FALSE;
    }
    
    check_sum = FucCheckSum(ret_data, CMDLEN);
    if(check_sum == reply_array[CMDLEN-1])
    {
        clean_rebuff();
        return NH3_TRUE;
    }
    else
    {
        clean_rebuff();
        return NH3_FALSE;
    }
}


void NH3_Sensor_Value_Request(void)
{
    uint8_t request_array[CMDLEN] = 
            {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    
    clean_rebuff();                 //清空了接收缓冲区数据
    
    //向氨气传感器请求浓度值
    Usart_SendArray(NH3_USARTx, request_array, CMDLEN);
}

int NH3_Sensor_Value_Read(void)
{
    int NH3_Ret = 0;
    char hig_byte = 0, low_byte = 0;
    
    uint8_t i,len;
    uint8_t check_sum;
    uint8_t *ret_data;
    
    //读取氨气传感器返回的浓度值
    ret_data = get_rebuff(&(len));
    /*
    for(i=0; i<len; i++)
    {
        printf("[NH3_Sensor_Value_Read] ret_data[%d] = %X \r\n", i, ret_data[i]);
    }
    */
    
    if(len != 0)
    {
        //检查返回数据的正确性
        check_sum = FucCheckSum(ret_data, CMDLEN);
        //printf("[NH3_Sensor_Value_Read] check_sum = %X \r\n", check_sum);
        if(check_sum = ret_data[CMDLEN-1])
        {
            hig_byte = ret_data[2];
            low_byte = ret_data[3];
            
            //气体浓度值=气体浓度高位*256+气体浓度低位
            NH3_Ret = hig_byte*256 + low_byte;
            
            clean_rebuff();             //清空了接收缓冲区数据
            return NH3_Ret;
        }
        else
        {
            clean_rebuff();
            return -1;
        }
    }
    clean_rebuff();
    return 0;
}


/*
氨气传感器工作原理:
模组在出厂时配置为主动上传的通讯模式, 模组每隔一秒会对外发送一次当前的浓度值(浓度为 16 进制)
这里采用问答式, STM32串口向氨气传感器请求数据, 氨气传感器发送当前浓度值, STM32再通过串口读出

串口通过发送 0x78 指令, 把通讯模式更改为 0x04(问答式):
    起始位 地址 命令 通讯模式            校验值
    FF      01   78     04    00 00 00 00  83  
串口接收氨气浓度传感器的返回数据判定是否修改成功
    起始位 命令 返回标定             校验值
    FF      78     01   00 00 00 00 00 87           //01 代表成功

模组在收到 0x86 指令(读取模组浓度)后会发送当前的浓度值
    起始位 地址 命令              校验值
    FF      01   86 00 00 00 00 00  79
0x86 读取传感器浓度
    起始位 命令 传感器浓度值               校验值
                高字节 低字节
    FF      86    00     00      00 00 00 00 7A     //浓度值为 0

气体浓度值=气体浓度高位*256+气体浓度低位。
*/