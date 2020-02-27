/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>

#define		BEEP_OFF		0
#define		BEEP_FINISH		1
#define		BEEP_ALERT		2

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint32_t	gDisplay_BCD = 0;
uint32_t	gTotalCycle	=	100;
uint32_t	gPowerOnTime = 90;
uint32_t	gPowerCycleInterval = 5;

GPIO_PinState		gModeSel	= GPIO_PIN_RESET;
uint8_t		gPowerOnOkFlag	= 0;
uint8_t		gBeepOn		=	0;
uint8_t		gKeyIncrement	= 0;
uint8_t		gKeyDecrement	= 0;
uint8_t		gKeySet			= 0;
uint8_t		gMode	=	0;	/* 0: Set TotalCycle; 1: Set Power On time; 2: Set Power cycle interval; 3: Run Mode; 4: Pause Mode*/
uint8_t		gBeepMode = BEEP_OFF;
uint32_t	gPowerOffDelaySec;
uint8_t		gPauseFlag = 0;
uint8_t		gResumeFlag = 0;
/* USER CODE END PV */
const uint8_t NixieTube[] = {
/* 	0,		1,		2,		3		*/
	0xc0,	0xf9,	0xa4,	0xb0,
/*	4,		5,		6,		7,		*/
	0x99,	0x92,	0x82,	0xf8,
/*	8,		9,		a,		b		*/
	0x80,	0x90,	0x88,	0x83,
/*	c,		d,		e,		f		*/
	0xc6,	0xa1,	0x86,	0x8e
};



