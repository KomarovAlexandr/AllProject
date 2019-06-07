#include "Encoder.h"


GPIO_InitTypeDef Pins;

int EncoderCount = 0;
int EncoderMenu = 0;
int pinAValue = 0;            
int pinBValue = 0;
int front = 0;
int pinButtonValue = 0;
int timeButtonDown = 0;    

void Encoder_Init(void)
{
	EncoderCount = 0;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

	Pins.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
	Pins.Mode = GPIO_MODE_INPUT;
	Pins.Speed = GPIO_SPEED_FREQ_LOW;
	Pins.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &Pins);
	front = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	
	EXTI->IMR |= (EXTI_IMR_MR0 | EXTI_IMR_MR1);
	EXTI->RTSR |= (EXTI_RTSR_TR0);
	EXTI->FTSR |= (EXTI_FTSR_TR0 | EXTI_FTSR_TR1);
	NVIC_EnableIRQ (EXTI0_IRQn);
	NVIC_EnableIRQ (EXTI1_IRQn);
}

void EXTI0_IRQHandler(void)
{
	NVIC_DisableIRQ (EXTI0_IRQn);
	NVIC_DisableIRQ (EXTI1_IRQn); 
	EXTI->PR|=0x01;
	pinAValue = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);           
	pinBValue = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);
	
	if( !pinAValue && pinBValue && front) EncoderCount ++;
	if( pinAValue && !pinBValue && !front) EncoderCount ++;
	if( !pinAValue && !pinBValue && front) EncoderCount --;
	if( pinAValue && pinBValue && !front) EncoderCount --;
	delay_ms(5);
	pinAValue = 0;
	pinBValue = 0;
	front = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	NVIC_EnableIRQ (EXTI0_IRQn);
	NVIC_EnableIRQ (EXTI1_IRQn);
}	
	
void EXTI1_IRQHandler(void)
{
	NVIC_DisableIRQ (EXTI0_IRQn);
	NVIC_DisableIRQ (EXTI1_IRQn);
	EXTI->PR|=0x02;
	
	while( HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET ){
		delay_ms(10);
		timeButtonDown++;
		if(timeButtonDown >= 25){
			EncoderMenu--;
			break;
		}
	}
	if(timeButtonDown < 25){
		EncoderMenu++;
	}
	timeButtonDown = 0; 
	while( !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) ){}
	delay_ms(5);
	NVIC_EnableIRQ (EXTI0_IRQn);
	NVIC_EnableIRQ (EXTI1_IRQn);
}	
