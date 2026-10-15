// STM32F401RE_TIM.c
// TIM functions

#include "STM32L432KC_TIM.h"
#include "STM32L432KC_RCC.h"

void initTIM(TIM_TypeDef * TIMx){
  // Set prescaler to give 1 ms time base THIS IS BROKEN b/c will go over prescaler max!!!!
  //uint32_t psc_div = (uint32_t) ((SystemCoreClock/1e3));

  // Set prescaler division factor
  //TIMx->PSC = (psc_div - 1);
  TIMx->PSC = (SystemCoreClock / 10000) - 1;
  // Generate an update event to update prescaler value
  TIMx->EGR |= 1;
  // Enable counter
  TIMx->CR1 |= 1; // Set CEN = 1
}

void delay_millis(TIM_TypeDef * TIMx, uint32_t ms){
  //TIMx->ARR = ms;// Set timer max count
  TIMx->ARR = ms * 10 - 1;   // ms * 10 ticks b/c of change to fix prescaler
  TIMx->EGR |= 1;     // Force update
  TIMx->SR &= ~(0x1); // Clear UIF
  TIMx->CNT = 0;      // Reset count

  while(!(TIMx->SR & 1)); // Wait for UIF to go high
}