#include "Lcd.h"

#define     LCM_OUT               GPIOC->ODR
#define     LCM_PIN_RW            GPIO_PIN_12					// PC12
#define     LCM_PIN_RS            GPIO_PIN_11         // PC11
#define     LCM_PIN_EN            GPIO_PIN_10         // PC10
#define     LCM_PIN_D7            GPIO_PIN_9          // PC9
#define     LCM_PIN_D6            GPIO_PIN_8          // PC8
#define     LCM_PIN_D5            GPIO_PIN_7          // PC7
#define     LCM_PIN_D4            GPIO_PIN_6          // PC6
#define     LCM_PIN_MASK  (LCM_PIN_RW | LCM_PIN_EN | LCM_PIN_RS | LCM_PIN_D7 | LCM_PIN_D6 | LCM_PIN_D5 | LCM_PIN_D4)

//GPIO_InitTypeDef LCDWRITE;
//GPIO_InitTypeDef LCDREAD;

//---Импульс на ноге Е для записи дисплеем заданных параметров---//
void PulseLCD()
{
	LCM_OUT |= LCM_PIN_EN;
	delay_us(2);
	LCM_OUT &= (~LCM_PIN_EN);
}
 
//---Отсылка байта в дисплей---//
void SendByte(char ByteToSend, int IsData)
{
	LCM_OUT &= (~LCM_PIN_MASK);
	LCM_OUT |= ((ByteToSend & 0xF0) << 2);
	if (IsData == 1){
		LCM_OUT |= LCM_PIN_RS;								
		delay_us(4);
	}
	else{
		LCM_OUT &= ~LCM_PIN_RS;	
		delay_us(4);
	}
	PulseLCD();
	delay_us(5);
	
	LCM_OUT &= (~LCM_PIN_MASK);	
	LCM_OUT |= ((ByteToSend & 0x0F) << 6);
	if (IsData == 1)
			LCM_OUT |= LCM_PIN_RS;
	else
			LCM_OUT &= ~LCM_PIN_RS;
	PulseLCD();
	delay_ms(5);
	/*LCM_OUT &= ~(LCM_PIN_MASK);
	LCDREAD.GPIO_Pin = LCM_PIN_D7;
	LCDREAD.GPIO_Mode = GPIO_Mode_IPU;
	LCDREAD.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &LCDREAD); 
	
	LCM_OUT |= LCM_PIN_RW;
	 
	while(GPIO_ReadInputDataBit(GPIOC, LCM_PIN_D7) == 1){}
	LCM_OUT &= (~LCM_PIN_RW);
	
	LCM_OUT &= ~(LCM_PIN_MASK);
	LCDREAD.GPIO_Pin = LCM_PIN_D7;
	LCDREAD.GPIO_Mode = GPIO_Mode_Out_PP;
	LCDREAD.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &LCDREAD); */
	
	LCM_OUT &= ~(LCM_PIN_MASK);
}
 
//---Установка позиции курсора---//
void Cursor(char Row, char Col)
{
	char address;
	if (Row == 0)
		address = 0;
	else
	address = 0x40;
	address |= Col;
	SendByte(0x80 | address, 0);
}
 
//---Очистка дисплея---//
void ClearLCDScreen()
{
	SendByte(0x01, 0);
	delay_ms(3);
}
 
//---Инициализация дисплея---//
void LCD_Init(void)
{
	GPIO_InitTypeDef Pin;
	__HAL_RCC_GPIOC_CLK_ENABLE();
	Pin.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
	Pin.Mode = GPIO_MODE_OUTPUT_PP;
	Pin.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOC, &Pin);
	
	delay_ms(20);
	LCM_OUT &= ~(LCM_PIN_MASK);					
	LCM_OUT &= ~LCM_PIN_RS;							
	LCM_OUT &= ~LCM_PIN_EN;
	LCM_OUT |= 0x80;
	PulseLCD();
	delay_ms(5);
	LCM_OUT &= ~(LCM_PIN_MASK);
	
	LCM_OUT |= 0x20 << 2;
	PulseLCD();
	delay_us(10);
	LCM_OUT &= ~(LCM_PIN_MASK);
	LCM_OUT |= 0x80 << 2;
	PulseLCD();
	delay_ms(5);
	LCM_OUT &= ~(LCM_PIN_MASK);
			
	SendByte(0x08, 0);
	SendByte(0x06, 0);
	SendByte(0x14, 0);
	SendByte(0x0C, 0);
}
 
//---Печать строки---//
void PrintStr(char *Text)
{	
	NVIC_DisableIRQ (EXTI0_IRQn);
	NVIC_DisableIRQ (EXTI1_IRQn); 
	char *c;
	c = Text;
	while ((c != 0) && (*c != 0))
	{
			SendByte(*c, 1);
			c++;
	}
	NVIC_EnableIRQ (EXTI0_IRQn);
	NVIC_EnableIRQ (EXTI1_IRQn);
}

void PrintVar(int y)
{	
	NVIC_DisableIRQ (EXTI0_IRQn);
	NVIC_DisableIRQ (EXTI1_IRQn); 
	char str[10];
	sprintf (str, "%d", y);
	char *c = str;
	PrintStr(c);
	NVIC_EnableIRQ (EXTI0_IRQn);
	NVIC_EnableIRQ (EXTI1_IRQn);
}

void Cleaning (int row, int col, int number)
{
	NVIC_DisableIRQ (EXTI0_IRQn);
	NVIC_DisableIRQ (EXTI1_IRQn); 
	Cursor(row, col);
	for(int i = 0; i < number; i++){
		SendByte(' ', 1);
	}
	NVIC_EnableIRQ (EXTI0_IRQn);
	NVIC_EnableIRQ (EXTI1_IRQn);
}

void Marker(char Row, char Col){
	Cursor(Row,Col);
	SendByte(0x7E, 1);
}
	

