#ifndef NRF24L01_H_
#define NRF24L01_H_
#include <stdint.h>

void NRF_tx_init(char* addr, uint8_t n, uint8_t message_length);
void NRF_rx_init(char* addr, uint8_t n, uint8_t message_length);

void NRF_rx_receive(uint8_t* data, uint8_t n);
void NRF_receive(uint8_t* data, uint8_t* n);
void NRF_transmit(uint8_t* data, uint8_t n);

extern volatile uint8_t packet_failed;

#endif
