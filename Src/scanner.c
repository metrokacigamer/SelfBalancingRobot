#define STM32F411xE

#include "stm32f4xx.h"
#include "i2c.h"
#include "uart.h"
#include "scanner.h"

void I2C_device_check(uint8_t sad)
{
	uart2_tx_init();

	I2C1_init();
    volatile int tmp;

    // wait until bus free
    while (I2C1->SR2 & BUSY);

    // START condition
    I2C1->CR1 |= START;

    // wait for SB
    while (!(I2C1->SR1 & SB));

    // send address (write mode)
    I2C1->DR = (sad << 1);

    // wait for either ADDR or AF
    uint32_t timeout = 10000;

    while (!(I2C1->SR1 & (ADDR | (1 << 10))) && --timeout);

    if (timeout == 0)
    {
        // stuck → reset I2C
        I2C1->CR1 |= STOP;
        printf("stuck when looking for device at 0x%02X\r\n", sad);
        return;
    }

    // ACK received
    if (I2C1->SR1 & ADDR)
    {
        tmp = I2C1->SR2;   // clear ADDR

        printf("Device found at 0x%02X\r\n", sad);

        I2C1->CR1 |= STOP;
    }

    // AF = no device
    else if (I2C1->SR1 & (1 << 10))
    {
        I2C1->SR1 &= ~(1 << 10); // clear AF
        I2C1->CR1 |= STOP;

        printf("Device not found at 0x%02X\r\n", sad);
    }
}

void I2C_scan(void)
{
	I2C1_init();

	uart2_tx_init();

    volatile int tmp;
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        // wait until bus free
        while (I2C1->SR2 & BUSY);

        // START condition
        I2C1->CR1 |= START;

        // wait for SB
        while (!(I2C1->SR1 & SB));

        // send address (write mode)
        I2C1->DR = (addr << 1);

        // wait for either ADDR or AF
        uint32_t timeout = 10000;

        while (!(I2C1->SR1 & (ADDR | (1 << 10))) && --timeout);

        if (timeout == 0)
        {
            // stuck → reset I2C
            I2C1->CR1 |= STOP;
            continue;
        }

        // ACK received
        if (I2C1->SR1 & ADDR)
        {
            tmp = I2C1->SR2;   // clear ADDR

            printf("Device found at 0x%02X\r\n", addr);

            I2C1->CR1 |= STOP;
        }

        // AF = no device
        else if (I2C1->SR1 & (1 << 10))
        {
            I2C1->SR1 &= ~(1 << 10); // clear AF
            I2C1->CR1 |= STOP;
        }
    }
}
