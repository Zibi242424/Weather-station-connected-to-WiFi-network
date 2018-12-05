#include <stdio.h>
#include <stdint.h>
#include ".\Includes\Init.h"
#include ".\Includes\LCD.h"
#include "stm32f10x.h"
#include ".\Includes\Bitmap.h"
#include ".\Includes\Defines.h"

void LCD_PrintTime(int row, int Hour, int Minute, int Second);
void LCD_PrintTemperature(int row, float Temperature);
void LCD_PrintPressure(int row,  int Pressure);

volatile unsigned int Hour;
volatile unsigned int Minute;
volatile unsigned int Second;

volatile unsigned int HourRTC;
volatile unsigned int MinuteRTC;
volatile unsigned int SecondRTC;

volatile uint8_t Display = RESET;

volatile uint8_t ConnectedToWiFi = RESET;
volatile uint8_t ConnectedToBMP280 = RESET;
volatile uint8_t UpdateRTC = RESET;
volatile uint8_t TimeUpdated = RESET;

volatile float Temperature;
volatile int Pressure;

unsigned int State = 0;

int ticks;

void delay_ms(int i){
	ticks = 0;
	while(ticks < i);
}

static uint8_t SPI2_SendRecv(uint8_t byte)
{
 // poczekaj az bufor nadawczy bedzie wolny
 while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
 SPI_I2S_SendData(SPI2, byte);

 // poczekaj na dane w buforze odbiorczym
 while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
 return SPI_I2S_ReceiveData(SPI2);
}

int main(void)
{
	RCC_init();
	GPIO_init();
	SPI_init();
	USART_init();
	IWDG_init();

	ADC_init();
	TIM_init();

	RCC_ClocksTypeDef clock;
	RCC_GetClocksFreq(&clock);

	long int SYSclk, Hclk, Pclk1, Pclk2;
	SYSclk = clock.SYSCLK_Frequency;
	Hclk = clock.HCLK_Frequency;
	Pclk1 = clock.PCLK1_Frequency;
	Pclk2 = clock.PCLK2_Frequency;

	SysTick_Config(SystemCoreClock / 1000);

	LCD_Setup();
	LCD_Clear();
	LCD_DrawBitmap(Start_logo);
	LCD_Copy();
	delay_ms(4000);

	/* Wait for the WiFi connection */
	int i = 0;
	int ErrorCounter = 0;
	TIM2 -> CCR2 = 0;
	while(ConnectedToWiFi == RESET){
		i = 0;
		while(i<18){
			LCD_Clear();
			LCD_DrawBitmap(Gif[i]);
			LCD_Copy();
			TIM2->CCR2 += 55;
			delay_ms(25);
			i++;
		}
		i = 16;
		TIM2 -> CCR2 = 990;
		while(i>0){
			LCD_Clear();
			LCD_DrawBitmap(Gif[i]);
			LCD_Copy();
			TIM2->CCR2 -= 55;
			delay_ms(25);
			i--;
		}
		ErrorCounter++;
		/* If after multiple tries no WiFi connection was set - proceed*/
		if (ErrorCounter == 33){
			ConnectedToWiFi == SET;
			break;
		}
	}
	TIM2 -> CCR2 = 350;
	if(ConnectedToWiFi == RESET){
		LCD_Clear();
		LCD_DrawText(0, 0, "NO WiFi Conn.");
		if (ConnectedToBMP280 == RESET){
			LCD_DrawText(2, 0, "NO BMP Conn.");
		}
		LCD_Copy();
		int CurrentRTC = RTC_GetCounter();
		delay_ms(5000);
		if (CurrentRTC == RTC_GetCounter()){
			RTC_init(12, 0, 0);
		}else{
			RTC_ITConfig(RTC_IT_SEC, ENABLE);
			NVIC_EnableIRQ(RTC_IRQn);
		}
	}else{
		LCD_Clear();
		LCD_DrawText(0, 0, "WiFi CONNECTED");
		LCD_DrawText(5, 0, "Please wait...");
		LCD_Copy();
		while (State != TIME_SENT);
		while (TimeUpdated != SET);
		RTC_init(HourRTC, MinuteRTC, SecondRTC);
	}

	IWDG_ReloadCounter();
	IWDG_Enable();
	char c[10];
	Display = SET;
	while(1){
		if(UpdateRTC == SET){
			RTC_SetCounter(3600*HourRTC + MinuteRTC*60 + SecondRTC);
			UpdateRTC = RESET;
		}
		if (Display == SET){
			int Time = RTC_GetCounter() % 86400;
			Hour = Time/3600;
			Minute = (Time%3600)/60;
			Second = (Time%3600)%60;
			LCD_Clear();
			if (TimeUpdated == SET){
				LCD_DrawText(0, 75, "U");
				TimeUpdated = RESET;
			}
			if(ConnectedToWiFi == RESET){
				LCD_DrawText(5, 0, "NO WiFi");
			}
			if(ConnectedToBMP280 == RESET){
				LCD_DrawText(5, 42, " | BMP");
			}
			LCD_PrintTime(0, Hour, Minute, Second);
			LCD_PrintTemperature(1, Temperature);
			LCD_PrintPressure(3, Pressure);

			Display = RESET;
		}

	};



}

void LCD_PrintTime(int row, int Hour, int Minute, int Second){
	char c[10];
	if(row >= 6){
		row = 6;
	}
	sprintf(c, "%d", Hour);
	if(Hour < 10){
		LCD_DrawText(row, 0, "0");
		LCD_DrawText(row, 6, c);
	}else LCD_DrawText(row, 0, c);
	LCD_DrawText(row, 14, ":");
	sprintf(c, "%d", Minute);
	if(Minute < 10){
		LCD_DrawText(row, 21, "0");
		LCD_DrawText(row, 27, c);
	}else LCD_DrawText(row, 21, c);
	LCD_DrawText(row, 35, ":");
	sprintf(c, "%d", Second);
	if(Second < 10){
		LCD_DrawText(row, 42, "0");
		LCD_DrawText(row, 48, c);
	}else LCD_DrawText(row, 42, c);
	LCD_Copy();
}

void LCD_PrintTemperature(int row, float Temperature){
	if(row >= 5){
		row = 5;
	}
	char c[10];
	LCD_DrawText(row, 0, "Temperatura:");
	sprintf(c, "%d", (int)Temperature);
	LCD_DrawText(row+1, 0, c);
	LCD_DrawText(row+1, 14, ".");
	sprintf(c, "%d", ((int)(Temperature*100)) % 100);
	if ((((int)(Temperature*100)) % 100) < 10){
		LCD_DrawText(row+1, 21, "0");
		LCD_DrawText(row+1, 27, c);
		LCD_DrawText(row+1, 34, "C");
	}else {
		LCD_DrawText(row+1, 21, c);
		LCD_DrawText(row+1, 34, "C");
	}
	LCD_Copy();
}

void LCD_PrintPressure(int row,  int Pressure){
	if(row >= 5){
		row = 5;
	}
	Pressure = Pressure;
	char c[10];
	LCD_DrawText(row, 0, "Cisnienie:");
	sprintf(c, "%d", Pressure);
	LCD_DrawText(row+1, 0, c);
	if (Pressure >= 1000){
		LCD_DrawText(row+1, 26, "hPa");
	}else{
		LCD_DrawText(row+1, 19, "hPa");
	}
	LCD_Copy();
}

