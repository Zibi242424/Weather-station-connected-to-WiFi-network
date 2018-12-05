#include <string.h>
#include "..\Includes\LCD.h"
#include "..\Includes\Defines.h"

#define PCD8544_FUNCTION_SET		0x20
#define PCD8544_DISP_CONTROL		0x08
#define PCD8544_DISP_NORMAL			0x0c
#define PCD8544_SET_Y				0x40
#define PCD8544_SET_X				0x80
#define PCD8544_H_TC				0x04
#define PCD8544_H_BIAS				0x10
#define PCD8544_H_VOP				0x80

#define LCD_BUFFER_SIZE			(84 * 48 / 8)

uint8_t lcd_buffer[LCD_BUFFER_SIZE];

static uint8_t SPI_SendRecv(uint8_t byte)
{
 // poczekaj az bufor nadawczy bedzie wolny
 while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
 SPI_I2S_SendData(SPI1, byte);

 // poczekaj na dane w buforze odbiorczym
 while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
 return SPI_I2S_ReceiveData(SPI1);
}

static void LCD_CMD(uint8_t cmd)
{
 GPIO_ResetBits(LCD_GPIO_PORT, LCD_CE|LCD_DC);
 SPI_SendRecv(cmd);
 GPIO_SetBits(LCD_GPIO_PORT, LCD_CE);
}

void LCD_Setup(void)
{
 GPIO_ResetBits(LCD_GPIO_PORT, LCD_RST);
 GPIO_SetBits(LCD_GPIO_PORT, LCD_RST);

 LCD_CMD(PCD8544_FUNCTION_SET | 1);
 LCD_CMD(PCD8544_H_BIAS | 4);
 LCD_CMD(0x80 | 0x3f); //Ustawienie kontrastu
 LCD_CMD(PCD8544_FUNCTION_SET);
 LCD_CMD(PCD8544_DISP_NORMAL);

}

void LCD_Clear(void)
{
 memset(lcd_buffer, 0, LCD_BUFFER_SIZE);
}

void LCD_DrawBitmap(const uint8_t* data)
{
 memcpy(lcd_buffer, data, LCD_BUFFER_SIZE);
}

void LCD_DrawText(int row, int col, const char* text)
{
 int i;
 uint8_t* pbuf = &lcd_buffer[row * 84 + col];
 while ((*text) && (pbuf < &lcd_buffer[LCD_BUFFER_SIZE - 6])) {
 int ch = *text++;
 const uint8_t* font = &font_ASCII[ch - ' '][0];
 for (i = 0; i < 5; i++) {
 *pbuf++ = *font++;
 }
 *pbuf++ = 0;
 }
}

void LCD_Copy(void)
{
 int i;
 GPIO_SetBits(LCD_GPIO_PORT, LCD_DC);
 GPIO_ResetBits(LCD_GPIO_PORT, LCD_CE);
 for (i = 0; i < LCD_BUFFER_SIZE; i++)
 SPI_SendRecv(lcd_buffer[i]);
 GPIO_SetBits(LCD_GPIO_PORT, LCD_CE);
}

void LCD_Data(const uint8_t* data, int size)
{
 int i;
 GPIO_SetBits(LCD_GPIO_PORT, LCD_DC);
 GPIO_ResetBits(LCD_GPIO_PORT, LCD_CE);
 for (i = 0; i < size; i++)
 SPI_SendRecv(data[i]);
 GPIO_SetBits(LCD_GPIO_PORT, LCD_CE);
}
