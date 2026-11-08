#include "nrf24l01.h"
#include "spi.h"
#include "systick.h"

#define CONFIG_ADDR					(0x0)
#define SETUP_AW					(0x3)
#define RX_ADDR_P0 					(0xA)
#define RX_ADDR_P1 					(0xB)
#define TX_ADDR						(0x10)
#define RX_PW_P1					(0x12)
#define RX_PW_P0					(0x11)
#define FIFO_STATUS_ADDR			(0x17)
#define RF_CH						(0x05)
#define STATUS						(0x07)
#define EN_AA						(0x01)
#define EN_RXADDR					(0x02)

#define R_RX_PL_WID					(0x60)
#define R_RX_PAYLOAD				(0x61)
#define W_TX_PAYLOAD				(0xA0)
#define FLUSH_TX					(0xE1)
#define FLUSH_RX 					(0xE2)
#define NOP 						(0xFF)

#define TX_EMPTY					(1U << 4)
#define RX_DR						(1U << 6)
#define TX_DS 						(1U << 5)
#define MAX_RT						(1U << 4)

#define CE_PIN 						(1U << 9)
#define CE_RX_PIN					(1U << 12)

#define WR_COMMAND					(0x20)

uint8_t sr;
volatile uint8_t packet_failed = 0;


void CE_rx_low(void)
{
	GPIOA->ODR &= ~CE_RX_PIN;
}

void CE_rx_high(void)
{
	GPIOA->ODR |= CE_RX_PIN;
}

void CE_low(void)
{
	GPIOA->ODR &= ~CE_PIN;
}

void CE_high(void)
{
	GPIOA->ODR |= CE_PIN;
}

// SETUP_AW for ID length: 5 bytes, abcde, edcba;
// set own ID: TX_ADDR, RX_ADDR_P0. If receiver, set ID of sender: RX_ADDR_P0-5. 2-5 repeat first 4 bytes from 1
//	R_RX_PAYLOAD 0110 0001
//	W_TX_PAYLOAD 1010 0000, only command, no register address.
// 10us pulse on CE means transmit. CE = 1, means transmit all.
// for RX, CE = 1;
// CE on 0 : standby mode
// config->PRIM_RX: 0-TX, 1-RX
void NRF_tx_init(char* addr, uint8_t n, uint8_t message_length)
{
	SPI1_init();
	CE_low();
	uint8_t data_wr, data_r;
	//TX mode + PWR up mode

	systickDelayMs(5);
	data_wr = 0x3;
	//ID length - 5 bytes
	SPI1_write_bytes(WR_COMMAND | SETUP_AW, &data_wr, &sr, 1);

	//P0 auto-ack
	data_wr = 0x01;
	SPI1_write_bytes(WR_COMMAND | EN_AA, &data_wr, &sr, 1);

	//message length
	SPI1_write_bytes(WR_COMMAND | RX_PW_P0, &message_length, &sr, 1);

	//set ID
	SPI1_write_bytes(WR_COMMAND | RX_ADDR_P0, (uint8_t*)addr, &sr, 5);
	SPI1_write_bytes(WR_COMMAND | TX_ADDR, (uint8_t*)addr, &sr, 5);

	//frequency for good measure
	data_wr = 0x2;
	SPI1_write_bytes(WR_COMMAND | RF_CH, &data_wr, &sr, 1);

	SPI1_read_bytes(CONFIG_ADDR, &data_r, &sr, 1);

	data_wr = data_r | 0x2;
	SPI1_write_bytes(WR_COMMAND | CONFIG_ADDR, &data_wr, &sr, 1);
}

