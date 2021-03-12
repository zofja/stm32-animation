#include <irq.h>
#include "animation/screen.h"
#include "timer/tim_config.h"
#include "spi/spi_config.h"
#include "animation/encoder.h"

int main(void) {
  IRQprotectionConfigure();

  SPIconfigure();
  LCDconfigure();
  TIMconfigure();

  writeCurrentFrame();
}