/**
  ******************************************************************************
  * @file    bsp_internalFlash.c
  * @author  huakai.han
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   内部FLASH读写
  ******************************************************************************
  * @attention
  * 操作Flash步骤：
  *     1.unlock Flash
  *     2.擦除指定的页(查看工程空间分布，以防擦除程序本身)
  *     3.写入数据
  *     4.读出数据校验
  ******************************************************************************
  */

#include "bsp_internal_flash.h" 

/* 
读内部flash的指定地址的N个字节；
    ReadAddress：指定地址
    ReadBuf：数据存储指针
    ReadNum：读取字节数
*/ 

void ReadFlashNBtye(uint32_t ReadAddress, uint8_t *ReadBuf, int32_t ReadNum) 
{ 
    int DataNum = 0; 
    ReadAddress = (uint32_t)CONF_START_ADDR + ReadAddress; 
    
    while(DataNum < ReadNum) 
    { 
        *(ReadBuf + DataNum) = *(__IO uint8_t*) ReadAddress++; 
        DataNum++; 
    } 
    
    //return DataNum; 
} 



//********************************* 测试函数 ***************************************
//********************************* 测试函数 ***************************************
//测试结构体数据
typedef struct {
    int power_onoff;
    int liquid_density;
    char err_code_bitmap;
    char work_mode;
} test_cfg_st;
//上述结构 OK -- 四字节对齐
/*
typedef struct {
    char power_onoff;
    int liquid_density;
    int err_code_bitmap;
    char work_mode;
} test_cfg_st;
mp_cfg.power_onoff = 0;
tmp_cfg.liquid_density = 66;
tmp_cfg.err_code_bitmap = 89;
tmp_cfg.work_mode = 15;

cfg_get.power_onoff = 0 
cfg_get.liquid_density = 66 
cfg_get.err_code_bitmap = 89 
cfg_get.work_mode = 97

//最后一个数据错误

//尽量保证四字节对齐，防止写入出错

//OK 
typedef struct {
    char power_onoff;
    int liquid_density;
    char err_code_bitmap;
    char work_mode;
} test_cfg_st;


//这种结构体 OK 
typedef struct {
    char power_onoff;
    char liquid_density;
    char err_code_bitmap;
    char work_mode;
} test_cfg_st;
*/

void test_write_flash()
{
    uint32_t EraseCounter = 0x00;   //记录要擦除多少页
    uint32_t Address = 0x00;        //记录写入的地址
    uint32_t NbrOfPage = 0x00;      //记录写入多少页
    FLASH_Status FLASHStatus = FLASH_COMPLETE;  //记录每次擦除的结果
    
    int i;
    //类型
    unsigned char temp_test_cfg[512];
    unsigned char temp[4] = {'\0'};
    
    test_cfg_st tmp_cfg;
    tmp_cfg.power_onoff = 3;
    tmp_cfg.liquid_density = 66;
    tmp_cfg.err_code_bitmap = 89;
    tmp_cfg.work_mode = 15;
    
    memset(temp_test_cfg, 0, sizeof(temp_test_cfg));
    //类型
    memcpy(temp_test_cfg, (unsigned char *)&tmp_cfg, sizeof(test_cfg_st));
    for(i=0; i<20; i++)
        printf("temp_test_cfg[%d] = 0x%X \r\n",i,temp_test_cfg[i]);
    
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
    
    /*
    while((Address < WRITE_END_ADDR) && (FLASHStatus == FLASH_COMPLETE))
    {
        FLASHStatus = FLASH_ProgramWord(Address, Data);
        Address = Address + 4;
    }
    */
    for(i=0; (i<(512/4)) && (FLASHStatus == FLASH_COMPLETE); i++)
    {
        memset(temp, 0, sizeof(temp));
        memcpy(temp, temp_test_cfg+4*i, sizeof(temp));
        printf("*(int*)temp = %d \r\n", *(int*)temp);	//这样转换是否会有 "存储大小端序" 问题？？
        FLASHStatus = FLASH_ProgramWord(Address, *(int*)temp);
        //FLASHStatus = FLASH_ProgramWord(Address, 0x1234);
        Address = Address + 4;
    }
    FLASH_Lock();
    
    //这里没有验证写入的正确性
    //结束 ************************************************************
}

