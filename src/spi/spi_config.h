#ifndef CONFIG_SPI_CONFIG_H_
#define CONFIG_SPI_CONFIG_H_

#include <stdint.h>

void CS(uint32_t bit);
void A0(uint32_t bit);
void SDA(uint32_t bit);
void SCK(uint32_t bit);

void SPIconfigure(void);

#endif // CONFIG_SPI_CONFIG_H_
