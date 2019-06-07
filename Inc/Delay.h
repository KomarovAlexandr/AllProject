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

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_tim.h"

void Delay_Init(void);

void delay_us(int delay);

void delay_ms(int delay);
