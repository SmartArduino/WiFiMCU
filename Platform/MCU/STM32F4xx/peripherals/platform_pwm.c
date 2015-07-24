/**
******************************************************************************
* @file    platform_pwm.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide PWM driver functions.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/ 


#include "MICORTOS.h"
#include "MICOPlatform.h"

#include "platform.h"
#include "platform_peripheral.h"
#include "debug.h"
/******************************************************
*                    Constants
******************************************************/

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/

/******************************************************
*                    Structures
******************************************************/

/******************************************************
*               Variables Definitions
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/

/******************************************************
*               Function Definitions
******************************************************/


void TIM14_PWM_Init(uint32_t arr,uint32_t psc)
{		 					 
	//此部分需手动修改IO口设置
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  	//TIM2时钟使能    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 	//使能PORTF时钟	
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_TIM2); //GPIOB10复用为定时器2
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;           //GPIOF9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉
	GPIO_Init(GPIOF,&GPIO_InitStructure);              //初始化PF9
	  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr;   //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);//初始化定时器14
	
	//初始化TIM2 Channel3 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性低

	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM2OC1
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
 
        TIM_ARRPreloadConfig(TIM2,ENABLE);//ARPE使能 
	
	TIM_Cmd(TIM2, ENABLE);  //使能TIM14
 
										  
}  

void pwmTest()
{
        TIM14_PWM_Init(500-1,120-1);
        uint16_t led0pwmval=500;
        TIM_SetCompare1(TIM14,led0pwmval);
}

#include "lua.h"
OSStatus platform_pwm_init( const platform_pwm_t* pwm, uint32_t frequency, float duty_cycle )
{
  TIM_TimeBaseInitTypeDef tim_time_base_structure;
  TIM_OCInitTypeDef       tim_oc_init_structure;
  RCC_ClocksTypeDef       rcc_clock_frequencies;
  uint16_t                period              = 0;
  float                   adjusted_duty_cycle = ( ( duty_cycle > 100.0f ) ? 100.0f : duty_cycle );
  OSStatus                err                 = kNoErr;
  
  require_action_quiet( pwm != NULL, exit, err = kParamErr);

  platform_mcu_powersave_disable();

  RCC_GetClocksFreq( &rcc_clock_frequencies );
  
  if ( pwm->tim == TIM1 || pwm->tim == TIM8 || pwm->tim == TIM9 || pwm->tim == TIM10 || pwm->tim == TIM11 )
  {
    RCC_APB2PeriphClockCmd( pwm->tim_peripheral_clock, ENABLE );
    period = (uint16_t)( rcc_clock_frequencies.PCLK2_Frequency / frequency - 1 ); /* Auto-reload value counts from 0; hence the minus 1 */
  }
  else
  {
    RCC_APB1PeriphClockCmd( pwm->tim_peripheral_clock, ENABLE );
    period = (uint16_t)( rcc_clock_frequencies.PCLK1_Frequency / frequency - 1 ); /* Auto-reload value counts from 0; hence the minus 1 */
  }
  
  /* Enable peripheral clock for this port */
    err = platform_gpio_enable_clock( pwm->pin );
    require_noerr(err, exit);
    
  /* Set alternate function */
  platform_gpio_set_alternate_function( pwm->pin->port, pwm->pin->pin_number, GPIO_OType_PP, GPIO_PuPd_UP, pwm->gpio_af );
  
  /* Time base configuration */
  tim_time_base_structure.TIM_Period            = (uint32_t) period;
  tim_time_base_structure.TIM_Prescaler         = (uint16_t) 1;  /* Divide clock by 1+1 to enable a count of high cycle + low cycle = 1 PWM cycle */
  tim_time_base_structure.TIM_ClockDivision     = 0;
  tim_time_base_structure.TIM_CounterMode       = TIM_CounterMode_Up;
  tim_time_base_structure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit( pwm->tim, &tim_time_base_structure );
  
  /* PWM1 Mode configuration */
  tim_oc_init_structure.TIM_OCMode       = TIM_OCMode_PWM1;
  tim_oc_init_structure.TIM_OutputState  = TIM_OutputState_Enable;
  tim_oc_init_structure.TIM_OutputNState = TIM_OutputNState_Enable;
  tim_oc_init_structure.TIM_Pulse        = (uint16_t) ( adjusted_duty_cycle * (float) period / 100.0f );
  tim_oc_init_structure.TIM_OCPolarity   = TIM_OCPolarity_High;
  tim_oc_init_structure.TIM_OCNPolarity  = TIM_OCNPolarity_High;
  tim_oc_init_structure.TIM_OCIdleState  = TIM_OCIdleState_Reset;
  tim_oc_init_structure.TIM_OCNIdleState = TIM_OCIdleState_Set;
  
  switch ( pwm->channel )
  {
  case 1:
    {
      TIM_OC1Init( pwm->tim, &tim_oc_init_structure );
      TIM_OC1PreloadConfig( pwm->tim, TIM_OCPreload_Enable );
      break;
    }
  case 2:
    {
      TIM_OC2Init( pwm->tim, &tim_oc_init_structure );
      TIM_OC2PreloadConfig( pwm->tim, TIM_OCPreload_Enable );
      break;
    }
  case 3:
    {
      TIM_OC3Init( pwm->tim, &tim_oc_init_structure );
      TIM_OC3PreloadConfig( pwm->tim, TIM_OCPreload_Enable );
      break;
    }
  case 4:
    {
      TIM_OC4Init( pwm->tim, &tim_oc_init_structure );
      TIM_OC4PreloadConfig( pwm->tim, TIM_OCPreload_Enable );
      break;
    }
  default:
    {
      break;
    }
  }

  //TIM_ARRPreloadConfig(pwm->tim,ENABLE);//enable ARPE
  
exit:  
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_pwm_start( const platform_pwm_t* pwm )
{
  OSStatus err = kNoErr;
  
  platform_mcu_powersave_disable();

  require_action_quiet( pwm != NULL, exit, err = kParamErr);
  
  TIM_Cmd( pwm->tim, ENABLE );
  TIM_CtrlPWMOutputs( pwm->tim, ENABLE );
  
exit:  
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_pwm_stop( const platform_pwm_t* pwm )
{
  OSStatus err = kNoErr;
  
  platform_mcu_powersave_disable();

  require_action_quiet( pwm != NULL, exit, err = kParamErr);
  
  TIM_CtrlPWMOutputs( pwm->tim, DISABLE );
  TIM_Cmd( pwm->tim, DISABLE );
  
exit:  
  platform_mcu_powersave_enable();
  return err;
}




