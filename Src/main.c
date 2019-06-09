/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sd.h"
#include "Lcd.h"
#include "Delay.h"
#include "Encoder.h"
#include "ws2812b.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
DMA_HandleTypeDef hdma_tim3_ch4_up;

extern int EncoderCount;
extern int EncoderMenu;
extern uint16_t BUF_DMA [ARRAY_LEN];
extern volatile uint16_t Timer1;
uint8_t sect[512];
uint32_t byteswritten,bytesread;
uint8_t result;
extern char USERPath[4]; /* logical drive path */
FATFS SDFatFs;
FATFS *fs;
FIL MyFile;
extern int count;
int NUMLEDS = 144;
int DMA_is_Ready = 1;

struct menu                             // Структура описывающая меню
{
  int ID;                               // Идентификационный уникальный индекс ID
  int parentID;                         // ID родителя
	int childID;
	int group;															// кол-во выводимых пунктов
	bool isParam;                         // Является ли пункт изменяемым параметром
  char name[8];                         // Название
  int value;                            // Актуальное значение
  int _min;                             // Минимально возможное значение параметра
  int _max;                             // Максимально возможное значение параметра
};

int menuArraySize = 8;                 // Задаем размер массива
struct menu menus[] = {                        // Задаем пункты меню
/*1.номер 2.роидтель,3.имя,4.тек.зн.56.диапазон*/
	{0, 0, 1, 1, false, "Menu",     0,   0,  0},
  {1, 0, 4, 3, false, "Files",    0,   0,  0},
  {2, 0, 5, 3, false, "Setting",  0,   0,  0},
  {3, 0, 0, 3, false, "Restart",  0,   0,  0},
  {4, 1, 4, 1, false, "Files",    0,   0,  0},
  {5, 1, 0, 3, true,  "NumLED",   72, 0,  144},
  {6, 1, 0, 3, true,  "Delay",    5,   0,  1000},
  {7, 1, 0, 3, true,  "Cycle",    0,   0,  1}
};
struct Image                           
{
	int ImageID;      //номер
  char name[6];     //имя картинки              
  int Width;				//ширина (кол-во светодиодов)
	int Height;				//высота
	int Offset;				//смещение
	int Size;					//размер
};
struct Image Images[] = {
	{1,  "", 0, 0, 0, 0},
	{2,  "", 0, 0, 0, 0},
	{3,  "", 0, 0, 0, 0},
	{4,  "", 0, 0, 0, 0},
	{5,  "", 0, 0, 0, 0},
	{6,  "", 0, 0, 0, 0},
	{7,  "", 0, 0, 0, 0},
	{8,  "", 0, 0, 0, 0},
	{9,  "", 0, 0, 0, 0},
	{10, "", 0, 0, 0, 0},
};
int GroupNow = 0;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void printmenu(int parID, bool direction){
	EncoderCount = 0;
	ClearLCDScreen();
	int chid = 0;
	int number = 0;
	if(direction){
		chid = menus[parID].childID;
		number = menus[chid].group;
		GroupNow = chid;
	}
	else{
		chid = menus[parID].parentID;
		number = menus[chid].group;
		GroupNow = chid;
	}
	Marker(EncoderCount % 2, EncoderCount / 2 * 8);
	for(int i  = 0; i < menus[chid].group; i++){
		Cursor(i % 2, i / 2 * 8 + 1);
		PrintStr(menus[chid+i].name);
	}		
}
//--------------------------------------------------
void printfiles(int NumOfFiles, int shift, int restriction){
	EncoderCount = 0;
	ClearLCDScreen();
	Marker(EncoderCount % 2, EncoderCount / 2 * 8);
	for(int i  = 0; i <= restriction; i++){
		Cursor(i % 2, i / 2 * 8 + 1);
		PrintStr(Images[i+shift].name);
	}		
}
//--------------------------------------------------
void PrintPicture(int NumPic){
	FIL Pic;
	int k = 0;
	int x = strlen(Images[NumPic].name) + 4 + 2;
	char *name = (char*) malloc(x * sizeof(char));
	strcpy( (char*)name, (char*)Images[NumPic].name);
	strcat(name, ".bmp");
	f_open(&Pic, name, FA_READ);
	k = Images[NumPic].Offset+19*3;
	if(f_lseek(&Pic, k) != FR_OK) Error_Handler();
	while(k <= (Images[NumPic].Width * Images[NumPic].Height * 3)){
		//ws2812_init();
		if(f_read(&Pic, sect, (menus[5].value * 3),(UINT *) &bytesread) != FR_OK) Error_Handler();
		delay_ms(menus[6].value);
		while(DMA_is_Ready != 1);
		for(int i = 0, j = 0; i < (menus[5].value * 3); i+=3, j++){
			ws2812_pixel_rgb_to_buf_dma(sect[i] , sect[i+1] , sect[i+2] , j);
		}
		//while(DMA_is_Ready != 1);
		DMA_is_Ready = 0;
		HAL_TIM_PWM_Start_DMA(&htim3,TIM_CHANNEL_4,(uint32_t*)&BUF_DMA,ARRAY_LEN);
		k += menus[5].value*3;
		if(f_lseek(&Pic, k) != FR_OK) Error_Handler();
	}
	/*while(DMA_is_Ready != 1);
	for(int i = 0; i < menus[5].value + 10; i++){
			ws2812_pixel_rgb_to_buf_dma(0 , 0, 0, i);
		}
	//while(DMA_is_Ready != 1);
	DMA_is_Ready = 0;*/
	HAL_TIM_PWM_Start_DMA(&htim3,TIM_CHANNEL_4,(uint32_t*)&BUF_DMA,DELAY_LEN);
	for(int i = 0; i < menus[5].value + 10; i++){
			ws2812_pixel_rgb_to_buf_dma(0 , 0, 0, i);
		}
}
//--------------------------------------------------
void reset (){
	SCB->AIRCR = 0x05FA0004;
}
//--------------------------------------------------
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
	//комментарий для гитхаба
	
  /* USER CODE BEGIN Init */
	uint16_t i;
	FRESULT res; //результат выполнения
	FILINFO fileInfo;
	char *fn;
	DIR dir;	
	DWORD fre_clust, fre_sect, tot_sect;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
	Delay_Init();
	Encoder_Init();
	ws2812_init();
	HAL_TIM_PWM_Start_DMA(&htim3,TIM_CHANNEL_4,(uint32_t*)&BUF_DMA,ARRAY_LEN);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	delay_ms(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	LCD_Init();
	HAL_TIM_Base_Start_IT(&htim2);
	disk_initialize(SDFatFs.drv);
	//readDir
	int NumOfFiles = 0;
	if(f_mount(&SDFatFs,(TCHAR const*)USERPath,0)!=FR_OK)
	{
		Error_Handler();
	}
	else
	{
		fileInfo.lfname = (char*)sect;
		fileInfo.lfsize = sizeof(sect);
		result = f_opendir(&dir, "/");
		if (result == FR_OK)
		{
			while(1)
			{
				result = f_readdir(&dir, &fileInfo);
				if ((result==FR_OK && fileInfo.fname[0]) && NumOfFiles <= 9)
				{
					char *fn = NULL;
					if(strlen(fileInfo.lfname)) fn = (char*) fileInfo.lfname;
					else	fn = (char*) fileInfo.fname;
					if(strstr(fn, ".bmp"))
					{
						char *i = strpbrk(fn, ".");
						int numbbyte = (( i - fn) > 6) ? 6 :  i - fn;
						strncpy(Images[NumOfFiles].name, fn, numbbyte);
						NumOfFiles++;
					}
				}
				else break;
			}
			f_closedir(&dir);
		}
		else Error_Handler();
	}
	//read
	for(int i = 0; i < NumOfFiles; i++)
	{
		delay_ms(10);
		int x = strlen(Images[i].name) + 4 + 2;
		char *name = (char*) malloc(x * sizeof(char));
		strcpy( (char*)name, (char*)Images[i].name);
		//if(NumOfFiles == 0) break;
		strcat(name, ".bmp");
		if(f_open(&MyFile,(char*) name,FA_READ)!=FR_OK)
		{
			Error_Handler();
		}
		else
		{
			f_read(&MyFile, sect, 64, &bytesread);
			//Width
			Images[i].Width |= sect[21] << 24;	//складываем ширину
			Images[i].Width |= sect[20] << 16;
			Images[i].Width |= sect[19] << 8;
			Images[i].Width |= sect[18];
			Images[i].Height |= sect[25] << 24;	//складываем высоту
			Images[i].Height |= sect[24] << 16;
			Images[i].Height |= sect[23] << 8;
			Images[i].Height |= sect[22];
			Images[i].Offset |= sect[13] << 24;	//смещение отн.начала файла
			Images[i].Offset |= sect[12] << 16;
			Images[i].Offset |= sect[11] << 8;
			Images[i].Offset |= sect[10];
			Images[i].Offset += 1024;
			Images[i].Size |= sect[5] << 24;		//размер файла
			Images[i].Size |= sect[4] << 16;
			Images[i].Size |= sect[3] << 8;
			Images[i].Size |= sect[2];
			if(Images[i].Width != menus[5].value)
			{
				Images[i].Width = 0;
				Images[i].Height = 0;
				Images[i].Offset = 0;
				Images[i].Size = 0;
				for( int j = i; j <= NumOfFiles; j++)
				{
					strcpy(Images[j].name, Images[j+1].name);
				}
				NumOfFiles--;
				i--;
			}	
			if( f_close(&MyFile) != FR_OK) {
				Error_Handler();
			}
		}
		free(name);
	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	int OLD_EncoderMenu = 0;
	int OLD_EncoderCount = 0;
	int OLD_MarkerRow = 0;
	int OLD_MarkerCol = 0;
	Marker(EncoderCount % 2, EncoderCount / 2 * 8);
	Cursor(0,1);	
	PrintStr(menus[0].name);
	int restriction = menus[0].group - 1;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(OLD_EncoderCount != EncoderCount){
			if(EncoderCount < 0 || EncoderCount > restriction)
				EncoderCount = (EncoderCount < 0 ? restriction : 0);
			Cleaning(OLD_MarkerRow, OLD_MarkerCol, 1);
			Marker(EncoderCount % 2, EncoderCount / 2 * 8);
			OLD_EncoderCount = EncoderCount;
			OLD_MarkerRow = EncoderCount % 2;
			OLD_MarkerCol = EncoderCount / 2 * 8;
		}
		if(OLD_EncoderMenu != EncoderMenu){
			Cleaning(EncoderCount % 2, EncoderCount / 2 * 8, 1);
			if(EncoderMenu > OLD_EncoderMenu){
				if (GroupNow + EncoderCount == 3){
					reset();
				}
				else if(menus[GroupNow + EncoderCount].isParam){	//if на изменение настроек
					int pos = EncoderCount;
					ClearLCDScreen();
					Marker(0, 0);
					Cursor(0, 1);
					PrintStr("min:");
					PrintVar(menus[GroupNow + EncoderCount]._min);
					Cursor(0, 8);
					PrintStr("max:");
					PrintVar(menus[GroupNow + EncoderCount]._max);
					Cursor(1, 0);
					PrintStr("Value:");
					PrintVar(menus[GroupNow + EncoderCount].value);
					OLD_EncoderMenu = EncoderMenu;
					OLD_EncoderCount = 0;
					EncoderCount = menus[GroupNow + EncoderCount].value;
					restriction = menus[GroupNow + pos]._max;
					while(OLD_EncoderMenu == EncoderMenu){
						if(OLD_EncoderCount != EncoderCount){
							if(EncoderCount < 0 || EncoderCount > restriction)
								EncoderCount = (EncoderCount < 0 ? restriction : 0);			
							Cleaning(1,6,4);
							Cursor(1,6);
							PrintVar(EncoderCount);
							OLD_EncoderCount = EncoderCount;
						}
					}					 
					menus[GroupNow + pos].value = EncoderCount;
					EncoderCount = 0;
					OLD_EncoderCount = 0;
					printmenu(menus[2].ID, 1);
				}
				else if(menus[GroupNow + EncoderCount].ID == 4){		//If для выбора картинки
					EncoderCount = 0;
					OLD_EncoderCount = 0;
					OLD_EncoderMenu = EncoderMenu;
					int shift = 0;
					restriction = ((NumOfFiles - shift) > 3 ? 3 : (NumOfFiles - shift));
					printfiles(NumOfFiles, shift, restriction);
					while(OLD_EncoderMenu == EncoderMenu){
						if(OLD_EncoderCount != EncoderCount){
							if(EncoderCount < 0 || EncoderCount > restriction){
								if((shift + EncoderCount) >= NumOfFiles){
									EncoderCount = 0;
									shift = 0;
									restriction = (NumOfFiles - shift) > 3 ? 3 : (NumOfFiles - shift);
									printfiles(NumOfFiles, shift, restriction);
								}
								else if(EncoderCount < 0 && shift != 0){
									EncoderCount = 0;
									shift -= 4;
									restriction = (NumOfFiles - shift) > 3 ? 3 : (NumOfFiles - shift);
									printfiles(NumOfFiles, shift, restriction);
								}
								else{
									EncoderCount = 0;
									shift += 4;
									restriction = (NumOfFiles - shift) > 3 ? 3 : (NumOfFiles - shift - 1);
									printfiles(NumOfFiles, shift, restriction);
								}									
							}
							Cleaning(OLD_MarkerRow, OLD_MarkerCol, 1);
							Marker(EncoderCount % 2, EncoderCount / 2 * 8);
							OLD_EncoderCount = EncoderCount;
							OLD_MarkerRow = EncoderCount % 2;
							OLD_MarkerCol = EncoderCount / 2 * 8;
						}
					}
					if(OLD_EncoderMenu < EncoderMenu){
						PrintPicture(shift+EncoderCount);
						
					}
				}
				else
					printmenu(menus[GroupNow + EncoderCount].ID, 1);
			}
			else{
				printmenu(menus[GroupNow + EncoderCount].ID, 0);
			}
			restriction = menus[GroupNow].group - 1;
			OLD_EncoderMenu = EncoderMenu;
			OLD_MarkerRow = 0;
			OLD_MarkerCol = 0;
		}
  /* USER CODE END 3 */
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = WS2812B_TIMER_AAR;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA3 PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
//----------------------------------------------------------

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim2)
	{
		Timer1++;
	}
}
//----------------------------------------------------------
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
