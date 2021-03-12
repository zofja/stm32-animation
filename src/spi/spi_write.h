#ifndef SPI_SPI_WRITE_H_
#define SPI_SPI_WRITE_H_

typedef enum mode {
  ADDRESS, CONTENT
} mode_t;

typedef enum width {
  BYTE, HALF_WORD, WORD
} width_t;

typedef struct data {
  uint8_t A0;
  mode_t mode;
  width_t width;
  const uint16_t *addr;
  uint32_t content;
  uint32_t len;
} data_t;

void SPIupdateBuf(data_t d);

void SPIwrite(void);

#endif //SPI_SPI_WRITE_H_
