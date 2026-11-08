#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

#define I2C1EN					(1U << 21)
#define GPIOBEN 				(1U << 1)
#define AF						(0x2)
#define AF04					(0x4)
#define PU						(0x1)

#define PE 						(0x1)
#define SB						(0x1)
#define BUSY					(0x2)
#define TXE						(1U << 7)
#define ADDR					(1U << 1)
#define RXNE 					(1U << 6)
#define ACK						(1U << 10)
#define BTF						(1U << 2)
#define STOP 					(1U << 9)
#define SWRST					(1U << 15)
#define I2C1_100KHZ				80U
#define SM_MODE_RISE_TIME		17U
#define START 					(1U << 8)

void I2C1_burst_write(char saddr, char maddr, char n, uint8_t* data);

void I2C1_burst_write_direct(char saddr, char n, char* data);

void I2C1_burst_read(char saddr, char maddr, char n, uint8_t* data);

void I2C1_byte_read(char saddr, char maddr, char* data);

void I2C1_init(void);

#endif
