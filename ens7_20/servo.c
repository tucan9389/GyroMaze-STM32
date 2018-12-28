#include "stm32f10x.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

void SERVO_Set_Clock() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
}

void SERVO_Set_GPIO() {
    GPIO_InitTypeDef PORTA;
    GPIO_InitTypeDef PORTB;

    PORTA.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_6;
    PORTA.GPIO_Mode = GPIO_Mode_AF_PP;
    PORTA.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &PORTA);

    PORTB.GPIO_Pin = GPIO_Pin_6;
    PORTB.GPIO_Mode = GPIO_Mode_AF_PP;
    PORTB.GPIO_Speed = GPIO_Speed_10MHz;
	  GPIO_Init(GPIOB, &PORTB);

//    // remove if not working
//    PORTB.GPIO_Pin = GPIO_Pin_0;
//    PORTB.GPIO_Mode = GPIO_Mode_Out_PP;
//    PORTB.GPIO_Speed = GPIO_Speed_10MHz;
//    GPIO_Init(GPIOB, &PORTB);
}

void SERVO_Set_Timer2() {
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef OutputChannel;

    /* TIM2 Clock Enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);


    /* Enable TIM2 Global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* TIM2 Initialize */
    TIM_TimeBaseStructure.TIM_Period=2000-1; // 100micro second //100kHz
    TIM_TimeBaseStructure.TIM_Prescaler=720-1;
    TIM_TimeBaseStructure.TIM_ClockDivision=0;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* TIM2 PWM Initialize */
    OutputChannel.TIM_OCMode = TIM_OCMode_PWM1;
    OutputChannel.TIM_OutputState = TIM_OutputState_Enable;
    OutputChannel.TIM_OutputNState = TIM_OutputNState_Enable;
    OutputChannel.TIM_Pulse = 50-1; // 50% duty ratio
    OutputChannel.TIM_OCPolarity = TIM_OCPolarity_Low;
    OutputChannel.TIM_OCNPolarity = TIM_OCNPolarity_High;
    OutputChannel.TIM_OCIdleState = TIM_OCIdleState_Set;
    OutputChannel.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    TIM_OC1Init(TIM2, &OutputChannel);
    TIM_OC1Init(TIM3, &OutputChannel);
    TIM_OC1Init(TIM4, &OutputChannel);

    /* TIM2 Enale */
    TIM_Cmd(TIM3,ENABLE);
    TIM_Cmd(TIM2,ENABLE);
    TIM_Cmd(TIM4,ENABLE);
    TIM_ITConfig(TIM3, TIM_IT_Update | TIM_IT_CC1 ,ENABLE); // interrupt enable
    TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_CC1 ,ENABLE); // interrupt enable
    TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_CC1 ,ENABLE); // interrupt enable
}

void SERVO_Init(void) {
    SERVO_Set_Clock();
    SERVO_Set_GPIO();
    SERVO_Set_Timer2();
}

// servo function1
void setMyServo(int degree) {
    int cycle = 2000;
    int minCycle = 60;
    int maxCycle = 240;

    int lTime = (double)(cycle - (float)(maxCycle - minCycle) * (float)(degree / 180.0) - minCycle);
    int hTime = cycle - lTime;

    TIM2->CCR1 = lTime;
}

void setServo(volatile uint16_t *ccr, int degree) {
  int cycle = 2000;
  int minCycle = 60;
  int maxCycle = 240;

  int lTime = (double)(cycle - (float)(maxCycle - minCycle) * (float)(degree / 180.0) - minCycle);
  int hTime = cycle - lTime;

  *ccr = lTime;
}

void setThreeServo(int degree1, int degree2, int degree3) {
  // setServo(&(TIM2->CCR1), degree1);
  // setServo(&(TIM2->CCR2), degree2);
  // setServo(&(TIM2->CCR3), degree3);
}

void setServo1(int degree) {
  setServo(&(TIM2->CCR1), degree);
}

void setServo2(int degree) {
  setServo(&(TIM3->CCR1), degree);
}

void setServo3(int degree) {
  setServo(&(TIM4->CCR1), degree);
}
