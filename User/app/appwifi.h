#ifndef _APP_WIFI_H
#define _APP_WIFI_H

#include "includes.h"
#include  <os.h>
#include  <stm32f10x.h>

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

typedef struct {
    uint8_t head;
    uint16_t data_length;
    uint8_t mode;
    uint8_t data_info[16];
    uint8_t check_data;
}DevInfo;

typedef struct {
    uint8_t head;
    uint16_t data_length;
    uint8_t mode;
    uint8_t data_info[11];
    uint8_t check_data;
}FaultInfo;

#define BOOL uint8_t
#define True 0x01
#define False 0x00

extern uint8_t rxBuffer[100];
extern uint8_t idleFlag;
extern uint8_t rxCounter;

uint8_t GetDeviceSwitch();
uint8_t GetWorkingMode();
uint8_t GetAtomizerStatus();
uint8_t GetAirVolumeStatus();
uint8_t GetConcentrationStatus();
uint8_t GetWashingStatus();
uint8_t GetDeviceRunningStatus();
uint8_t GetNH3Value();
uint8_t GetNH3Value();
BOOL IsFanAAbnormal();
BOOL IsFanBAbnormal();
BOOL IsAtomizerAAbnormal();
BOOL IsAtomizerBAbnormal();
BOOL IsPumpAAbnormal();
BOOL IsPumpBAbnormal();
BOOL IsValveAAbnormal();
BOOL IsValveBAbnormal();
BOOL IsNH3SensorAbnormal();
BOOL IsLiquidBoxAbnormal();
BOOL IsWorkBoxAbnormal();
uint8_t GetCustomMode();
int GetCustomTime1();
int GetCustomTime2();
int GetCustomTime3();
int GetCustomTime4();
int GetCustomTime5();
int GetCustomTime6();
int GetCustomTime7();
int GetCustomTime8();
int GetCustomTime9();
int GetCustomTime10();
uint8_t GetAtomizerRunningTime();
uint8_t GetAtomizerStopTime();
unsigned char gen_verify_code(char *buf, int len);
uint8_t GetDeviceVerifyCode(DevInfo devInfo, int num);
uint8_t GetFaultVerifyCode(FaultInfo faultInfo, int num);
uint8_t GetWifiState();
void SendDevInfo(DevInfo devInfo);
void SendFaultInfo(FaultInfo faultInfo);
void UploadFaultInfo();
void TriggleWifi();
void SetupDevice();
void UploadDeviceInfo(uint8_t insCode);
void CustomizeSetup();
void CustomizeQuery();
void RefreshWifiStatus();
#endif