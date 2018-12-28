#include "stm32f10x.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"


void LED_Set_Clock(void) {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
}

void LED_Set_GPIO(void) {
  GPIO_InitTypeDef gpio;
  // LED Output Mode
  gpio.GPIO_Mode = GPIO_Mode_Out_PP;
  gpio.GPIO_Pin=(GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7);
  gpio.GPIO_Speed = GPIO_Speed_50MHz; // 주파수
  GPIO_Init(GPIOD, &gpio);
}

void LED_Init(void) {
  LED_Set_Clock();
  LED_Set_GPIO();
}

void LED_OnOff(int index, int on) {

 	if (on) {
    GPIO_ResetBits(GPIOD, GPIO_Pin_2);
   	GPIO_ResetBits(GPIOD, GPIO_Pin_3);
   	GPIO_ResetBits(GPIOD, GPIO_Pin_4);
   	GPIO_ResetBits(GPIOD, GPIO_Pin_7);
 		if (index == 0) {
 			GPIO_SetBits(GPIOD, GPIO_Pin_2);
 		} else if (index == 1) {
 			GPIO_SetBits(GPIOD, GPIO_Pin_3);
 		} else if (index == 2) {
 			GPIO_SetBits(GPIOD, GPIO_Pin_4);
 		} else if (index == 3) {
 			GPIO_SetBits(GPIOD, GPIO_Pin_7);
 		} else {

 		}
 	} else {
    if (index == 0) {
 			GPIO_ResetBits(GPIOD, GPIO_Pin_2);
 		} else if (index == 1) {
 			GPIO_ResetBits(GPIOD, GPIO_Pin_3);
 		} else if (index == 2) {
 			GPIO_ResetBits(GPIOD, GPIO_Pin_4);
 		} else if (index == 3) {
 			GPIO_ResetBits(GPIOD, GPIO_Pin_7);
 		} else {

 		}
  }
}
