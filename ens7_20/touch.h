#ifndef __TOUCH_H__
#define __TOUCH_H__

#include "stm32f10x.h"

extern float xfac;
extern float yfac;
extern short xoff;
extern short yoff;

#define T_DCLK_L GPIOC->BRR = 1 << 10
#define T_DCLK_H GPIOC->BSRR = 1 << 10

#define T_CS_L GPIOC->BRR = 1 << 8
#define T_CS_H GPIOC->BSRR = 1 << 8

#define T_DOUT_L GPIOC->BRR = 1 << 12
#define T_DOUT_H GPIOC->BSRR = 1 << 12

#define T_DIN (1 & ((GPIOC->IDR) >> 11))

#define T_INT (1 & ((GPIOC->IDR) >> 5))

void ADS_Write_Byte(uint8_t num);
uint16_t ADS_Read_AD(uint8_t CMD);
void Touch_Configuration(void);
void Draw_Big_Point(u16 x, u16 y);
uint8_t Touch_GexX(uint16_t *y, uint8_t ext);
uint8_t Touch_GexY(uint16_t *y, uint8_t ext);
void Touch_GetXY(uint16_t *x, uint16_t *y, uint8_t ext);
void Touch_Adjust(void);
void Convert_Pos(u16 x_in, u16 y_in, u16 *x_out, u16 *y_out);

#endif
