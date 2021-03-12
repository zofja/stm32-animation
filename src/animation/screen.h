#ifndef CONFIG_LCD_CONFIG_H
#define CONFIG_LCD_CONFIG_H

#include <lcd_board_def.h>

#define LCD_PIXEL_WIDTH   128
#define LCD_PIXEL_HEIGHT  160
#define LCD_SIZE (LCD_PIXEL_HEIGHT * LCD_PIXEL_WIDTH)

void LCDconfigure(void);

#endif // CONFIG_LCD_CONFIG_H
