#include "spi.h"
#include "systick.h"

#define LSBFIRST 						(1U << 7)
#define SSOE 							(1U << 2)

#define MSTR 							(1U << 2)

#define SPE 							(1U << 6)

#define TXE	 							(1U << 1)
#define RXNE 							(1U << 0)

#define BIDIMODE						(1U << 15)
#define BIDIOE							(1U << 14)
#define RXONLY							(1U << 10)
#define SPI1_EN 						(1U << 12)
#define GPIOA_EN						(1U << 0)


#define CE_RX_PIN_NUM					(12U)
#define CSN_RX_PIN_NUM					(11U)
#define CE_PIN_NUM						(9U)
#define CSN_PIN_NUM						(4U)

#define CSN_PIN							(1U << CSN_PIN_NUM)
#define CE_PIN							(1U << CE_PIN_NUM)

#define CSN_RX_PIN						(1U << CSN_RX_PIN_NUM)
#define CE_RX_PIN						(1U << CE_RX_PIN_NUM)

#define CPOL							(1U << 1)
#define CPHA							(1U << 0)
#define SSM 							(1U << 9)
#define SSI 							(1U << 8)

#define BSY								(1U << 7)
//CPOL & CPHA = 0,0; 8bit mode => DFF = 0; Baud rate = Fclk/2 => BR = 000;

//SPI 1 -- PA4-7 -- AF05 - NSS,SCK,MISO,MOSI -- CLK IDLE - 0, SAMPLE ON RISE-> CPOL = 0, CPHA = 0;
//PRIOR TO SETTING CPOL & CPHA, DISABLE SPI -- SPE BIT
void SPI1_rx_init(void)
{
	//enable clock, peripheral, GPIO
	RCC->APB2ENR |= SPI1_EN;
	RCC->AHB1ENR |= GPIOA_EN;

	//MODE TO ALTERNATIVE FUNCTION

	GPIOA->MODER &= ~(3U << 2 * 5);
	GPIOA->MODER |= (2U << 2 * 5);

	GPIOA->MODER &= ~(3U << 2 * 6);
	GPIOA->MODER |= (2U << 2 * 6);

	GPIOA->MODER &= ~(3U << 2 * 7);
	GPIOA->MODER |= (2U << 2 * 7);
	//PA9 for CE control
	GPIOA->MODER &= ~(3U << 2 * CE_RX_PIN_NUM);//CE
	GPIOA->MODER |= (1U << 2 * CE_RX_PIN_NUM);

	GPIOA->MODER &= ~(3U << 2 * CSN_RX_PIN_NUM);//CSN/NSS
	GPIOA->MODER |= (1U << 2 * CSN_RX_PIN_NUM);

	GPIOA->ODR |= CSN_RX_PIN;
//	GPIOA->ODR |= CE_RX_PIN;

	//AF05 for SPI pins
	GPIOA->AFR[0] &= ~(0xF << 4 * 5);
	GPIOA->AFR[0] |= (5U << 4 * 5);

	GPIOA->AFR[0] &= ~(0xF << 4 * 6);
	GPIOA->AFR[0] |= (5U << 4 * 6);

	GPIOA->AFR[0] &= ~(0xF << 4 * 7);
	GPIOA->AFR[0] |= (5U << 4 * 7);

	SPI1->CR1 |= SSM | SSI;
	SPI1->CR1 |= MSTR;
	SPI1->CR1 |= SPE;
}

void SPI1_init(void)
{
	//enable clock, peripheral, GPIO
	RCC->APB2ENR |= SPI1_EN;
	RCC->AHB1ENR |= GPIOA_EN;

	//MODE TO ALTERNATIVE FUNCTION
	GPIOA->MODER &= ~(3U << 2 * CSN_PIN_NUM);//CSN/NSS
	GPIOA->MODER |= (1U << 2 * CSN_PIN_NUM);

	GPIOA->MODER &= ~(3U << 2 * 5);
	GPIOA->MODER |= (2U << 2 * 5);

	GPIOA->MODER &= ~(3U << 2 * 6);
	GPIOA->MODER |= (2U << 2 * 6);

	GPIOA->MODER &= ~(3U << 2 * 7);
	GPIOA->MODER |= (2U << 2 * 7);
	//PA9 for CE control
	GPIOA->MODER &= ~(3U << 2 * CE_PIN_NUM);//CE
	GPIOA->MODER |= (1U << 2 * CE_PIN_NUM);

	GPIOA->ODR |= CSN_PIN;
	GPIOA->ODR &= ~CE_PIN;

	//AF05 for SPI pins
	GPIOA->AFR[0] &= ~(0xF << 4 * 5);
	GPIOA->AFR[0] |= (5U << 4 * 5);

	GPIOA->AFR[0] &= ~(0xF << 4 * 6);
	GPIOA->AFR[0] |= (5U << 4 * 6);

	GPIOA->AFR[0] &= ~(0xF << 4 * 7);
	GPIOA->AFR[0] |= (5U << 4 * 7);

	SPI1->CR1 |= SSM | SSI;
	SPI1->CR1 |= MSTR;

	SPI1->CR1 |= SPE;
}