void tets_flash_func()
{
    int i;
    unsigned char temp[10] = {'\0'};
    test_cfg_st cfg_get;
    
    //写 Flash
    test_write_flash();
    
    //读 Flash
    ReadFlashNBtye(0, (uint8_t*)temp, sizeof(temp));
    for(i=0; i<10; i++)
    {
        printf("temp[%d] = 0x%X \r\n", i, temp[i]);
    }
    
    printf("************************************* \r\n");
    
    memcpy((unsigned char *)&cfg_get, temp, sizeof(test_cfg_st));
    
    printf("cfg_get.power_onoff = %d \r\n", cfg_get.power_onoff);
    printf("cfg_get.liquid_density = %d \r\n", cfg_get.liquid_density);
    printf("cfg_get.err_code_bitmap = %d \r\n", cfg_get.err_code_bitmap);
    printf("cfg_get.work_mode = %d \r\n", cfg_get.work_mode);
}
//********************************* 测试函数 ***************************************
//********************************* 测试函数 ***************************************



//appfunc.c 中 read_cfg_from_flash 和 write_cfg_to_flash 参考原项目的实现函数如下:
/*
char read_cfg_from_flash()
{
    char temp[1000];

    if(ReadFile(AIR_CLEAN_CFG_FILE, (char *)temp, sizeof(air_clean_cfg_st))<=0)
    {
        printf("read error.\r\n");
    }
    memcpy((char *)&air_clean_cfg,temp,sizeof(air_clean_cfg_st));
}
char write_cfg_to_flash()
{
    char temp[1000];

    memcpy(temp,(char *)&air_clean_cfg,sizeof(air_clean_cfg_st));
    printf("cfglen %d\n\r",sizeof(air_clean_cfg_st));
    SaveFile(AIR_CLEAN_CFG_FILE, (char *)temp, sizeof(air_clean_cfg_st));
}
*/


//原示例程序测试函数:
/**
  * @brief  InternalFlash_Test,对内部FLASH进行读写测试
  * @param  None
  * @retval None
  */
/*
#define WRITE_START_ADDR  ((uint32_t)0x08008000)
#define WRITE_END_ADDR    ((uint32_t)0x0800C000)
int InternalFlash_Test(void)
{
    uint32_t EraseCounter = 0x00;   //记录要擦除多少页
    uint32_t Address = 0x00;        //记录写入的地址
    uint32_t Data = 0x3210ABCD;     //记录写入的数据
    uint32_t NbrOfPage = 0x00;      //记录写入多少页

    FLASH_Status FLASHStatus = FLASH_COMPLETE;  //记录每次擦除的结果
    RWFlashStatus MemoryProgramStatus = PASSED; //记录整个测试结果

    //解锁
    FLASH_Unlock();

    printf("WRITE_START_ADDR = 0x%x \r\n", WRITE_START_ADDR);
    //计算要擦除多少页
    NbrOfPage = (WRITE_END_ADDR - WRITE_START_ADDR) / FLASH_PAGE_SIZE;

    //清空所有标志位
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    //按页擦除
    for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        FLASHStatus = FLASH_ErasePage(WRITE_START_ADDR + (FLASH_PAGE_SIZE * EraseCounter));
    }

    //向内部FLASH写入数据
    Address = WRITE_START_ADDR;

    while((Address < WRITE_END_ADDR) && (FLASHStatus == FLASH_COMPLETE))
    {
        FLASHStatus = FLASH_ProgramWord(Address, Data);
        Address = Address + 4;
    }

    FLASH_Lock();

    //检查写入的数据是否正确
    Address = WRITE_START_ADDR;

    while((Address < WRITE_END_ADDR) && (MemoryProgramStatus != FAILED))
    {
        if((*(__IO uint32_t*) Address) != Data)
        {
            MemoryProgramStatus = FAILED;
        }
        Address += 4;
    }
    
    return MemoryProgramStatus;
}
*/