/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
void TurnRelay(uint8_t RelayIndex, uint8_t TurnOn);
void BeepAlert(uint8_t mode);


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	uint32_t	runCnt = 1;
	uint32_t	timeout = 0;
	uint32_t	delayTime = 0;
   /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_USART1_UART_Init();
	MX_USART3_UART_Init();
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
	
	HAL_UART_Transmit(&huart3, (uint8_t*) "hello usart3\r\n", 14,  10);
	printf("Please set total cycles...\r\n");
	while(1)
	{		
		//set total cycle
		while(gMode == 0)
		{
			if(gKeyIncrement == 1)
			{	
				gKeyIncrement = 0;
				if(gTotalCycle < 9999)
					gTotalCycle ++;
			}
			
			if(gKeyDecrement == 1)
			{
				gKeyDecrement = 0;
				if(gTotalCycle > 0)
					gTotalCycle --;			
			}
			
			if(gKeySet == 1)
			{
				gKeySet = 0;
				gMode = 1;
				printf("Please set power on time, uint 1s...\r\n");
			}
			
			//display total cycles
			gDisplay_BCD = ((gTotalCycle/1000)<<24) | (((gTotalCycle/100)%10)<<16) | (((gTotalCycle/10)%10)<<8) | (gTotalCycle%10);
		}	
		
		//set power on time, unit: 1s.
		while(gMode == 1)
		{
			if(gKeyIncrement == 1)
			{	
				gKeyIncrement = 0;
				if(gPowerOnTime < 9999)
					gPowerOnTime ++;
			}
			
			if(gKeyDecrement == 1)
			{
				gKeyDecrement = 0;
				if(gPowerOnTime > 0)
					gPowerOnTime --;
			}
			
			if(gKeySet == 1)
			{
				gKeySet = 0;
				gMode = 2;
				printf("Please set power cycle interval time, uint 1s...\r\n");
			}
			
			//display power on time
			gDisplay_BCD = ((gPowerOnTime/1000)<<24) | (((gPowerOnTime/100)%10)<<16) | (((gPowerOnTime/10)%10)<<8) | (gPowerOnTime%10);
		}	
		
		//set power cycle interval time, unit: 1s.
		while(gMode == 2)
		{
			if(gKeyIncrement == 1)
			{	
				gKeyIncrement = 0;
				if(gPowerCycleInterval < 9999)
					gPowerCycleInterval ++;
				
				
			}
			
			if(gKeyDecrement == 1)
			{
				gKeyDecrement = 0;
				if(gPowerCycleInterval > 0)
					gPowerCycleInterval --;
			}
			
			if(gKeySet == 1)
			{
				gKeySet = 0;
				gMode = 3;
				printf("Running...\r\n");
			}
			
			//display power on time
			gDisplay_BCD = ((gPowerCycleInterval/1000)<<24) | (((gPowerCycleInterval/100)%10)<<16) | (((gPowerCycleInterval/10)%10)<<8) | (gPowerCycleInterval%10);
		}	
	
		//run mode
		while(gMode == 3)
		{
			gResumeFlag = 0;
			gModeSel	= HAL_GPIO_ReadPin(ModeSel_GPIO_Port, ModeSel_Pin);
			if(gModeSel == GPIO_PIN_SET)		//aging mode
			{
				if(runCnt > gTotalCycle)
				{
					printf("Run finished! Total %d.\r\n", runCnt);
					runCnt = 1;
					gBeepMode = BEEP_FINISH;
					gMode = 5;
				}
				else
				{
					printf("Run %d ...\r\n", runCnt);
					//display current run count
					gDisplay_BCD = ((runCnt/1000)<<24) | (((runCnt/100)%10)<<16) | (((runCnt/10)%10)<<8) | (runCnt%10);
					
					TurnRelay(1,1);		//Turn on relay 1
					TurnRelay(2,1);		//Turn on relay 2
					TurnRelay(3,1);
					HAL_Delay(2000);
					HAL_GPIO_WritePin(PwrBtn_Port, PwrBtn_Pin, (GPIO_PinState)GPIO_PIN_RESET);					//Generate low pulse for power_button
					HAL_Delay(500);
					HAL_GPIO_WritePin(PwrBtn_Port, PwrBtn_Pin, (GPIO_PinState)GPIO_PIN_SET);
					
					delayTime = 0;
					while(delayTime < gPowerOnTime*100)
					{
						HAL_Delay(9);
						if((gKeySet == 1) || (gPauseFlag == 1))
						{
							gKeySet = 0;
							gPauseFlag = 0;
							gMode = 4;		
							printf("Pause! Press set key to resume.\r\n");	
							break;
						}	
						delayTime ++;
					}
					
					if(delayTime < gPowerOnTime*100)	break;
					
					TurnRelay(1,0);		//Turn off relay 1
					TurnRelay(2,0);		//Turn off relay 2
					TurnRelay(3,0);
					delayTime = 0;		
					while(delayTime < gPowerCycleInterval*100)
					{
						HAL_Delay(9);	
						if((gKeySet == 1) || (gPauseFlag == 1))
						{
							gKeySet = 0;
							gPauseFlag = 0;
							gMode = 4;		
							printf("Pause! Press set key to resume.\r\n");	
							break;
						}
						delayTime ++;
					}
					
					if(delayTime < gPowerCycleInterval*100)	break;
					runCnt++;
				}	
			}
			else								//debug mode
			{
				if(runCnt > gTotalCycle)
				{
					printf("Run finished! Total %d.\r\n", runCnt);
					runCnt = 1;
					gBeepMode = BEEP_FINISH;
					gMode = 5;
				}
				else 
				{
					printf("Run %d ...\r\n", runCnt);
					//display current run count
					gDisplay_BCD = ((runCnt/1000)<<24) | (((runCnt/100)%10)<<16) | (((runCnt/10)%10)<<8) | (runCnt%10);
					
					
					TurnRelay(1,1);		//Turn on relay 1
					TurnRelay(2,1);		//Turn on relay 2
					TurnRelay(3,1);
					timeout = 0;
					while((gPowerOnOkFlag == 0)&&(timeout < gPowerOnTime*100))
					{
						HAL_Delay(9);
						if((gKeySet == 1) || (gPauseFlag == 1))
						{
							gKeySet = 0;
							gPauseFlag = 0;
							gMode = 4;		
							printf("Pause! Press set key to resume.\r\n");	
							break;
						}
						timeout ++;
					}
					
					if((timeout < gPowerOnTime*100) && (gPowerOnOkFlag == 0))	break;
										
					if(timeout >= gPowerOnTime*100)
					{
						printf("Error: system turn on failed!\r\n");
						//BeepAlert(BEEP_ALERT);
						gBeepMode = BEEP_ALERT;
						gMode = 4;
						break;
					}
					
					timeout = 0;
					while((gPowerOnOkFlag == 1)&&(timeout < gPowerOffDelaySec*100))
					{
						HAL_Delay(9);
						if((gKeySet == 1) || (gPauseFlag == 1))
						{
							gKeySet = 0;
							gPauseFlag = 0;
							gMode = 4;		
							printf("Pause! Press set key to resume.\r\n");	
							break;
						}
 						timeout ++;
					}
					
					TurnRelay(1,0);		//Turn off relay 1
					TurnRelay(2,0);		//Turn off relay 2
					TurnRelay(3,0);		//Turn off relay 3
					gPowerOnOkFlag = 0;
					gPowerOffDelaySec = 0;
					
					delayTime = 0;
					while(delayTime < gPowerCycleInterval*100)
					{
						HAL_Delay(9);
						if((gKeySet == 1) || (gPauseFlag == 1))
						{
							gKeySet = 0;
							gPauseFlag = 0;
							gMode = 4;		
							//printf("Pause! Press set key to resume.\r\n");	
							break;
						}	
						delayTime ++;
					}
					
					if(delayTime < gPowerCycleInterval*100)	break;
					runCnt++;
				}
			}
		}
		
		//Pause mode
		while(gMode  == 4)
		{
			if((gKeySet == 1) || (gResumeFlag == 1))
			{
				gKeySet = 0;
				gResumeFlag = 0;
				gPauseFlag = 0;
				if(gBeepMode == BEEP_OFF)
					gMode = 3;						
				else				
					gBeepMode = BEEP_OFF;
			}
		}
		
		//finish mode
		while(gMode  == 5)
		{
			if(gKeySet == 1)
			{
				gBeepMode = BEEP_OFF;
				gKeySet = 0;
				gMode = 0;		
				gPauseFlag = 0;
				gResumeFlag = 0;
				gPowerOffDelaySec = 0;
			}
		}
	}
}

