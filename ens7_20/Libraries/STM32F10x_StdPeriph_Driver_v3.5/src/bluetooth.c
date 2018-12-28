
// flash load C:\Users\Team07\week08\ens7_9\Debug\flashclear.axf
// flash load C:\Users\Team07\week08\ens7_9\Debug\ens7_7.axf


#include "stm32f10x.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

#include "bluetooth.h"

typedef int bool;

#define true 1
#define false 0

void BT_Set_Clock(void) {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
    RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, // AFIO가 인터럽트
    ENABLE);

 RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}

void BT_Set_GPIO(void) {
  GPIO_InitTypeDef gpio;
  // USART1 output Mode
  gpio.GPIO_Mode = GPIO_Mode_AF_PP;
  gpio.GPIO_Pin=(GPIO_Pin_9);
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &gpio);

  //USART1 input Mode
  gpio.GPIO_Mode = GPIO_Mode_IPD;
  gpio.GPIO_Pin=(GPIO_Pin_10);
  GPIO_Init(GPIOA, &gpio);

 // USART2 output Mode
  gpio.GPIO_Mode = GPIO_Mode_AF_PP;
  gpio.GPIO_Pin=(GPIO_Pin_2);
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &gpio);

  // USART2 input Mode
  gpio.GPIO_Mode = GPIO_Mode_IPD; // 확인 필요
  gpio.GPIO_Pin=(GPIO_Pin_3);
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &gpio);
}

void BT_Set_USART(void) {
  // set usart
  USART_InitTypeDef usart1;
  USART_InitTypeDef usart2;

  usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart1.USART_BaudRate = 9600; // USART1
  usart1.USART_WordLength = USART_WordLength_8b;
  usart1.USART_StopBits=USART_StopBits_1;
  usart1.USART_Parity=USART_Parity_No;
  usart1.USART_Mode = (USART_Mode_Rx | USART_Mode_Tx);


  usart2.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart2.USART_BaudRate = 9600; // USART2
  usart2.USART_WordLength = USART_WordLength_8b;
  usart2.USART_StopBits=USART_StopBits_1;
  usart2.USART_Parity=USART_Parity_No;
  usart2.USART_Mode = (USART_Mode_Rx | USART_Mode_Tx);


  USART_Init(USART1, &usart1);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  USART_Init(USART2, &usart2);
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

  EXTI_ClearITPendingBit(USART_IT_RXNE);

  USART_Cmd(USART1, ENABLE);
  USART_Cmd(USART2, ENABLE);
}

//  인터럽트시 우선순위 세팅
// 인터럽트 라인이 있음. 핀이랑 1:1로 매핑
// 1~4 1:1
void BT_Set_NVIC(void) {
   NVIC_InitTypeDef nvic;

   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

   /* RX NVIC Channel  */
   nvic.NVIC_IRQChannel = USART1_IRQn; // USART1번 쓰겠다
   nvic.NVIC_IRQChannelPreemptionPriority = 1 ; // 우선순위 세팅
   nvic.NVIC_IRQChannelSubPriority = 1 ;
   nvic.NVIC_IRQChannelCmd = ENABLE ;
   NVIC_Init(&nvic) ;

   // 확인 필요
   nvic.NVIC_IRQChannel = USART2_IRQn; // USART2번 쓰겠다
   nvic.NVIC_IRQChannelPreemptionPriority = 1 ; // 우선순위 세팅
   nvic.NVIC_IRQChannelSubPriority = 1 ;
   nvic.NVIC_IRQChannelCmd = ENABLE ;
   NVIC_Init(&nvic);
}


void bt_delay() {
	int i=0;
	for (i=0; i<2000000; i++) {}
}



void BT_Init(void) {
  BT_Set_Clock();
  BT_Set_GPIO();
  BT_Set_USART();
  BT_Set_NVIC();
}
