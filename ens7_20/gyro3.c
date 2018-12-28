
#include "stm32f10x.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

#include "bluetooth.h"
#include "queue.h"
#include "servo.h"
#include "lightsensor.h"

#include "lcd.h"
#include "touch.h"

#define ISDEBUGGING 1

#define START_RECIEVE_SIGNAL 's'
#define END_SEND_SIGNAL 'e'
#define JODO_THRESHOLD 1000//2700
#define LIGHT_SENSOR_ENABLE 1
#define INIT_DELAY_TIME 20
#define CONSECUTIVE_MAX 30

#define DEBUG_ANGLE_RATE 1.0



volatile unsigned int Timer2_Counter=0;
Queue queue;

typedef struct _GameStatue {
	int isGaming;
} GameStatue;

typedef struct _GyroInfo {
	char x;
	char y;
	int status;
} GyroInfo;

GameStatue gameStatus;
GyroInfo beforeGyroInfo;
GyroInfo currentGyroInfo;
int gyroValue;
int consecutiveActivateTimes = 0;

void ledonoff(int index, int on);



/* ======================================================= */
/* ================== configurate stm32 ================== */
/* ======================================================= */
void ledonoff(int index, int on) {
	GPIO_ResetBits(GPIOD, GPIO_Pin_2);
	GPIO_ResetBits(GPIOD, GPIO_Pin_3);
	GPIO_ResetBits(GPIOD, GPIO_Pin_4);
	GPIO_ResetBits(GPIOD, GPIO_Pin_7);

	if (on == 1) {
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
	}
	if (on == 0) {
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

/* ======================================================= */
/* ================== interrupt handler ================== */
/* ======================================================= */
void USART1_IRQHandler() { // 컴퓨터 키보드 입력시
	if (USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET) {
		char data;
		Task task;
		data = USART_ReceiveData(USART1);
		task.data = data;
		task.usartType = 1;
		queue_push(&queue, task);
	}

	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

void USART2_IRQHandler() { // 블루투스 입력시
	if (USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET) {
		char data;
		Task task;
		data = USART_ReceiveData(USART2);
		task.data = data;
		task.usartType = 2;
		queue_push(&queue, task);
//	   USART_SendData(USART1,data);
	}

	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
}


// interrupt callback handler
void TIM2_IRQHandler(void) {
    if(TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET) {
        // Clear the interrupt flag
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        Timer2_Counter++;
        GPIOB->BRR = GPIO_Pin_0;  // PB0 OFF
    }

    if(TIM_GetITStatus(TIM2,TIM_IT_CC1) != RESET) {
        TIM_ClearITPendingBit(TIM2,TIM_IT_CC1);
        GPIOB->BSRR = GPIO_Pin_0;  // PB0 ON
    }
}

// my functions
void delay(unsigned int del) {
    Timer2_Counter=0;
    while(Timer2_Counter < del);
}

/* ========================================== */
/* ================== game ================== */
/* ========================================== */
void Init_Game() {
	gameStatus.isGaming = 0;
	ledonoff(0, 1);
}

void start_Game() {
	if (gameStatus.isGaming!=1) {
		gameStatus.isGaming = 1;
		setServo3(80);
		ledonoff(1, 1);
		ledonoff(2, 1);
	}
}

//From now on, if ADC_Value[0] < 2500 then turn on led 1, else turn led 3
void end_Game(){
	if (gameStatus.isGaming != 0) {
		USART_SendData(USART2, END_SEND_SIGNAL);
		gameStatus.isGaming = 0;
		setServo3(180);


		setServo1(((90-90)*DEBUG_ANGLE_RATE)+90 + 16);
		setServo2(((90-90)*DEBUG_ANGLE_RATE)+90 - 11);
		ledonoff(0, 1);
	}
}


void Set_QueueInit() {
	queue.tail = 0;
	queue.head = 0;
	queue.overflow = 0;
}


/* ========================================== */
/* ========================================== */
/* ========================================== */
/* ========================================== */
/* ========================================== */
int main(){
	int i=0;
	unsigned int puttydelay = 10;
	int num = 10;
	int counting = 0;

	SystemInit();


	LT_Init();
	BT_Init();
	Set_QueueInit();
	SERVO_Init();




	LCD_Init();
	// Touch_Configuration();
	// Touch_Adjust(); //
	LCD_Clear(WHITE);

	//LCD_ShowString(150, 130, "USART2():", BLACK, WHITE);


	LCD_ShowString(50, 40, "READY:", BLACK, WHITE);

//	delay(50);
//	USART_SendData(USART2, 'a');
//	delay(puttydelay);
//	USART_SendData(USART2, 't');
//	delay(puttydelay);
//	USART_SendData(USART2, '+');
//	delay(puttydelay);
//	USART_SendData(USART2, 'b');
//	delay(puttydelay);
//	USART_SendData(USART2, 't');
//	delay(puttydelay);
//	USART_SendData(USART2, 's');
//	delay(puttydelay);
//	USART_SendData(USART2, 'c');
//	delay(puttydelay);
//	USART_SendData(USART2, 'a');
//	delay(puttydelay);
//	USART_SendData(USART2, 'n');
//	delay(puttydelay);
//	USART_SendData(USART2, '\n');
//	delay(puttydelay);


	ledonoff(1, 0);
	ledonoff(2, 0);
	ledonoff(3, 0);
	ledonoff(0, 0);

	delay(INIT_DELAY_TIME);

	setServo1(((90-90)*DEBUG_ANGLE_RATE)+90 + 16);
	setServo2(((90-90)*DEBUG_ANGLE_RATE)+90 - 11);
	setServo3(180);
	ledonoff(0, 1);

	LCD_ShowString(50, 40, "START:", BLACK, WHITE);


	Init_Game();

	while(1){


		LCD_ShowNum(50,100, LT_Get_LightValue(), 5, BLACK,WHITE); // 조도값 출력
		//LCD_ShowNum(50,120,ADC_Value[1],5,BLACK,WHITE);

		if (isEmpty(&queue) == false) {
			Task task;
			queue_pop(&queue, &task);
			if (task.usartType == 1) { // from putty
				while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
				USART_SendData(USART2, task.data);

				LCD_ShowString(150, 180, "USART2():", BLACK, WHITE);
				LCD_ShowNum(150, 200, task.data, 5, BLACK, WHITE);

			} else if (task.usartType == 2) { // from bluetooth

				if (gameStatus.isGaming == 0) { // waiting for starting game
					while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
					USART_SendData(USART1, task.data);

					if (task.data == START_RECIEVE_SIGNAL) {
						start_Game();
					}
				} else { // now playing game
					while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
					USART_SendData(USART1, task.data);

					if (task.data == '1') { // y
						currentGyroInfo.y = gyroValue;
					} else if (task.data == '0') { // x
						currentGyroInfo.x = gyroValue;
					} else {
						gyroValue = task.data;
					}

					if (task.data == '1') {
						beforeGyroInfo = currentGyroInfo;
						beforeGyroInfo.status = 1;
						currentGyroInfo.x = 0;
						currentGyroInfo.y = 0;
						currentGyroInfo.status = 0;

						counting ++;

						setServo1(((beforeGyroInfo.x-90)*DEBUG_ANGLE_RATE)+90 + 16);
						setServo2(((beforeGyroInfo.y-90)*DEBUG_ANGLE_RATE)+90 - 11);

						LCD_ShowString(150, 80, "USART1():", BLACK, WHITE);
						LCD_ShowNum(150, 100, beforeGyroInfo.x, 5, BLACK, WHITE);
						LCD_ShowNum(150, 120, beforeGyroInfo.y, 5, BLACK, WHITE);


					} else if (task.data == '2') { // end game signal from mobile device
						end_Game();
					}
				}



				//ledonoff(1, 1);
			}
		}

		if(gameStatus.isGaming == 1 && LIGHT_SENSOR_ENABLE && LT_Get_LightValue() < JODO_THRESHOLD){
			consecutiveActivateTimes++;
			LCD_ShowNum(50,150,LT_Get_LightValue(),5,BLACK,WHITE);
		} else {
			consecutiveActivateTimes = 0;
		}
		if (gameStatus.isGaming == 1 && consecutiveActivateTimes > CONSECUTIVE_MAX) {
			end_Game();
		}
	}
	return 0;
}


//7조 자리
// flash load C:\Users\Team07\Desktop\ens7_20\Debug\flashclear.axf
// flash load C:\Users\Team07\Desktop\ens7_20\Debug\ens7_20.axf