/*!
    \brief      retarget the C library printf function to the USART
    \param[in]  .
      \arg        
    \param[out] none
    \retval     the char send to usart.
*/
/*  */
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1,  10);
    while(RESET == __HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE));
    return ch;
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint8_t sel = 0;
	uint8_t displayData = 0;
	static uint8_t	key1down = 0, key2down = 0, key3down = 0;
	static uint32_t	modeLedCnt = 0;
	static uint32_t	beepCnt = 0;
	
	//Refresh Nixie Tube
	if(htim->Instance == TIM2)
	{	
		//turn off all
		HAL_GPIO_WritePin(SEL0_GPIO_Port, SEL0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEL1_GPIO_Port, SEL1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEL2_GPIO_Port, SEL2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEL3_GPIO_Port, SEL3_Pin, GPIO_PIN_SET);
		
		displayData = gDisplay_BCD>>(sel*8);
	
		//set data
		HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, (GPIO_PinState)(NixieTube[displayData]&0x01));
		HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, (GPIO_PinState)((NixieTube[displayData]>>1)&0x01));
		HAL_GPIO_WritePin(D2_GPIO_Port, D2_Pin, (GPIO_PinState)((NixieTube[displayData]>>2)&0x01));
		HAL_GPIO_WritePin(D3_GPIO_Port, D3_Pin, (GPIO_PinState)((NixieTube[displayData]>>3)&0x01));
		HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, (GPIO_PinState)((NixieTube[displayData]>>4)&0x01));
		HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (GPIO_PinState)((NixieTube[displayData]>>5)&0x01));
		HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (GPIO_PinState)((NixieTube[displayData]>>6)&0x01));
		HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
		
		//turn on Nixie Tube
		switch(sel)
		{
			case 0:
				HAL_GPIO_WritePin(SEL3_GPIO_Port, SEL3_Pin, GPIO_PIN_RESET);
				break;
			case 1:
				HAL_GPIO_WritePin(SEL2_GPIO_Port, SEL2_Pin, GPIO_PIN_RESET);
				break;
			case 2:
				HAL_GPIO_WritePin(SEL1_GPIO_Port, SEL1_Pin, GPIO_PIN_RESET);
				break;
			case 3:
				HAL_GPIO_WritePin(SEL0_GPIO_Port, SEL0_Pin, GPIO_PIN_RESET);
				break;
			default:
				sel = 0;
				break;
		}
		
		if(sel < 3)
			sel++;
		else
			sel = 0;
		
		//show mode led
		modeLedCnt++;
		switch(gMode)
		{
			case 0:	//SET total cycle				
				if(modeLedCnt > 200)
						modeLedCnt = 0;
				else if(modeLedCnt > 100)
				{
					if(sel == 0)
					{
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
					}
					else
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}
				else
				{
					if(sel == 0)
					{
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					}
					else
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}
				break;
			case 1: //Set power on timer
				if(modeLedCnt > 200)
					modeLedCnt = 0;
				else if(modeLedCnt > 100)
				{
					if(sel == 0) 
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					else if(sel == 3)
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
					else
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}
				else
				{
					if(sel == 0) 
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					else if(sel == 3)
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					else
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}
				break;
			case 2: //set power cycle interval time
				if(modeLedCnt > 200)
					modeLedCnt = 0;
				else if(modeLedCnt > 100)
				{
					if((sel == 0) || (sel == 3))
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					else if(sel == 2)
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
					else
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}
				else
				{
					if((sel == 0) || (sel == 3))
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					else if(sel == 2)
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					else
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}
				break;
			case 3: //run mode
				if(modeLedCnt > 500)
					modeLedCnt = 0;
				else if(modeLedCnt > 400)
				{
					HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
				}
				else if(modeLedCnt > 300)
				{
					if((sel == 0) || (sel == 3) || (sel == 2))
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					else
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}
				else  if(modeLedCnt > 200)
				{
					if((sel == 0) || (sel == 3))
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					else
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}
				else  if(modeLedCnt > 100)
				{
					if(sel == 0)
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
					else
						HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				}

				break;
			case 4: //Pause mode
				if(modeLedCnt > 200)
					modeLedCnt = 0;
				else if(modeLedCnt > 100)
					HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
				break;
			case 5: //Finish mode				
					HAL_GPIO_WritePin(Dot_GPIO_Port, Dot_Pin, GPIO_PIN_RESET);
				break;
			default:break;
		}
	
		//Beep 
		beepCnt++;
		if(gBeepMode == BEEP_OFF)
		{
			beepCnt = 0;
			HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
		}
		else if(gBeepMode == BEEP_FINISH)
		{
			if(beepCnt > 500)
				beepCnt = 0;
			else if(beepCnt == 200)	//beep off
				HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
			else if(beepCnt == 1)
				HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
		}
		else if(gBeepMode == BEEP_ALERT)
		{
			if(beepCnt == 250)
				beepCnt = 0;
			else if(beepCnt == 165)
			{
				HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);	
			}
			else if(beepCnt > 160)	//1
			{
				if(beepCnt%20 > 9)
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);	
			}
			else if(beepCnt > 155)	//2
			{
				if(beepCnt%16 > 7)
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);	
			}
			else if(beepCnt > 150)	//3
			{
				if(beepCnt%12 > 5)
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);	
			}
			else if(beepCnt > 145)	//4
			{
				if(beepCnt%8 > 3)
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);	
			}
			else if(beepCnt > 140)	//5
			{
				if(beepCnt%4 > 1)
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);	
			}
			else if(beepCnt > 120)	//4
			{
				if(beepCnt%8 > 3)
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);	
			}
			else if(beepCnt > 80)	//3
			{
				if(beepCnt%12 > 5)
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);	
			}
			else if(beepCnt > 40)	//2
			{
				if(beepCnt%16 > 7)
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);	
			}
			else 	//1 
			{
				if(beepCnt%20 > 9)
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);					
			}
		}
	}
	//scan key every 10ms
	else if(htim->Instance == TIM3)
	{
		//key1	+
		if(HAL_GPIO_ReadPin(Key1_Inc_GPIO_Port, Key1_Inc_Pin) == GPIO_PIN_RESET)
		{			
			if((key1down > 70) && (key1down%10 == 0))	
			{
				gKeyIncrement = 1;
			}
			else if(key1down == 2)	//20ms
			{
				gKeyIncrement = 1;
			}
			
			if(key1down < 200)
				key1down++;
			else
				key1down = 100;
		}
		else
			key1down = 0;
		
		//key2 -
		if(HAL_GPIO_ReadPin(Key2_Dec_GPIO_Port, Key2_Dec_Pin) == GPIO_PIN_RESET)
		{			
			if((key2down > 70)&& (key2down%10 == 0))	//1s
			{
				gKeyDecrement = 1;
			}
			else if(key2down == 2)	//20ms
			{
				gKeyDecrement = 1;
			}
			
			if(key2down < 200)
				key2down++;
			else
				key2down = 100;
			
		}
		else
			key2down = 0;
		
		//key3 Pause/Resume
		if(HAL_GPIO_ReadPin(Key3_ResumeGPIO_Port, Key3_Resume_Pin) == GPIO_PIN_RESET)
		{			
			if(key3down == 2)	//20ms
			{
				gKeySet = 1;
			}
			
			if(key3down < 200)
				key3down++;
			else
				key3down = 100;
		}
		else
			key3down = 0;			
	}
}

