#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"
#include "Delay.h"
#include "stdlib.h"
#include <stdio.h>

void PulseLCD(void);													//Импульс на ноге Е для записи дисплеем заданных параметров
 
void SendByte(char ByteToSend, int IsData);		//Отсылка байта в дисплей
 
void Cursor(char Row, char Col);							//Установка позиции курсора	
 
void ClearLCDScreen(void);										//Очистка дисплея
 
void LCD_Init(void);													//Инициализация дисплея
 
void PrintStr(char *Text);										//Печать строки

void PrintVar(int x);													//Печать переменной

void Cleaning(int row, int col, int number);	//Очистка куска дисплея

void Marker(char Row, char Col);
	
