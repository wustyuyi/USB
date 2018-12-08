#include "led.h"
#include "sw_delay.h"


void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);	     //使能PB端口时钟
    
    PWR_BackupAccessCmd(ENABLE);
    RCC_LSEConfig(RCC_LSE_OFF);
    BKP_TamperPinCmd(DISABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //LED-->PB.9 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		                         //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC,GPIO_Pin_13);	
    GPIO_SetBits(GPIOC,GPIO_Pin_14);	
    GPIO_SetBits(GPIOC,GPIO_Pin_15);	
    
    PWR_BackupAccessCmd(DISABLE);
}
 

/**
  * @brief  LEDx_Toggle
  * @param  
  * @retval None
  */
void LEDx_Toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x)
{
     GPIO_WriteBit(GPIOx, GPIO_Pin_x, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOx, GPIO_Pin_x)));
}


/**
  * @brief  LED_OnOff
  * @param  
  * @retval None
  */
void LEDx_OnOff(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x, int flag)
{
	LED_Init();	
	if (flag == ON)		
	{
        GPIO_ResetBits(GPIOx, GPIO_Pin_x);
    } 
	else if(flag == OFF) 
	{
        GPIO_SetBits(GPIOx, GPIO_Pin_x);
    }
}


/**
  * @brief  LEDx_Twinkle
  * @param  
  * @retval None
  */
void LEDx_Twinkle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x, uint32_t counter, uint32_t flag)
{
	LEDx_OnOff(GPIOx, GPIO_Pin_x, ON);
	while(counter--) 
	{
		LEDx_Toggle(GPIOx, GPIO_Pin_x);
		sw_delay_ms(flag);
	}
	LEDx_OnOff(GPIOx, GPIO_Pin_x, OFF);	 
}



