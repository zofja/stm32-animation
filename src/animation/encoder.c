#include <stm32.h>
#include <irq.h>

#include "encoder.h"
#include "frames.h"
#include "screen.h"
#include "timer/tim_config.h"
#include "spi/spi_write.h"

#define FRAME_BYTES_N 2 * LCD_SIZE

/* Internal functions. */

static int EXTIisPending(uint32_t pin) {
  return EXTI->IMR & EXTI->PR & (1 << pin);
}

static void EXTIresetPending(uint32_t pin) {
  EXTI->PR = 1 << pin;;
}

void intToString(uint32_t n, char *buf) {
  int digits = 6;
  for (int i = digits - 1; i >= 0; --i, n /= 10) {
    buf[i] = (n % 10) + '0';
  }
}

static void handleEncoderInterrupt(uint32_t currentCount) {
  static uint32_t previousCount = 0;
  uint32_t diff = 0;
  int right = 0;

  /* Calculate difference between current count and previous count,
   * take into consideration overflow. */
  if (currentCount > previousCount) {
    diff = currentCount - previousCount;
    right = 1;
    if (diff > FULL_TURN_IMPULSES / 2) right = 0; // overflow
  } else {
    diff = previousCount - currentCount;
    right = 0;
    if (diff > FULL_TURN_IMPULSES / 2) right = 1; // overflow
  }

  if (diff >= SINGLE_TURN_IMPULSES
      && diff <= FULL_TURN_IMPULSES - SINGLE_TURN_IMPULSES) {
    if (right) {
      updateFramePos(1);
      previousCount =
          (previousCount + SINGLE_TURN_IMPULSES) % FULL_TURN_IMPULSES;
    } else {
      updateFramePos(-1);
      previousCount =
          (previousCount - SINGLE_TURN_IMPULSES + FULL_TURN_IMPULSES)
              % FULL_TURN_IMPULSES;
    }
    writeCurrentFrame();
  }
}

/* IRQ handlers. */

void EXTI4_IRQHandler(void) {
  if (EXTIisPending(TIM_TI1_PIN_N)) {
    EXTIresetPending(TIM_TI1_PIN_N);
    uint32_t currentCount = TIM3->CNT;
    handleEncoderInterrupt(currentCount);
  }
}

void EXTI9_5_IRQHandler(void) {
  if (EXTIisPending(TIM_TI2_PIN_N)) {
    EXTIresetPending(TIM_TI2_PIN_N);
    uint32_t currentCount = TIM3->CNT;
    handleEncoderInterrupt(currentCount);
  }
}

/* Public interface implementation. */

void writeCurrentFrame() {
  irq_level_t level = IRQprotect(MIDDLE_IRQ_PRIO);
  data_t data = {1, ADDRESS, HALF_WORD, getFrameAddr(), 0, FRAME_BYTES_N};
  SPIupdateBuf(data);
  SPIwrite();
  IRQunprotect(level);
}