void SPI1_write_bytes(uint8_t cmd, uint8_t* data, uint8_t* status, uint8_t n)
{
	GPIOA->ODR &= ~CSN_PIN;

	while(!(SPI1->SR & TXE)) {}
	SPI1->DR = cmd;

	while(!(SPI1->SR & RXNE)) {}
	*status = SPI1->DR;

	while(n-- > 0)
	{
		while(!(SPI1->SR & TXE)) {}
		SPI1->DR = *data++;

        while(!(SPI1->SR & RXNE)) {}
        volatile uint8_t dummy = SPI1->DR;
	}

	while(SPI1->SR & BSY) {}

	GPIOA->ODR |= CSN_PIN;
}
void SPI1_rx_write_bytes(uint8_t cmd, uint8_t* data, uint8_t* status, uint8_t n)
{
	GPIOA->ODR &= ~CSN_RX_PIN;
	SPI1->CR1 |= SPE;

	while(!(SPI1->SR & TXE)) {}
	SPI1->DR = cmd;

	while(!(SPI1->SR & RXNE)) {}
	*status = SPI1->DR;
	while(n-- > 0)
	{
		while(!(SPI1->SR & TXE)) {}
		SPI1->DR = *data++;

        while(!(SPI1->SR & RXNE)) {}
        volatile uint8_t dummy = SPI1->DR;
	}

	while(SPI1->SR & BSY) {}

	GPIOA->ODR |= CSN_RX_PIN;
}

void SPI1_command(uint8_t cmd)
{
    GPIOA->ODR &= ~CSN_PIN;

    while(!(SPI1->SR & TXE)){}
    SPI1->DR = cmd;

    while(!(SPI1->SR & RXNE)){}
    volatile uint8_t temp = SPI1->DR;

    while(SPI1->SR & BSY){}

    GPIOA->ODR |= CSN_PIN;
}

void SPI1_rx_command(uint8_t cmd)
{
    GPIOA->ODR &= ~CSN_RX_PIN;

    while(!(SPI1->SR & TXE)){}
    SPI1->DR = cmd;

    while(!(SPI1->SR & RXNE)){}
    volatile uint8_t temp = SPI1->DR;

    while(SPI1->SR & BSY){}

    GPIOA->ODR |= CSN_RX_PIN;
}

void SPI1_read_bytes(uint8_t cmd, uint8_t* data, uint8_t* sr, uint8_t n)
{
	GPIOA->ODR &= ~CSN_PIN;

	while(!(SPI1->SR & TXE)) {}
	SPI1->DR = cmd;

	while(!(SPI1->SR & RXNE)) {}
	*sr = SPI1->DR;

	for(uint8_t i = 0; i < n; i++)
	{
		while(!(SPI1->SR & TXE)) {}
		SPI1->DR = 0x0;

		while(!(SPI1->SR & RXNE)) {}
		*data++ = SPI1->DR;

		while(SPI1->SR & RXNE)
		{
			volatile uint8_t temp = SPI1->DR;
		}
	}

	while(SPI1->SR & BSY) {}

	GPIOA->ODR |= CSN_PIN;
}

void SPI1_rx_read_bytes(uint8_t cmd, uint8_t* data, uint8_t* sr, uint8_t n)
{
	GPIOA->ODR &= ~CSN_RX_PIN;

	while(!(SPI1->SR & TXE)) {}
	SPI1->DR = cmd;

	while(!(SPI1->SR & RXNE)) {}
	*sr = SPI1->DR;

	for(uint8_t i = 0; i < n; i++)
	{
		while(!(SPI1->SR & TXE)) {}
		SPI1->DR = 0x0;

		while(!(SPI1->SR & RXNE)) {}
		*data++ = SPI1->DR;

		while(SPI1->SR & RXNE)
		{
			volatile uint8_t temp = SPI1->DR;
		}
	}

	while(SPI1->SR & BSY) {}

	GPIOA->ODR |= CSN_RX_PIN;
}