void NRF_rx_init(char* addr, uint8_t n, uint8_t message_length)//PA 12: CE, PA 11: NSS
{
	SPI1_rx_init();
	uint8_t data_wr, data_r;
	//TX mode + PWR up mode


	systickDelayMs(5);
	data_wr = 0x3;

	//5byte ID
	SPI1_rx_write_bytes(WR_COMMAND | SETUP_AW, &data_wr, &sr, 1);

	//enable auto-ack for P0
	data_wr = 0x01;
	SPI1_rx_write_bytes(WR_COMMAND | EN_AA, &data_wr, &sr, 1);

	//P0 enabled
	data_wr = 0x01;
	SPI1_rx_write_bytes(WR_COMMAND | EN_RXADDR, &data_wr, &sr, 1);

	//message(to be received) length
	SPI1_rx_write_bytes(WR_COMMAND | RX_PW_P0, &message_length, &sr, 1);

	//set ID
	SPI1_rx_write_bytes(WR_COMMAND | RX_ADDR_P0, (uint8_t*)addr, &sr, 5);

	//frequency for good measure
	data_wr = 0x2;
	SPI1_rx_write_bytes(WR_COMMAND | RF_CH, &data_wr, &sr, 1);

	SPI1_rx_read_bytes(CONFIG_ADDR, &data_r, &sr, 1);

	data_wr = data_r | 0x2 | 0x1;
	SPI1_rx_write_bytes(WR_COMMAND | CONFIG_ADDR, &data_wr, &sr, 1);

	SPI1_rx_command(FLUSH_RX);
	CE_rx_high();
	systickDelayUs(200);
}

void NRF_rx_receive(uint8_t* data, uint8_t n)
{
	uint8_t data_io;
	do
	{
		SPI1_rx_read_bytes(NOP, &data_io, &sr, 1);
	}
	while(!(sr & RX_DR));
	//read data
	SPI1_rx_read_bytes(R_RX_PAYLOAD, data, &sr, n);

	data_io = RX_DR;
	SPI1_rx_write_bytes(WR_COMMAND | STATUS, &data_io, &sr, 1);

	SPI1_rx_command(FLUSH_RX);
}

//ignore
void NRF_receive(uint8_t* data, uint8_t* n)
{
	uint8_t dummy;
	do
	{
		SPI1_read_bytes(CONFIG_ADDR, &dummy, &sr, 1);
	}
	while(!(sr & RX_DR));

	CE_high();

	//read data
	SPI1_read_bytes(R_RX_PAYLOAD, data, &sr, *n);

	CE_low();
}


volatile int a,b,c,d,e = 0;
void NRF_transmit(uint8_t* data, uint8_t n)
{
//make sure TX FIFO empty
//set
	uint8_t dummy = 0x00, data_read;

	//write data
	SPI1_write_bytes(W_TX_PAYLOAD, data, &sr, n);

	CE_high();
	systickDelayUs(15);

	CE_low();
	volatile int a_i = 0;
	while(1)
	{
		a_i++;
		SPI1_read_bytes(STATUS, &data_read, &sr, 1);
		if(sr & TX_DS || sr & MAX_RT)
		{
			if(a_i > 100)
			{
				if(a_i > 1000000)
				{
					packet_failed = 1;
					return;
//					if(a_1 > 1000000)
//					{
//						d++;//more than 1000000 attempts
//					}
//					else
//					{
//						c++;//10000-1000000 attempts
//					}
				}
				else
				{
					b++;//100-10000
				}
			}
			else
			{
				a++;//less than 100
			}
			if(sr & TX_DS)
				break;

			if(sr & MAX_RT)
			{
			    uint8_t cmd;

			    // clear MAX_RT
			    cmd = MAX_RT;
			    SPI1_write_bytes(WR_COMMAND | STATUS, &cmd, &sr, 1);

			    // discard failed packet
			    cmd = 0;
			    SPI1_write_bytes(FLUSH_TX, &cmd, &sr, 1);
			    packet_failed = 1;
			    return;
			}
		}
	}

	packet_failed = 0;
	dummy = TX_DS;
	SPI1_write_bytes(WR_COMMAND | STATUS, &dummy, &sr, 1);
}
