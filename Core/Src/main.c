/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "sx1509.h"
#include "ssd1306_tests.h"
#include <time.h>
#include <stdlib.h>

#include <stdbool.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADDR_FLASH_PAGE_16    ((uint32_t)0x08008000)
#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_16
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
uint32_t Address = 0, PAGEError = 0;
__IO uint32_t data32 = 0 , MemoryProgramStatus = 0;

static FLASH_EraseInitTypeDef EraseInitStruct;


static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;

  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }

  return page;
}

static uint32_t GetBank(uint32_t Addr)
{
  uint32_t bank = 0;

  if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0)
  {
  	/* No Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_1;
    }
    else
    {
      bank = FLASH_BANK_2;
    }
  }
  else
  {
  	/* Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_2;
    }
    else
    {
      bank = FLASH_BANK_1;
    }
  }

  return bank;
}



int lb[3];

uint8_t Menu()
{
	ssd1306_Init();
		ssd1306_Fill(0);
		ssd1306_SetCursor(15,15);
		ssd1306_WriteString("Space Invaders", Font_7x10, 1);
		ssd1306_SetCursor(10,35);
		ssd1306_WriteString("Press 'S' to start", Font_6x8, 1);
		ssd1306_SetCursor(5,45);
		ssd1306_WriteString("'L' for Leaderboards", Font_6x8, 1);
		ssd1306_UpdateScreen();
		uint8_t value='q';
		while(value=='q')
			HAL_UART_Receive(&huart2, &value, 1, 500);
		return value;
}

typedef struct
{
	int x;
	int y;
	int isAlive;
}Invaders;

typedef struct
{
	int x;
	int life;
}Domek;

void zapiszWynik(int wynik)
{
	if(wynik>lb[0])
		{
			int tmp=lb[0];
			lb[0]=wynik;
			if(tmp>lb[1])
			{
				int tmp2=lb[1];
				lb[1]=tmp;
				if(tmp2>lb[2])
				{
					lb[2]=tmp2;
				}
			}
		}
	else if(wynik>lb[1])
		{
			int tmp=lb[1];
			lb[1]=wynik;
			if(tmp>lb[2])
			{
				lb[2]=tmp;
			}
		}
	else if(wynik>lb[2]) lb[2]=wynik;
}


void GameLoop()
{

	srand(time(NULL));
	uint32_t invHor = HAL_GetTick();
	uint32_t invShoot = HAL_GetTick();
	uint32_t doubleXPTime = HAL_GetTick();
	uint32_t InvulTime = HAL_GetTick();
	int ruchX=0;
	int wynik=0;
	int skip=0;
	int firstAlive=0;
	int lastAlive=6;
	int shootPC=0;
	int shootInv=0;
	char pociskPC = '^';
	char pociskInv = 'v';
	int xpi=0;
	int ypi=0;
	int xp=0;
	int yp=0;
	int doubleXP =0;
	int Invul = 0;
	int ileZyje=7;
	Invaders przec[7];
			int xin=20;
			int yin=0;
			for(int i=0; i<7; i++)
			{
				przec[i].x=xin;
				przec[i].y=yin;
				przec[i].isAlive=1;
				xin+=13;
			}

		Domek shields[5];
		int xdom=0;
		for(int i=0; i<5; i++)
		{
			shields[i].x=xdom;
			shields[i].life=3;
			xdom+=26;
		}


	int x1=0, x2=5;
	int graTrwa=1;
	while(graTrwa==1)
	{

		ssd1306_Fill(0);
		for(int i=0; i<7; i++)
		{
			if(przec[i].isAlive)
			{
				ssd1306_DrawRectangle(przec[i].x, przec[i].y, przec[i].x+5, przec[i].y+5, White);
			}

		}

		for(int i=0; i<5; i++)
				{
					if(shields[i].life>0)
					{
						ssd1306_DrawRectangle(shields[i].x, 50, shields[i].x+5, 55, White);
					}

				}
		ssd1306_DrawRectangle(x1,58,x2, 63, White);
		if(doubleXP == 1)
		{
			ssd1306_SetCursor(121, 0);
			ssd1306_WriteChar('D', Font_6x8, White);
			ssd1306_UpdateScreen();
		}
		else if(Invul == 1)
		{
			ssd1306_SetCursor(121, 0);
			ssd1306_WriteChar('I', Font_6x8, White);
			ssd1306_UpdateScreen();
		}
		ssd1306_UpdateScreen();
		uint8_t akcja;
		HAL_UART_Receive(&huart2, &akcja, 1, 1);
		switch(akcja)
		{
			case 'a':
				if(x1>0)
				{
					x1--;
					x2--;
				}

				break;
			case 'd':
				if(x2 < 127)
				{
					x1++;
					x2++;
				}

				break;
			case ' ':
				if(shootPC==0)
				{
					shootPC=1;
					xp=x1+((x2-x1)/2)-1;
					yp=50;
				}

				break;
		}
		akcja='p';
		uint32_t Obecny = HAL_GetTick();
		if(Obecny-invHor >= 1000)
		{
			for(int i=0; i<7; i++)
			{
				if(przec[lastAlive].x+5>=127)
					{
						ruchX=1;
						for(int i=0; i<7; i++)
						{
							przec[i].y++;
						}
					}
				if(przec[firstAlive].x<=1)
				{
					ruchX=0;
					for(int i=0; i<7; i++)
					{
						przec[i].y++;
					}

				}

				if(ruchX==0 && skip==0)przec[i].x++;
				else if(skip==0) przec[i].x--;
			}
			if(przec[0].y+5>=50) graTrwa=0;
			invHor=Obecny;
			skip=0;
		}
		if(Obecny-invShoot >= 2000 && shootInv==0)
		{
			int rng = rand()%7;
			if(przec[rng].isAlive)
			{
				shootInv=1;
				xpi=przec[rng].x+2;
				ypi=przec[rng].y+5;
			}
			invShoot=Obecny;


		}

		if(shootPC==1)
		{
			ssd1306_SetCursor(xp,yp);
			ssd1306_WriteChar(pociskPC, Font_6x8, White);
			ssd1306_UpdateScreen();
			yp-=2;
			for(int i=0; i<7; i++)
			{
				if(xp>=przec[i].x-2 && xp<przec[i].x+5 && yp<=przec[i].y+5)
				{
					przec[i].isAlive=0;
					ileZyje--;
					int powerup = rand() % 3;
					switch(powerup)
					{
					case 1:
						doubleXP=1;
						doubleXPTime = HAL_GetTick();
						break;
					case 2:
						Invul=1;
						InvulTime = HAL_GetTick();
						break;
					}
					przec[i].x=-15;
					if(i==firstAlive)
					{
						while(przec[firstAlive].isAlive==0)
						{
							firstAlive++;
						}
					}
					if(i==lastAlive)
					{
						while(przec[lastAlive].isAlive==0)
						{
							lastAlive--;
						}
					}
					if(doubleXP==1) wynik+=2;
					else wynik++;
					shootPC=0;
					break;

				}
			}
			for(int i=0; i<5; i++)
			{
				if(xp>=shields[i].x-2 && xp<shields[i].x+5 && yp<=55)
				{
					shields[i].life--;
					if(shields[i].life==0) shields[i].x=-15;
					shootPC=0;
					break;

				}
			}
			if(yp<=0) shootPC=0;
		}


		if(shootInv==1)
		{
			ssd1306_SetCursor(xpi,ypi);
			ssd1306_WriteChar(pociskInv, Font_6x8, White);
			ssd1306_UpdateScreen();
			ypi+=2;
			for(int i=0; i<5; i++)
			{
				if(xpi>=shields[i].x-2 && xpi<shields[i].x+5 && ypi>=45)
				{
					shields[i].life--;
					if(shields[i].life==0) shields[i].x=-15;
					shootInv=0;
					break;

				}
			}
			if(xpi>=x1-2 && xpi<=x2+2 && ypi >= 55)
			{
				if(Invul==0)graTrwa=0;
				else shootInv=0;
			}
			if(ypi>=64) shootInv=0;
		}

		if(ileZyje==0) graTrwa=0;

		if(doubleXP == 1)
		{
			if(Obecny > doubleXPTime && Obecny-doubleXPTime >= 10000)
			{
				doubleXP=0;
			}
		}
		if(Invul == 1)
		{
			if(Obecny > InvulTime && Obecny-InvulTime >= 5000)
			{
				Invul=0;
			}
		}


	}
	char score[20];
	itoa(wynik, score, 10);
	ssd1306_Fill(0);
	ssd1306_SetCursor(0,32);
	ssd1306_WriteString("Koniec!", Font_6x8, 1);
	ssd1306_SetCursor(48,32);
	ssd1306_WriteString(score, Font_6x8, 1);
	ssd1306_UpdateScreen();
	HAL_Delay(3000);
	zapiszWynik(wynik);

}

void leaderboards()
{
	char sc[10];
	itoa(lb[0], sc, 10);
	ssd1306_Fill(0);
	ssd1306_SetCursor(0,0);
	ssd1306_WriteString("1. miejsce: ", Font_6x8, 1);
	ssd1306_SetCursor(72,0);
	ssd1306_WriteString(sc, Font_6x8,1);
	ssd1306_SetCursor(0,9);
	ssd1306_WriteString("2. miejsce: ", Font_6x8,1);
	ssd1306_SetCursor(72,9);
	itoa(lb[1], sc, 10);
	ssd1306_WriteString(sc, Font_6x8,1);
	ssd1306_SetCursor(0,18);
	ssd1306_WriteString("3. miejsce: ", Font_6x8,1);
	ssd1306_SetCursor(72,18);
	itoa(lb[2], sc, 10);
	ssd1306_WriteString(sc, Font_6x8,1);
	ssd1306_UpdateScreen();
	HAL_Delay(3000);

}

void flash()
{
	HAL_FLASH_Unlock();
	FirstPage = GetPage(FLASH_USER_START_ADDR);
	NbOfPages = 4;
	BankNumber = GetBank(FLASH_USER_START_ADDR);
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks       = BankNumber;
	EraseInitStruct.Page        = FirstPage;
	EraseInitStruct.NbPages     = NbOfPages;
	HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
	Address = FLASH_USER_START_ADDR;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, (uint64_t)lb[0]);
	Address = Address + 8;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, (uint64_t)lb[1]);
	Address = Address + 8;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, (uint64_t)lb[2]);
	HAL_FLASH_Lock();
}

void wczytaj()
{
	Address = FLASH_USER_START_ADDR;
	int pierwszy = *(int*)Address;
	lb[0]=pierwszy;
	Address = Address + 8;
	int drugi = *(int*)Address;
	lb[1]=drugi;
	Address = Address + 8;
	int trzeci = *(int*)Address;
	lb[2]=trzeci;
}
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

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  switch(Menu())
	  {
	  case 's':
		  GameLoop();
		  break;
	  case 'l':
	  		  leaderboards();
	  		  break;
	  case 'z':
		  flash();
		  break;
	  case 'y':
		  wczytaj();
		  break;
	  }
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00100D14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOH_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);



	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(SSD1306_CS_Port,SSD1306_CS_Pin , GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(SSD1306_DC_Port,SSD1306_DC_Pin , GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pin : B1_Pin */
	  GPIO_InitStruct.Pin = B1_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);


	  GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


	  /*Configure GPIO pins : OLED_CS_Pin OLED_DC_Pin */
	  GPIO_InitStruct.Pin = SSD1306_CS_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(SSD1306_CS_Port, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin = SSD1306_DC_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(SSD1306_DC_Port, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin =  SX1509_OSC_Pin | SX1509_nRST_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(SX1509_nINIT_PORT, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin = SX1509_nINIT_Pin ;
	    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	    HAL_GPIO_Init(SX1509_nINIT_PORT, &GPIO_InitStruct);


	  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
	  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pin : OLED_Res_Pin */
	  GPIO_InitStruct.Pin = SSD1306_Reset_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(SSD1306_Reset_Port, &GPIO_InitStruct);

	  HAL_GPIO_WritePin(SX1509_nRST_PORT, SX1509_nRST_Pin, GPIO_PIN_SET);


/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
