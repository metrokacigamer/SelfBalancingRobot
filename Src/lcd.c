#include "lcd.h"
#include "i2c.h"
#include "systick.h"

#define BPACK_ADDR 0x27


//bits 0-1
#define RS_WRITE 						(0x1)//
//bit 2
#define EN								(0x4)//
//bit 3
#define BACKLIGHT_ON					(0x8)
//bits 4-7
#define DATA_4BIT						(0xF0)// AND WITH


void lcd_init()
{
	uint8_t data[2];

	uint8_t data_4bit[4];
	I2C1_init();
	//bit mapping D7..4 = P7..4;
	//init procedure, 8bit mode
	systickDelayMs(15);

	data[0] = ((0x3 << 4) & DATA_4BIT) | BACKLIGHT_ON | EN;
	data[1] = BACKLIGHT_ON;
	I2C1_burst_write_direct(BPACK_ADDR, 2, (char *)data);

	systickDelayMs(5);

	I2C1_burst_write_direct(BPACK_ADDR, 2, (char *)data);

	systickDelayMs(1);

	I2C1_burst_write_direct(BPACK_ADDR, 2, (char *)data);

	data[0] = ((0x2 << 4) & DATA_4BIT) | BACKLIGHT_ON | EN;
	data[1] = BACKLIGHT_ON;
	I2C1_burst_write_direct(BPACK_ADDR, 2, (char *)data);//function set; DL = 0, 4bit mode;
	systickDelayMs(1);
	data_4bit[0] = ((0x2 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[1] = BACKLIGHT_ON;

	data_4bit[2] = ((0x8 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[3] = BACKLIGHT_ON;
	I2C1_burst_write_direct(BPACK_ADDR, 4, (char *)data_4bit);//function set; N = 2rows, Font = 5x8;
	systickDelayMs(1);
	data_4bit[0] = ((0x0 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[1] = BACKLIGHT_ON;

	data_4bit[2] = ((0x8 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[3] = BACKLIGHT_ON;
	I2C1_burst_write_direct(BPACK_ADDR, 4, (char *)data_4bit);//display off
	systickDelayMs(1);
	data_4bit[0] = ((0x0 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[1] = BACKLIGHT_ON;

	data_4bit[2] = ((0x1 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[3] = BACKLIGHT_ON;
	I2C1_burst_write_direct(BPACK_ADDR, 4, (char *)data_4bit);//display clear
	systickDelayMs(1);
	data_4bit[0] = ((0x0 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[1] = BACKLIGHT_ON;

	data_4bit[2] = ((0x6 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[3] = BACKLIGHT_ON;
	I2C1_burst_write_direct(BPACK_ADDR, 4, (char *)data_4bit);//input set; inc = 1; shift = 0
	systickDelayMs(1);
	data_4bit[0] = ((0x0 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[1] = BACKLIGHT_ON;

	data_4bit[2] = ((0x0F << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[3] = BACKLIGHT_ON;
	I2C1_burst_write_direct(BPACK_ADDR, 4, (char *)data_4bit);//display on, cursor on, blink on
	systickDelayMs(1);
}
////bits 0-1
//#define RS_WRITE 						(0x2)
////bit 2
//#define EN							(0x4)
////bit 3
//#define BACKLIGHT_ON					(0x8)
////bits 4-7
//#define DATA_4BIT						(0xF0)
void lcd_write_character(int c)
{
	uint8_t data_4bit[4];//MSB FIRST, disable EN afterwards
	data_4bit[0] = 		  (c & DATA_4BIT) | BACKLIGHT_ON | EN | RS_WRITE;
	data_4bit[1] = BACKLIGHT_ON | RS_WRITE;//EN OFF

	data_4bit[2] = ((c << 4) & DATA_4BIT) | BACKLIGHT_ON | EN | RS_WRITE;
	data_4bit[3] = BACKLIGHT_ON | RS_WRITE;
	I2C1_burst_write_direct(BPACK_ADDR, 4, (char *)data_4bit);
	systickDelayMs(1);
}

void lcd_clear()
{
	uint8_t data_4bit[4];

	data_4bit[0] = ((0x0 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[1] = BACKLIGHT_ON;

	data_4bit[2] = ((0x1 << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
	data_4bit[3] = BACKLIGHT_ON;
	I2C1_burst_write_direct(BPACK_ADDR, 4, (char *)data_4bit);//display clear
	systickDelayMs(2);
}

void lcd_send_command(uint8_t cmd)
{
    uint8_t data[4];

    data[0] = (cmd & DATA_4BIT) | EN | BACKLIGHT_ON;
    data[1] = BACKLIGHT_ON;

    data[2] = ((cmd << 4) & DATA_4BIT) | EN | BACKLIGHT_ON;
    data[3] = BACKLIGHT_ON;

    I2C1_burst_write_direct(BPACK_ADDR, 4, (char *)data);

    systickDelayMs(1);
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    uint8_t row_offsets[] = {0x00, 0x40};

    lcd_send_command(0x80 | (col + row_offsets[row]));
}
