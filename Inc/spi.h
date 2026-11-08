#ifndef SPI_H_
#define SPI_H_
#include <stdint.h>

#define STM32F411xE
#include "stm32f4xx.h"

void SPI1_init(void);
void SPI1_rx_init(void);

void SPI1_read_bytes(uint8_t cmd, uint8_t* data, uint8_t* sr, uint8_t n);
void SPI1_write_bytes(uint8_t cmd, uint8_t* data, uint8_t* status, uint8_t n);
void SPI1_rx_read_bytes(uint8_t cmd, uint8_t* data, uint8_t* sr, uint8_t n);
void SPI1_rx_write_bytes(uint8_t cmd, uint8_t* data, uint8_t* status, uint8_t n);
void SPI1_command(uint8_t cmd);
void SPI1_rx_command(uint8_t cmd);

#endif
