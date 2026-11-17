#include "i2c.h"

#define STM32F411xE

#include "stm32f4xx.h"


void I2C1_init(void)
{
	RCC->AHB1ENR |= GPIOBEN;
	//PORTB

	GPIOB->MODER &= ~(3 << 2 * 8);
	GPIOB->MODER |= (AF << 2 * 8);

	GPIOB->MODER &= ~(3 << 2 * 9);
	GPIOB->MODER |= (AF << 2 * 9);

	//output type - open-drain, enable pull-ups
	GPIOB->OTYPER |= (1U << 8);
	GPIOB->OTYPER |= (1U << 9);

	GPIOB->PUPDR &= ~(3 << 2 * 8);
	GPIOB->PUPDR |= (PU << 2 * 8);

	GPIOB->PUPDR &= ~(3 << 2 * 9);
	GPIOB->PUPDR |= (PU << 2 * 9);

	//PB9 PB8 AF pins, AF04, PB9 - SDA PB8 - SCL.
	GPIOB->AFR[1] &= ~(0xF << (4 * 0));//8-8
	GPIOB->AFR[1] |= (AF04 << (4 * 0));//8-8

	GPIOB->AFR[1] &= ~(0xF << (4 * 1));//9-8
	GPIOB->AFR[1] |= (AF04 << (4 * 1));//9-8

	//enable I2C1
	RCC->APB1ENR |= I2C1EN;

	I2C1->CR1 |= SWRST;

	I2C1->CR1 &= ~SWRST;

	I2C1->CR2 = 1U << 4;//16MHZ

	I2C1->CCR = I2C1_100KHZ;

	I2C1->TRISE = SM_MODE_RISE_TIME;

	I2C1->CR1 |= PE;
}

void I2C1_byte_read(char saddr, char maddr, char* data)
{
	volatile int tmp;

	while(I2C1->SR2 & BUSY){}
	I2C1->CR1 |= START;

	while(!(I2C1->SR1 & SB)){}
	I2C1->DR = saddr << 1;

	while(!(I2C1->SR1 & ADDR)){}
	tmp = I2C1->SR2;

	while(!(I2C1->SR1 & TXE)){}
	I2C1->DR = maddr;

	while(!(I2C1->SR1 & TXE)){}
	I2C1->CR1 |= START;

	while(!(I2C1->SR1 & SB)){}
	I2C1->DR = saddr << 1 | 1;

	while(!(I2C1->SR1 & ADDR)){}
	I2C1->CR1 &= ~ACK;

	tmp = I2C1->SR2;

	I2C1->CR1 |= STOP;

	while(!(I2C1->SR1 & RXNE)){}
	*data++ = I2C1->DR;
}

void I2C1_burst_read(char saddr, char maddr, char n, uint8_t * data)
{
	volatile int tmp;

	while(I2C1->SR2 & BUSY){}
	I2C1->CR1 |= START;

	while(!(I2C1->SR1 & SB)){}
	I2C1->DR = saddr << 1;

	while(!(I2C1->SR1 & ADDR)){}
	tmp = I2C1->SR2;

	while(!(I2C1->SR1 & TXE)){}
	I2C1->DR = maddr;

	while(!(I2C1->SR1 & TXE)){}

	I2C1->CR1 |= START;

	while(!(I2C1->SR1 & SB)){}
	I2C1->DR = saddr << 1 | 1;

	while(!(I2C1->SR1 & ADDR)){}
	tmp = I2C1->SR2;

	I2C1->CR1 |= ACK;

	while(n--)
	{
		if(n == 0)
		{
			I2C1->CR1 &= ~ACK;
			I2C1->CR1 |= STOP;
			while (I2C1->CR1 & STOP);   // WAIT FOR HARDWARE CLEAR

			while(!(I2C1->SR1 & RXNE)){}
			*data++ = I2C1->DR;

			break;
		}

		while(!(I2C1->SR1 & RXNE)){}
		*data++ = I2C1->DR;
	}
    while (I2C1->CR1 & I2C_CR1_STOP);
}

void I2C1_burst_write(char saddr, char maddr, char n, uint8_t * data)
{
	volatile int tmp;

	while(I2C1->SR2 & BUSY){}
	I2C1->CR1 |= START;

	while(!(I2C1->SR1 & SB)){}
	I2C1->DR = saddr << 1;

	while(!(I2C1->SR1 & ADDR)){}
	tmp = I2C1->SR2;

	while(!(I2C1->SR1 & TXE)){}
	I2C1->DR = maddr;

	for(int i = 0; i < n; i++)
	{
		while(!(I2C1->SR1 & TXE)){}
		I2C1->DR = *data++;
	}

	while(!(I2C1->SR1 & BTF)){}
	I2C1->CR1 |= STOP;
}
