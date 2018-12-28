#include "lightsensor.h"

#include "stm32f10x.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"


#include <stm32f10x_adc.h>
#include "stm32f10x_dma.h"


uint32_t ADC_Value[2];


void LT_Set_Clock(void){
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	 // interrupt
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);     // RCC GPIO E
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);     // RCC GPIO C
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);     // RCC GPIO D
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	 // ADC1
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
}

void LT_Set_Gpio(void){
   GPIO_InitTypeDef gpio;

   // LED Output Mode
   gpio.GPIO_Mode = GPIO_Mode_Out_PP;
   gpio.GPIO_Pin=(GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7);
   gpio.GPIO_Speed = GPIO_Speed_50MHz; // ���ļ�
   GPIO_Init(GPIOD, &gpio);


   //Jodo sensor input 1
   gpio.GPIO_Mode = GPIO_Mode_AIN;
   gpio.GPIO_Pin = (GPIO_Pin_1);
   gpio.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOC, &gpio);

   //Jodo sensor input 2
   gpio.GPIO_Mode = GPIO_Mode_AIN;
   gpio.GPIO_Pin = (GPIO_Pin_2);
   gpio.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOC, &gpio);


}

void LT_Set_ADC(void){
	ADC_InitTypeDef adc;

	adc.ADC_ContinuousConvMode = ENABLE; // ���������� �������ΰ� �ƴѰ�
	adc.ADC_DataAlign = ADC_DataAlign_Right; // ������ ������ ���ʳ� �����ʳ�
	adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //�ܺ� ���ͷ�Ʈ >>>> x
	adc.ADC_Mode = ADC_Mode_Independent;  //
	adc.ADC_NbrOfChannel = 2;
	adc.ADC_ScanConvMode = ENABLE;

	ADC_Init(ADC1, &adc);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_13Cycles5); // �ñ�! plck2 +
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_13Cycles5);

	ADC_Cmd(ADC1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);

	while(ADC_GetResetCalibrationStatus(ADC1)) {}
	ADC_ResetCalibration(ADC1);

	while(ADC_GetCalibrationStatus(ADC1)) {}
	ADC_StartCalibration(ADC1);

	ADC_SoftwareStartConvCmd(ADC1, ENABLE); // ������ enable



}


void LT_Set_DMA(void){
	DMA_InitTypeDef dma;

	dma.DMA_PeripheralBaseAddr = (uint32_t) &ADC1->DR;
	dma.DMA_MemoryBaseAddr = (uint32_t) ADC_Value; // �ּҰ��� ������ ���� �־��.
	dma.DMA_DIR = DMA_DIR_PeripheralSRC; //DMA_DIR_PeripheralSRC = 0 (read from peripheral to memory)
	dma.DMA_BufferSize = 2;
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //????? �ֺ� ��ġ �ּ� �������Ͱ� �����ϴ��� ���θ� �����մϴ�.
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable; 	//????? �޸� �ּ� �������Ͱ� �����ϴ��� ���θ� �����մϴ�.
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // 4byte int ���� �ޱ� ����  word.
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	//(DMA_Mode)������ ä�ο��� �޸� - �޸� ������ ������ ������ ��� ��ȯ ���� ��带 ����� �� �����ϴ�. ������ ������ �帧�� ���� ���ΰ�
	dma.DMA_Mode = DMA_Mode_Circular;
	dma.DMA_Priority = DMA_Priority_High;
	dma.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel1, &dma);

	DMA_Cmd(DMA1_Channel1, ENABLE);
}

void LT_Init() {
	LT_Set_Clock();
	LT_Set_Gpio();
	LT_Set_ADC();
	LT_Set_DMA();
}

uint32_t LT_Get_LightValue() {
	return ADC_Value[0];
}


