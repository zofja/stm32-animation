#include <stm32.h>
#include <gpio.h>
#include <irq.h>
#include <xcat.h>

#include "tim_config.h"

#define TIM_T1_GPIO_N B
#define TIM_T2_GPIO_N B

#define GPIO_TIM_T1 xcat(GPIO, TIM_T1_GPIO_N)
#define GPIO_TIM_T2 xcat(GPIO, TIM_T2_GPIO_N)

#define RCC_TIM     xcat(RCC_APB1ENR_TIM3, EN)
#define RCC_TIM_T1  xcat3(RCC_AHB1ENR_GPIO, TIM_T1_GPIO_N, EN)
#define RCC_TIM_T2  xcat3(RCC_AHB1ENR_GPIO, TIM_T2_GPIO_N, EN)
#define RCC_SYSCFG  xcat(RCC_APB2ENR_SYSCFG, EN)

static void RCCconfigure(void) {
  /* Enable TIM3 clock. */
  RCC->APB1ENR |= RCC_TIM;
  /* Enable GPIO clocks. */
  RCC->AHB1ENR |= RCC_TIM_T1 | RCC_TIM_T2;
  /* Enable clock for external interrupts. */
  RCC->APB2ENR |= RCC_SYSCFG;
}

static void GPIOconfigure(void) {
  GPIOinConfigure(GPIO_TIM_T1, TIM_TI1_PIN_N, GPIO_PuPd_NOPULL,
                  EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);

  GPIOinConfigure(GPIO_TIM_T2, TIM_TI2_PIN_N, GPIO_PuPd_NOPULL,
                  EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);

  GPIOafConfigure(GPIO_TIM_T1, TIM_TI1_PIN_N, GPIO_OType_PP,
                  GPIO_Low_Speed, GPIO_PuPd_UP,
                  GPIO_AF_TIM3);

  GPIOafConfigure(GPIO_TIM_T2, TIM_TI2_PIN_N, GPIO_OType_PP,
                  GPIO_Low_Speed, GPIO_PuPd_UP,
                  GPIO_AF_TIM3);
}

void TIMconfigure(void) {
  RCCconfigure();
  GPIOconfigure();

  /* Configure update request source. */
  TIM3->CR1 = TIM_CR1_URS;

  /* Encoder mode 3 – Counter counts up/down on both TI1FP1 and TI2FP2 edges
   * depending on the level of the other input.*/
  TIM3->SMCR = TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0;

  /* Set TIM3_CH1 as input, IC1 is mapped on TI1. */
  TIM3->CCMR1 = TIM_CCMR1_CC1S_0;
  /* Set TIM3_CH2 as input, IC2 is mapped on TI2. */
  TIM3->CCMR1 |= TIM_CCMR1_CC2S_0;
  /* Enable digital filter for TI1 and TI2 signals to remove accidental counts. */
  TIM3->CCMR1 |= TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1 |
      TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1;

  /* Right turn increments counter, left turn decrements it. */
  TIM3->CCER = TIM_CCER_CC1P | TIM_CCER_CC2P;

  /* Disable prescaler. */
  TIM3->PSC = 0;

  /* Overflow upon reaching full turn impulse count. */
  TIM3->ARR = FULL_TURN_IMPULSES - 1;
  TIM3->EGR = TIM_EGR_UG;

  /* Reset counter. */
  TIM3->CNT = 0;

  /* Clear external interrupts pending register. */
  EXTI->PR = (1 << TIM_TI1_PIN_N) | (1 << TIM_TI2_PIN_N);

  /* Set interrupt priorities. */
  IRQsetPriority(EXTI4_IRQn, LOW_IRQ_PRIO, HIGH_IRQ_SUBPRIO);
  IRQsetPriority(EXTI9_5_IRQn, LOW_IRQ_PRIO, HIGH_IRQ_SUBPRIO);

  /* Enable external interrupts. */
  NVIC_EnableIRQ(EXTI4_IRQn);   // PB4
  NVIC_EnableIRQ(EXTI9_5_IRQn); // PB5

  /* Enable timer. */
  TIM3->CR1 |= TIM_CR1_CEN;
}