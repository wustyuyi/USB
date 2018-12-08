#ifndef __LED_H__
#define __LED_H__

#include "stm32f10x.h"

#define 		ON 				0
#define 		OFF 			1

#define			HIGH			1
#define			LOW				0

void LED_Init(void);
void LEDx_Toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x);
void LEDx_OnOff(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x, int flag);
void LEDx_Twinkle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x, uint32_t counter, uint32_t flag);

#endif

