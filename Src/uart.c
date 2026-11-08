#include "uart.h"


#define GPIOAEN 			(1U<<0)
#define UART2EN 			(1U<<17)
#define DMA1EN 				(1U << 21)

#define CR1_TE				(1U<<3)
#define CR1_RE				(1U<<2)
#define CR1_UE				(1U<<13)

#define SYS_FREQ			16000000
#define APB1_CLK			SYS_FREQ
#define UART_BAUDRATE		115200

#define SR_TXE				(1U<<7)
#define SR_RXNE				(1U<<5)
#define RXNEIE 				(1U << 5)

#define DIR 				(0x6)
#define MINC				(1U << 10)
#define CINC 				(1U << 8)
#define TCIE				(1U << 4)
#define DMDIS				(1U << 2)
#define FEIE				(1U << 7)
#define DMA1_STR6_EN		(1U << 0)
#define DMAT				(1U << 7)
uint32_t usart_data = 0;
static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t periphClk, uint32_t baudRate);
static uint16_t compute_uart_div(uint32_t periphClk, uint32_t baudRate);

void uart2_write(int ch)
{
	while(!(USART2->SR & SR_TXE)){}

	USART2->DR = ch & 0xFF;
}

void uart2_rxtx_int_init(void)
{
	RCC->AHB1ENR |= GPIOAEN;

	//pin2-alternate function mode - for usart
	GPIOA->MODER &= ~(1U<<4);
	GPIOA->MODER |= (1U<<5);

	//pin3-alternate function mode - for usart rx

	GPIOA->MODER &= ~(1U<<6);
	GPIOA->MODER |= (1U<<7);

	//AF type - AF7 - usart chosen for pin 2
	GPIOA->AFR[0] |= (1U<<10) | (1U<<9) | (1U<<8);//11-8 0111;
	GPIOA->AFR[0] &= ~(1U<<11);

	//AF type - AF7 - usart chosen for pin 3
	GPIOA->AFR[0] |= (1U<<14) | (1U<<13) | (1U<<12);//11-8 0111;
	GPIOA->AFR[0] &= ~(1U<<15);

	//en uart2
	RCC->APB1ENR |= UART2EN;

	uart_set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	USART2->CR1 = CR1_TE;
	USART2->CR1 |= CR1_RE;
	USART2->CR1 |= CR1_UE;
	USART2->CR1 |= RXNEIE;

	NVIC_EnableIRQ(USART2_IRQn);
}

void uart2_rxtx_init(void)
{
	RCC->AHB1ENR |= GPIOAEN;

	//pin2-alternate function mode - for usart
	GPIOA->MODER &= ~(1U<<4);
	GPIOA->MODER |= (1U<<5);

	//pin3-alternate function mode - for usart rx

	GPIOA->MODER &= ~(1U<<6);
	GPIOA->MODER |= (1U<<7);

	//AF type - AF7 - usart chosen for pin 2
	GPIOA->AFR[0] |= (1U<<10) | (1U<<9) | (1U<<8);//11-8 0111;
	GPIOA->AFR[0] &= ~(1U<<11);

	//AF type - AF7 - usart chosen for pin 3
	GPIOA->AFR[0] |= (1U<<14) | (1U<<13) | (1U<<12);//11-8 0111;
	GPIOA->AFR[0] &= ~(1U<<15);

	//en uart2
	RCC->APB1ENR |= UART2EN;

	uart_set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	USART2->CR1 = CR1_TE;
	USART2->CR1 |= CR1_RE;
	USART2->CR1 |= CR1_UE;
}


char uart2_read()
{
	while(!(USART2->SR & SR_RXNE)){}
	return USART2->DR;
}

void uart2_tx_init(void)
{
	RCC->AHB1ENR |= GPIOAEN;

	//pin2-alternate function mode - for usart
	GPIOA->MODER &= ~(1U<<4);
	GPIOA->MODER |= (1U<<5);

	//AF type - AF7 - usart chosen for pin 2
	GPIOA->AFR[0] |= (1U<<10) | (1U<<9) | (1U<<8);//11-8 0111;
	GPIOA->AFR[0] &= ~(1U<<11);

	//en uart2
	RCC->APB1ENR |= UART2EN;

	uart_set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	USART2->CR1 = CR1_TE;
	USART2->CR1 |= CR1_UE;
}

void dma1_stream6_init(uint32_t src, uint32_t dst, uint32_t len)
{
	RCC->AHB1ENR |= DMA1EN;//bit 21
	//DMA1 str6 ch4

	DMA1_Stream6->CR &= ~(1U << 0);//disable

	DMA1->HIFCR |= (61U << 16);

	DMA1_Stream6->PAR = dst;
	DMA1_Stream6->M0AR = src;
	DMA1_Stream6->NDTR = len;

	DMA1_Stream6->CR &= ~(7U << 25);
	DMA1_Stream6->CR |= (4U << 25);//channel 4

	//DBM, MINC,PINC,DIR(MtoP),MSize,PSize
	DMA1_Stream6->CR |= MINC | CINC;//10,8

	DMA1_Stream6->CR &= ~(3U << DIR);//6
	DMA1_Stream6->CR |= (1U << DIR);//6
//
//	DMA1_Stream6->CR &= ~(3U << MSIZE);//13
////	DMA1_Stream6->CR |= (1U << MSIZE); //16bits
//
//	DMA1_Stream6->CR &= ~(3U << PSIZE);//11
////	DMA1_Stream6->CR |= (1U << PSIZE);//16 bits

	DMA1_Stream6->CR |= TCIE;//bit 4
	//Direct mode
	DMA1_Stream6->FCR &= ~DMDIS;//DMDIS 0 bit 2, enable direct mode
	DMA1_Stream6->FCR &= ~FEIE;//FEIE 0 bit 7, enable direct mode


	DMA1_Stream6->CR |= DMA1_STR6_EN;//enable

	//UART2 tx DMA enable
	USART2->CR3 |= DMAT;// bit 7

	//DMA interrupt
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}


static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t periphClk, uint32_t baudRate)
{
	USARTx->BRR = compute_uart_div(periphClk, baudRate);
}


static uint16_t compute_uart_div(uint32_t periphClk, uint32_t baudRate)
{
	return  ((periphClk + (baudRate/2U))/baudRate);
}
