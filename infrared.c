#include "infrared.h"

Infrared_data_t  Infrared_data_S, *Infrared_data = &Infrared_data_S;

/**********************************************************************************************************
*	函 数 名: Infrared_Init
*	功能说明: 初始化
*	传    参: 
*	返 回 值: 找到的参数
*	说    明: 
*********************************************************************************************************/
void Infrared_Init(void)
{
    	HAL_TIM_IC_Start_IT(&INFRA_HTIM, INFRA_TIM_CH);
	__HAL_TIM_ENABLE_IT(&INFRA_HTIM, TIM_IT_UPDATE);  //使能更新中断
	
	Infrared_data->TriPolarity = 0;
	Infrared_data->FrameStart = 0;
}
/**********************************************************************************************************
*	函 数 名: Infrared_ms_it
*	功能说明: 放进主循环进行状态判断
*	传    参: 
*	返 回 值: 
*	说    明: 
*********************************************************************************************************/
void Infrared_ms_it(void)
{
  	if(Infrared_data->Rec_Flag == 1)
	{
		Infrared_data->Key_check_ms_cnt++;
		if(Infrared_data->Key_check_ms_cnt > INFRARED_KEY_CHECK_MS)
		{
			Infrared_data->Key_check_ms_cnt = 0;
			Infrared_data->Rec_Flag = 2;
		}
	}
	
}
/**********************************************************************************************************
*	函 数 名: Infrared_Rec_data
*	功能说明: 接收到红外信号后触发事件得到按键码
*	传    参: 
*	返 回 值: 
*	说    明: 
*********************************************************************************************************/
void Infrared_Rec_data(void)
{
	Infrared_data->Key_Num = Infrared_Get_Key_Addr();
	Infrared_data->Key_Addr = Infrared_Get_Key_Num();
	Infrared_data->Key_Cnt = Infrared_Get_Key_Cnt();
	
	/*  长按过程中一直会执行  */
	if(Infrared_data->Key_Cnt != Infrared_data->Key_Cnt_last)
	{
		Infrared_data->Rec_Flag = 1;
		Infrared_data->Key_Cnt_last = Infrared_data->Key_Cnt;
	}
	else
	{
		/*  松开后执行  */
		Infrared_data->Rec_Flag = 0;
	}
}

/**********************************************************************************************************
*	函 数 名: Infrared_main_while
*	功能说明: 放在主循环里判断接收状态
*	传    参: 
*	返 回 值: 
*	说    明: 
*********************************************************************************************************/
void Infrared_main_while(void)
{
	if(Infrared_data->Rec_Flag != 2)
		return;
	Infrared_Rec_data();
	
}

/**********************************************************************************************************
*	函 数 名: Infrared_Get_Key_Num
*	功能说明: 校验并获取按键值
*	传    参: 
*	返 回 值: 按键值
*	说    明: 
*********************************************************************************************************/
uint8_t  Infrared_Get_Key_Num(void)
{
   	//通过与反码异或，验证按键码的正确性
   	if((uint8_t)((Infrared_data->Key_Tmp >> 24) ^ (Infrared_data->Key_Tmp >> 16)) == 0xff)
   	return (uint8_t)(Infrared_data->Key_Tmp >> 16);
	 
   	else  return 0;
}
 
/**********************************************************************************************************
*	函 数 名: Infrared_Get_Key_Addr
*	功能说明: 校验并获取地址值
*	传    参: 
*	返 回 值: 地址值
*	说    明: 
*********************************************************************************************************/
uint8_t  Infrared_Get_Key_Addr(void)
{
   	if((uint8_t)((Infrared_data->Key_Tmp ) ^ (Infrared_data->Key_Tmp >> 8)) == 0xff)
   	return (uint8_t)(Infrared_data->Key_Tmp);
	 
   	else  return 0;
}

/**********************************************************************************************************
*	函 数 名: Infrared_Get_Key_Cnt
*	功能说明: 校验并获取按键长按等效次数
*	传    参: 
*	返 回 值: 按键长按等效次数
*	说    明: 
*********************************************************************************************************/
uint32_t Infrared_Get_Key_Cnt(void)
{
	return Infrared_data->Key_Cnt;
}
 
/**********************************************************************************************************
*	函 数 名: Infrared_TIM_Period
*	功能说明: 更新中断处理函数
*	传    参: 
*	返 回 值: 
*	说    明: 
*********************************************************************************************************/
void Infrared_TIM_Period(void)
{
	Infrared_data->TriPolarity = 0;
    	Infrared_data->FrameStart = 0;
	Infrared_data->TriTime[0] = 0;
	Infrared_data->TriTime[1] = 0;
}
 
