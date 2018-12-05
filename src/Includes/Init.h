/*
 * Init.h
 *
 *  Created on: 21.09.2018
 *      Author: manse
 */

#ifndef INCLUDES_INIT_H_
#define INCLUDES_INIT_H_

#include "stm32f10x.h"
#include "Defines.h"

void RCC_init();
void GPIO_init();
void USART_init();
void SPI_init();
void I2C_init();
void TIM_init();
void IWDG_init();
void RTC_init(int H, int M, int S);

#endif /* INCLUDES_INIT_H_ */
