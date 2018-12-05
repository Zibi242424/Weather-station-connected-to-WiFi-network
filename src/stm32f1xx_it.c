/**
  ******************************************************************************
  * @file    stm32f1xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-February-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_it.h"
#include ".\Includes\Defines.h"
#include <unistd.h>



extern void LCD_PrintTime(int, int, int, int);
extern void LCD_PrintTemperature(int, int);

extern volatile unsigned int Hour;
extern volatile unsigned int Minute;
extern volatile unsigned int Second;

extern volatile unsigned int HourRTC;
extern volatile unsigned int MinuteRTC;
extern volatile unsigned int SecondRTC;

extern volatile float Temperature;
extern volatile  int Pressure;

extern volatile uint8_t Display;

volatile uint8_t ConnectedToWiFi;
volatile uint8_t ConnectedToBMP280;
extern volatile uint8_t UpdateRTC;
extern volatile uint8_t TimeUpdated;

uint16_t DutyCycle [24] = { 5, 5, 25, 50,
		100, 200, 250, 400, 500, 600, 800, 800,
		900, 900, 800, 700, 600, 550, 500, 400,
		300, 200, 10, 10
};

extern volatile  int State;


extern int ticks;

/** @addtogroup IO_Toggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M Processor Exceptions Handlers                          */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */

void SysTick_Handler(void)
{
	ticks++;
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_md.s).                                            */
/******************************************************************************/

void USART3_IRQHandler(){
	if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET){
		char c = USART3->DR;
		if(State != STOP){
			if (c == '$'){
				State = STOP;
			}
			if (c == 'w' && ConnectedToWiFi == RESET){
				ConnectedToWiFi = SET;
			}
			if (c == 'b' && ConnectedToBMP280 == RESET){
				ConnectedToBMP280 = SET;
			}
			if (c == 'h' && (State == TIME_SENT || State == IDLE || State == TEMP_SENT || State == PRESSURE_SENT)){
				State = STARTED;
			}else if (State == 1){
				HourRTC = c - '\0';
				State = 2;
			}else if (State == 2){
				MinuteRTC = c - '\0';
				State = 3;
			}else if (State == 3){
				SecondRTC  = c - '\0';
				TimeUpdated = SET;
				UpdateRTC = SET;
				State = TIME_SENT;
			}else if(c == 't'){
				State = 4;
			}else if (State == 4){
				Temperature = c - '\0';
				State = 5;
			}else if (State == 5){
				int x = c - '\0';
				Temperature += (float)x/100.0;
				State = TEMP_SENT;
			}else if (c == 'p'){
				State = 6;
			}else if (State == 6){
				Pressure = (c - '\0')*100;
				State = 7;
			}else if (State == 7){
				Pressure += (c - '\0');
				State = PRESSURE_SENT;
				Display = SET;
			}else {
				State = IDLE;
			}
		}else if (State == STOP && c == '^'){
			State = IDLE;
		}
	}
	IWDG_ReloadCounter();
	USART_ClearFlag(USART3, USART_FLAG_RXNE);
}


void RTC_IRQHandler(){
	if(RTC_GetFlagStatus(RTC_FLAG_SEC) == SET){
		Display = SET;
		IWDG_ReloadCounter();

	}
	RTC_ClearITPendingBit(RTC_IT_SEC);
}


void TIM1_UP_IRQHandler(){
	if (TIM_GetITStatus(TIM1, TIM_FLAG_Update) == SET){
		/* Reset watchdog */
		IWDG_ReloadCounter();
		TIM2 -> CCR2 = DutyCycle[Hour];

	}
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
