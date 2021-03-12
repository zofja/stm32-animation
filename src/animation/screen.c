#include <delay.h>
#include <lcd_board_def.h>
#include <stdint.h>

#include "screen.h"
#include "spi/spi_write.h"

/* Needed delay(s)  */

#define Tinit   150
#define T120ms  (MAIN_CLOCK_MHZ * 120000 / 4)

/** Internal functions **/

static void LCDwriteCommand(uint32_t content) {
  data_t data = {0, CONTENT, BYTE, 0, content, 1};
  SPIupdateBuf(data);
}

static void LCDwriteData8(uint32_t content) {
  data_t data = {1, CONTENT, BYTE, 0, content, 1};
  SPIupdateBuf(data);
}

/* In the following functions, content bytes are swapped to ensure sending bytes
 * in correct order. For example, 0x00123456 will become 0x00563412
 * and 0x12345678 will become 0x78563412. */

static void LCDwriteData16(uint32_t content) {
  uint32_t swapped = (content >> 8 | content << 8);
  data_t data = {1, CONTENT, HALF_WORD, 0, swapped, 1};
  SPIupdateBuf(data);
}

static void LCDwriteData24(uint32_t content) {
  uint32_t swapped = (content >> 16) |  // move byte 2 to byte 0
      ((content) & 0xff00) |            // leave byte 1 as is
      (content << 16);                  // byte 0 to byte 2
  data_t data = {1, CONTENT, BYTE, 0, swapped, 3};
  SPIupdateBuf(data);
}

static void LCDwriteData32(uint32_t content) {
  uint32_t swapped = (content >> 24) |  // move byte 3 to byte 0
      ((content << 8) & 0xff0000) |     // move byte 1 to byte 2
      ((content >> 8) & 0xff00) |       // move byte 2 to byte 1
      (content << 24);                  // move byte 0 to byte 3
  data_t data = {1, CONTENT, WORD, 0, swapped, 1};
  SPIupdateBuf(data);
}

static void LCDsetRectangle(uint16_t x1,
                            uint16_t y1,
                            uint16_t x2,
                            uint16_t y2) {
  LCDwriteCommand(0x2A);
  LCDwriteData16(x1);
  LCDwriteData16(x2);

  LCDwriteCommand(0x2B);
  LCDwriteData16(y1);
  LCDwriteData16(y2);

  LCDwriteCommand(0x2C);
}

static void LCDcontrollerConfigure(void) {
  Delay(Tinit);

  /* Sleep out */
  LCDwriteCommand(0x11);

  Delay(T120ms);

  /* Frame rate */
  LCDwriteCommand(0xB1);
  LCDwriteData24(0x053C3C);
  LCDwriteCommand(0xB2);
  LCDwriteData24(0x053C3C);
  LCDwriteCommand(0xB3);
  LCDwriteData24(0x053C3C);
  LCDwriteData24(0x053C3C);

  /* Dot inversion */
  LCDwriteCommand(0xB4);
  LCDwriteData8(0x03);

  /* Power sequence */
  LCDwriteCommand(0xC0);
  LCDwriteData24(0x280804);
  LCDwriteCommand(0xC1);
  LCDwriteData8(0xC0);
  LCDwriteCommand(0xC2);
  LCDwriteData16(0x0D00);
  LCDwriteCommand(0xC3);
  LCDwriteData16(0x8D2A);
  LCDwriteCommand(0xC4);
  LCDwriteData16(0x8DEE);

  /* VCOM */
  LCDwriteCommand(0xC5);
  LCDwriteData8(0x1A);

  /* Memory and color framesBufWrite direction */
  LCDwriteCommand(0x36);
  LCDwriteData8(0xC0);

  /* Color mode 16 bit per pixel */
  LCDwriteCommand(0x3A);
  LCDwriteData8(0x05);

  /* Gamma sequence */
  LCDwriteCommand(0xE0);
  LCDwriteData32(0x0422070A);
  LCDwriteData32(0x2E30252A);
  LCDwriteData32(0x28262E3A);
  LCDwriteData32(0x00010313);
  LCDwriteCommand(0xE1);
  LCDwriteData32(0x0416060D);
  LCDwriteData32(0x2D262327);
  LCDwriteData32(0x27252D3B);
  LCDwriteData32(0x00010413);

  /* Display on */
  LCDwriteCommand(0x29);
}

/** Public interface implementation **/

void LCDconfigure(void) {
  LCDcontrollerConfigure();
  LCDsetRectangle(0, 0, LCD_PIXEL_WIDTH - 1, LCD_PIXEL_HEIGHT - 1);

  SPIwrite();
}

