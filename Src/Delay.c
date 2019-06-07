/** 
	**************************************************************
	*Файл с реализацией функций задержек
	* Работают за счет 2ех таймеров (TIM2 и TIM3)
	* На их основе реализованы две фукнции задержки в
	* миллисекундках и микросекундах.
	* Для корректной работы перед использованием функций задержек
	* в основной части программы нужно вызвать функцию TimersInit()
	* для настройки таймеров.
	**************************************************************
	*/

#include "Delay.h"

TIM_Base_InitTypeDef TimerUS;
TIM_Base_InitTypeDef TimerMS;
	
void delay_us(int delay)
{
	TIM4->PSC     = 192;               		//настроить делитель для формирования миллисекунд
	TIM4->CR1     = TIM_CR1_OPM;          //режим одного импульса
	TIM4 -> ARR = delay - 1;                
	TIM4 -> CNT = 0;
	TIM4 -> CR1 = TIM_CR1_CEN | TIM_CR1_OPM;    		  
	while((TIM4 -> SR & TIM_SR_UIF)==0){}
	TIM4 -> SR &= ~TIM_SR_UIF;
}
	
void delay_ms(int delay)
{
	TIM4->PSC     = 19800;               		//настроить делитель для формирования миллисекунд
	TIM4->CR1     = TIM_CR1_OPM;          //режим одного импульса
	TIM4 -> ARR = delay * 10;                
	TIM4 -> CNT = 0;
	TIM4 -> CR1 = TIM_CR1_CEN | TIM_CR1_OPM;    		  
	while((TIM4 -> SR & TIM_SR_UIF)==0){}
	TIM4 -> SR &= ~TIM_SR_UIF;
}
	
void Delay_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;   //подать тактирование на TIM2
	TIM4->CR1     = TIM_CR1_OPM;          //режим одного импульса
}