void TurnRelay(uint8_t RelayIndex, uint8_t TurnOn)
{
	switch(RelayIndex)
	{
		case 1:
			HAL_GPIO_WritePin(Relay1_GPIO_Port, Relay1_Pin, (GPIO_PinState)TurnOn);
			break;
		case 2:
			HAL_GPIO_WritePin(Relay2_GPIO_Port, Relay2_Pin, (GPIO_PinState)TurnOn);
			break;
		case 3:
			HAL_GPIO_WritePin(Relay3_GPIO_Port, Relay3_Pin, (GPIO_PinState)TurnOn);
			break;
		default:
			printf("Invalid Relay Index %d\r\n", RelayIndex);
			break;
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

  /**Initializes the CPU, AHB and APB busses clocks 
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
  /**Initializes the CPU, AHB and APB busses clocks 
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7200-1;	//10KHZ
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 25;	//n*100us
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
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

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7200-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100;	//10ms
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, D0_Pin|D2_Pin|D4_Pin|D5_Pin|D6_Pin|Dot_Pin|Relay3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, D1_Pin|D3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, PwrBtn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SEL1_Pin|SEL2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SEL0_Pin|SEL3_Pin|Relay1_Pin 
                          |Relay2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB,Relay1_Pin|Relay2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Key1_Inc_Pin Key1_Dec_Pin Key3_Resume_Pin */
  GPIO_InitStruct.Pin = Key1_Inc_Pin|Key2_Dec_Pin|Key3_Resume_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : D0_Pin D1_Pin D2_Pin D3_Pin 
                           D4_Pin D5_Pin D6_Pin SEL0_Pin */
  GPIO_InitStruct.Pin = D0_Pin|D2_Pin|D4_Pin|D5_Pin|D6_Pin|Dot_Pin|SEL1_Pin|SEL2_Pin|Relay3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = D1_Pin|D3_Pin|PwrBtn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  

  /*Configure GPIO pins : SEL1_Pin SEL2_Pin SEL3_Pin Relay1_Pin 
                           Relay2_Pin Beep_Pin */
  GPIO_InitStruct.Pin = SEL0_Pin|SEL3_Pin|Relay1_Pin|Relay2_Pin|Beep_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ModeSel_Pin */
  GPIO_InitStruct.Pin = ModeSel_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(ModeSel_GPIO_Port, &GPIO_InitStruct);

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
