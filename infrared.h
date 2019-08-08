#ifndef _Infrared_H
#define _Infrared_H

/*
硬件配置：
使用定时器输入捕获功能驱动红外接收
定时器14 通道1 下降沿触发
晶振48M 分频48 周期10000
使用方法：
添加初始化函数Infrared_Init
添加主循环函数Infrared_main_while
在接收到信号时会触发调用Infrared_Rec_data在里面根据按键值做处理
*/
#include "stm32f0xx_hal.h"
#include "main.h"

#define INFRA_TIM_REG					TIM14
#define INFRA_HTIM 						htim14
#define TIM_CH_NUM						1
#define INFRA_TIM_CH 					TIM_CHANNEL_1
#define RangJudge(val,min,max)  	(val > min ? (val < max ? 1 : 0) : 0)

/*  长按至松开的检测间隔  */
#define INFRARED_KEY_CHECK_MS			20//ms
 
typedef struct
{
	uint32_t	TriPolarity;
	uint32_t	FrameStart;
	uint32_t	TriTime[2];  //记录电平持续时间 0：低电平  1:高电平
    uint32_t	Key_Tmp;   //记录32位地址以及按键码
	uint32_t	Data_Bit;
	uint32_t	Key_Cnt;	
	uint32_t 	Key_Cnt_last;
	uint32_t	Rec_Flag;
	uint32_t	Key_Num;
	uint32_t	Key_Addr;
	uint32_t 	Key_check_ms_cnt;
}Infrared_data_t;
 
extern Infrared_data_t  *Infrared_data;

void Infrared_Init(void);
void Infrared_ms_it(void);
void Infrared_Rec_data(void);
void Infrared_main_while(void);
uint8_t  Infrared_Get_Key_Num(void);
uint8_t  Infrared_Get_Key_Addr(void);
uint32_t Infrared_Get_Key_Cnt(void);
void Infrared_TIM_Period(void);
void Infrared_TIM_Capture(void);
void TIM_OC1PolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPolarity);

#endif