/**********************************************************************************************************
*	函 数 名: Infrared_TIM_Capture
*	功能说明: 捕获中断处理函数
*	传    参: 
*	返 回 值: 
*	说    明: 
*********************************************************************************************************/
void Infrared_TIM_Capture(void)
{

	Infrared_data->TriPolarity = !Infrared_data->TriPolarity;
	
	/************************** 切换极性以及捕获电平持续时间 **************************/
	
	if(!Infrared_data->TriPolarity)  //上升沿触发
	{
		TIM_OC1PolarityConfig(INFRA_TIM_REG, TIM_INPUTCHANNELPOLARITY_FALLING);			
		
		Infrared_data->TriTime[Infrared_data->TriPolarity] = \
					HAL_TIM_ReadCapturedValue(&INFRA_HTIM, INFRA_TIM_CH);	
		//Infrared_data->TriTime[Infrared_data->TriPolarity]++;
		/*  此时接收的是同步码低电平  */
		/*if(Infrared_data->FrameStart == 1) 
		{
			Infrared_data->FrameStart = 2;
		}*/
		/*  此时接收的是数据码低电平  */
		/*if(Infrared_data->FrameStart == 3) 
		{
			Infrared_data->FrameStart = 4;
		}*/
	}
	else  //下降沿触发
	{
		TIM_OC1PolarityConfig(INFRA_TIM_REG, TIM_INPUTCHANNELPOLARITY_RISING);
		
		if(Infrared_data->FrameStart != 0) 
		{
			Infrared_data->TriTime[Infrared_data->TriPolarity] = \
			HAL_TIM_ReadCapturedValue(&INFRA_HTIM, INFRA_TIM_CH);
			//Infrared_data->TriTime[Infrared_data->TriPolarity]++;
		}	
		
		if(Infrared_data->FrameStart == 0)
		{
			Infrared_data->FrameStart = 1;	//开始接收帧
		}
		/*  此时接收的是同步码高电平  */
		/*if(Infrared_data->FrameStart == 2) 
		{
		  Infrared_data->FrameStart = 3;
		}*/
		/*  此时接收的是数据码高电平  */
		/*if(Infrared_data->FrameStart == 4) 
		{
			Infrared_data->FrameStart = 5;
		}*/
	}	
	
	INFRA_TIM_REG->CNT = 0; 	//清除INFRA_TIM_CH计数值
	
	/**************************** 判断同步码以及重复码 ***************************/
		
	if(RangJudge(Infrared_data->TriTime[0],8500,9500) && \
			RangJudge(Infrared_data->TriTime[1],4000,5500) ) 
	{
		Infrared_data->Key_Tmp = 0;  //同步码
		Infrared_data->Key_Cnt = 0;  
		Infrared_data->Data_Bit = 0;  		
	}		
	else if(RangJudge(Infrared_data->TriTime[0],8500,9500) && \
					RangJudge(Infrared_data->TriTime[1],2000,3000) ) 
	{
		if(++Infrared_data->Key_Cnt > 250) Infrared_data->Key_Cnt = 250;    //连发码
	}
	
	/********************************* 接收数据 *********************************/		
	/*  8位地址码 8位地址反码 8位数据码 8位数据反码  */
	if( Infrared_data->TriPolarity == 1) 
	{
		if(RangJudge(Infrared_data->TriTime[0],450,650) && \
					RangJudge(Infrared_data->TriTime[1],450,650) ) 
		{
			Infrared_data->Key_Tmp &= ~(1 << Infrared_data->Data_Bit++);
		}
		
		else if(RangJudge(Infrared_data->TriTime[0],450,650) && \
						RangJudge(Infrared_data->TriTime[1],1450,1750) ) 
		{
			Infrared_data->Key_Tmp |= (1 << Infrared_data->Data_Bit++);
		}
	}	
	/*  接收完成  */
	if(Infrared_data->Data_Bit >= 32)
	{
		Infrared_data->Data_Bit = 0;
		Infrared_data->Rec_Flag = 1;
	}
	
}

/**********************************************************************************************************
*	函 数 名: TIM_OC1PolarityConfig
*	功能说明: 改变捕获极性
*	传    参: TIMx 定时器 TIM_OCPolarity 极性
*	返 回 值: 
*	说    明: 
*********************************************************************************************************/
void TIM_OC1PolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPolarity)
{
  	uint16_t tmpccer = 0;
 
  	tmpccer = TIMx->CCER;
  	/*  和通道有关  */
  	/*  TIM_CCER_CC1P  通道  */
  	tmpccer &= (uint16_t)~((uint16_t)(1 << TIM_CH_NUM));
  	/*  TIM_CCER_CC1E_Pos == 0  */
  	tmpccer |= (uint16_t)(TIM_OCPolarity << (TIM_CH_NUM - 1));
 
  	TIMx->CCER = tmpccer;
}
 
/**********************************************************************************************************
*	函 数 名: HAL_TIM_PeriodElapsedCallback
*	功能说明: HAL库更新中断入口
*	传    参: htim 定时器句柄
*	返 回 值: 
*	说    明: 
*********************************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    	if(htim->Instance == INFRA_TIM_REG)
	{
		Infrared_TIM_Period();
	}
}
 
/**********************************************************************************************************
*	函 数 名: HAL_TIM_IC_CaptureCallback
*	功能说明: HAL库捕获中断入口
*	传    参: htim 定时器句柄
*	返 回 值: 
*	说    明: 
*********************************************************************************************************/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    	if(htim->Instance == INFRA_TIM_REG)
	{
		Infrared_TIM_Capture();
	}
}
