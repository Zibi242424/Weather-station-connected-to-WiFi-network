/*
 * Defines.h
 *
 *  Created on: 21.09.2018
 *      Author: manse
 */

#ifndef INCLUDES_DEFINES_H_
#define INCLUDES_DEFINES_H_

#define LCD_DC			GPIO_Pin_6	//12
#define LCD_CE			GPIO_Pin_7	//13
#define LCD_RST			GPIO_Pin_8	//14
#define LCD_GPIO_PORT   GPIOB

#define TEMP_SENSOR_AVG_SLOPE_MV_PER_CELSIUS                        2.5f
#define TEMP_SENSOR_VOLTAGE_MV_AT_25                                760.0f
#define ADC_REFERENCE_VOLTAGE_MV                                    3300.0f
#define ADC_MAX_OUTPUT_VALUE                                        4095.0f
#define TEMP110_CAL_VALUE                                           ((uint16_t*)((uint32_t)0x1FFF7A2E))
#define TEMP30_CAL_VALUE                                            ((uint16_t*)((uint32_t)0x1FFF7A2C))
#define TEMP110                                                     110.0f
#define TEMP30                                                      30.0f

enum State {IDLE = 0, STARTED = 1, TIME_SENT = 10, TEMP_SENT = 11, PRESSURE_SENT = 12, STOP = 13};
#endif /* INCLUDES_DEFINES_H_ */
