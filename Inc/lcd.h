#ifndef LCD_H_
#define LCD_H_
#include <stdint.h>

void lcd_write_character(int c);

void lcd_init();
void lcd_clear();
void lcd_set_cursor(uint8_t col, uint8_t row);

#endif
