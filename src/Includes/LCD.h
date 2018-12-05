#ifndef __LCD__
#define __LCD__

#include <stdint.h>
#include "stm32f10x.h"
#include "Font.h"

void LCD_Setup(void);

void LCD_Clear(void);
void LCD_DrawBitmap(const uint8_t* data);
void LCD_DrawText(int row, int col, const char* text);

void LCD_Copy(void);

void LCD_Data(const uint8_t* data, int size);

#endif // __LCD__
