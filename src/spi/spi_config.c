#include <stm32.h>
#include <gpio.h>
#include <xcat.h>
#include <irq.h>

#include "spi_config.h"
#include "animation/screen.h"

#define RCC_LCD_CS    xcat3(RCC_AHB1ENR_GPIO, LCD_CS_GPIO_N, EN)
#define RCC_LCD_A0    xcat3(RCC_AHB1ENR_GPIO, LCD_A0_GPIO_N, EN)
#define RCC_LCD_SDA   xcat3(RCC_AHB1ENR_GPIO, LCD_SDA_GPIO_N, EN)
#define RCC_LCD_SCK   xcat3(RCC_AHB1ENR_GPIO, LCD_SCK_GPIO_N, EN)
#define RCC_SPI       xcat(RCC_APB1ENR_SPI3, EN)
#define RCC_SYSCFG    xcat(RCC_APB2ENR_SYSCFG, EN)
#define RCC_DMA       xcat(RCC_AHB1ENR_DMA1, EN)

#define PIN_LCD_CS    (1U << LCD_CS_PIN_N)
#define PIN_LCD_A0    (1U << LCD_A0_PIN_N)
#define PIN_LCD_SDA   (1U << LCD_SDA_PIN_N)
#define PIN_LCD_SCK   (1U << LCD_SCK_PIN_N)

#define GPIO_LCD_CS   xcat(GPIO, LCD_CS_GPIO_N)
#define GPIO_LCD_A0   xcat(GPIO, LCD_A0_GPIO_N)
#define GPIO_LCD_SDA  xcat(GPIO, LCD_SDA_GPIO_N)
#define GPIO_LCD_SCK  xcat(GPIO, LCD_SCK_GPIO_N)

static void RCCconfigure(void) {
  /* Enable GPIO clocks. */
  RCC->AHB1ENR |= RCC_LCD_CS | RCC_LCD_A0 | RCC_LCD_SDA | RCC_LCD_SCK | RCC_DMA;
  /* Enable SPI3 clock. */
  RCC->APB1ENR |= RCC_SPI;
  /* Enable clock for external interrupts. */
  RCC->APB2ENR |= RCC_SYSCFG;
}

static void GPIOconfigure(void) {
  CS(1); // chip select inactive by default
  GPIOoutConfigure(GPIO_LCD_CS,
                   LCD_CS_PIN_N,
                   GPIO_OType_PP,
                   GPIO_High_Speed,
                   GPIO_PuPd_NOPULL);

  A0(1); // send data by default
  GPIOoutConfigure(GPIO_LCD_A0,
                   LCD_A0_PIN_N,
                   GPIO_OType_PP,
                   GPIO_High_Speed,
                   GPIO_PuPd_NOPULL);

  SDA(0);
  GPIOafConfigure(GPIO_LCD_SDA,
                  LCD_SDA_PIN_N,
                  GPIO_OType_PP,
                  GPIO_High_Speed,
                  GPIO_PuPd_NOPULL,
                  GPIO_AF_SPI3);

  SCK(0); // write data bit on rising clock edge
  GPIOafConfigure(GPIO_LCD_SCK,
                  LCD_SCK_PIN_N,
                  GPIO_OType_PP,
                  GPIO_High_Speed,
                  GPIO_PuPd_NOPULL,
                  GPIO_AF_SPI3);
}

void CS(uint32_t bit) {
  if (bit) {
    /* Activate chip select line. */
    GPIO_LCD_CS->BSRR = PIN_LCD_CS;
  } else {
    /* Deactivate chip select line. */
    GPIO_LCD_CS->BSRR = PIN_LCD_CS << 16;
  }
}

void A0(uint32_t bit) {
  if (bit) {
    /* Set data/command line to data. */
    GPIO_LCD_A0->BSRR = PIN_LCD_A0;
  } else {
    /* Set data/command line to command. */
    GPIO_LCD_A0->BSRR = PIN_LCD_A0 << 16;
  }
}

void SDA(uint32_t bit) {
  if (bit) {
    /* Set data bit one. */
    GPIO_LCD_SDA->BSRR = PIN_LCD_SDA;
  } else {
    /* Set data bit zero. */
    GPIO_LCD_SDA->BSRR = PIN_LCD_SDA << 16;
  }
}

void SCK(uint32_t bit) {
  if (bit) {
    /* Rising clock edge. */
    GPIO_LCD_SCK->BSRR = PIN_LCD_SCK;
  } else {
    /* Falling clock edge. */
    GPIO_LCD_SCK->BSRR = PIN_LCD_SCK << 16;
  }
}

void SPIconfigure(void) {
  RCCconfigure();
  GPIOconfigure();

  SPI3->CR1 = SPI_CR1_MSTR |              // master mode
      SPI_CR1_SSM | SPI_CR1_SSI |         // software management of CS line
      SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;  // transmit only mode

  SPI3->CR2 = SPI_CR2_TXDMAEN;            // enable SPI transmission using DMA

  IRQsetPriority(DMA1_Stream5_IRQn, MIDDLE_IRQ_PRIO, VERY_HIGH_IRQ_SUBPRIO);

  NVIC_EnableIRQ(DMA1_Stream5_IRQn);      // enable external interrupts

  SPI3->CR1 |= SPI_CR1_SPE;               // enable SPI
}
