#include <stm32.h>

#include "spi_write.h"
#include "spi_config.h"

#define DMA_SPI     DMA1_Stream5
#define BUF_LEN     128

/* Cyclic SPI buffer. */
static data_t SPIbuf[BUF_LEN] = {0};

/* SPI buffer write posistion. */
static int write = 0;

/* SPI buffer read position. */
static int read = 0;

/* Internal functions. */

static int8_t SPIbufNotEmpty(void) {
  return read != write;
}

static void SPIupdateRead(void) {
  read = (read + 1) % BUF_LEN;
}

static const data_t *SPIgetBuf() {
  return &SPIbuf[read];
}

static int SPIcanWrite(void) {
  return (DMA_SPI->CR & DMA_SxCR_EN) == 0 && (DMA1->HISR & DMA_HISR_TCIF5) == 0;
}

static void DMAconfigure(void) {
  const data_t *data = SPIgetBuf();
  uint32_t ptr = 0;

  /* Set data pointer depending on the mode value. */
  if (data->mode == ADDRESS) {
    ptr = (uint32_t) data->addr;
  } else {
    ptr = (uint32_t) &data->content;
  }

  DMA1->HIFCR = DMA_HIFCR_CTCIF5;         // set stream 5 transfer complete
  DMA_SPI->PAR = (uint32_t) &SPI3->DR;    // send data to SPI
  DMA_SPI->M0AR = ptr;                    // set data address
  DMA_SPI->NDTR = data->len;              // set data length

  uint32_t cr = DMA_SxCR_PL_1 |   // high priority
      DMA_SxCR_DIR_0 |            // memory to peripheral
      DMA_SxCR_TCIE |             // transfer complete interrupt enable
      ((data->len > 1) ? DMA_SxCR_MINC : 0);  // memory increment if len > 0

  uint32_t fcr = 0;

  /* Wider than 8 bits memory read requires FIFO to be enabled
  (cannot unpack without it). */
  if (data->width == HALF_WORD) {
    cr |= DMA_SxCR_MSIZE_0;   // read half words
    fcr |= DMA_SxFCR_DMDIS;   // enable FIFO (disable direct mode)
    if (data->len >= 16) {
      cr |= DMA_SxCR_MBURST_1;  // INCR8 (8 x 2 bytes)
      fcr |= DMA_SxFCR_FTH_0 | DMA_SxFCR_FTH_1; // full
    }
  } else if (data->width == WORD) {
    cr |= DMA_SxCR_MSIZE_1;  // read words
    fcr |= DMA_SxFCR_DMDIS;  // enable FIFO (disable direct mode)
    if (data->len >= 16) {
      cr |= DMA_SxCR_MBURST_0;  // INCR4 (4 x 4 bytes)
      fcr |= DMA_SxFCR_FTH_0 | DMA_SxFCR_FTH_1; // full
    }
  }

  A0(data->A0);     // set data sending mode
  CS(0);        // activate chip select
  DMA_SPI->CR = cr;
  DMA_SPI->FCR = fcr;
}

/* Public interface implementations. */

void SPIupdateBuf(data_t d) {
  SPIbuf[write] = d;
  write = (write + 1) % BUF_LEN;
}

void SPIwrite() {
  if (SPIcanWrite()) {
    DMAconfigure();
    SPIupdateRead();
    DMA_SPI->CR |= DMA_SxCR_EN; // enable DMA
  }
}

/* IRQ handlers. */

void DMA1_Stream5_IRQHandler(void) {
  uint32_t isr = DMA1->HISR;
  if (isr & DMA_HISR_TCIF5) {
    /* Wait for SPI transfer to complete. */
    while (!(SPI3->SR & SPI_SR_TXE))
        __NOP();
    while (SPI3->SR & SPI_SR_BSY)
        __NOP();

    /* Set stream 5 transfer complete. */
    DMA1->HIFCR = DMA_HIFCR_CTCIF5;

    /* If buffer is not empty, initialize next transfer. */
    if (SPIbufNotEmpty()) {
      SPIwrite();
    } else {
      CS(1); // deactivate chip select
    }
  }
}