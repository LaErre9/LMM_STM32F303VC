#include <stm32f30x.h>
#include <stdbool.h>

bool acceso = false;

int main()
{
  //ABILITAZIONE TIMER2
  //ABILITAZIONE CLOCK LED GPIOE e TIMER (TIM2)
  RCC->AHBENR |= RCC_AHBENR_GPIOEEN;    //Abilitazione GPIOE per il LED
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;   //Abilitazione TIM2 (Timer collegato al BUS APB1) (Timer collegato al BUS APB1)
  
  //ABILITAZIONE PORTA LED BLU PE8
  GPIOE->MODER |= 0x00050000;

  //ABILITAZIONE INTERRUPT DEL TIM2
  TIM2->DIER |= TIM_DIER_UIE;
  NVIC->ISER[0] |= (1<<28); //abilitare la 28esima posizione della tabella di NVIC
  
  TIM2->ARR = 36000000; //MEZZO SECONDO
  TIM2->CNT = 0;

    
    /* Calcolo N conteggi
  *  fck = 8MHz (essendo scheda con clock da 8 milioni di conteggi al secondo) fck = frequenza del clock
  *  Tck = 1/fck = 1/72MHz 
  *  Deltat = 0.5s
  *  N = Deltat/Tck = 36.000.000
  *  dunque TIM2_ARR = 36000000. 
  */
    
  TIM2->CR1 |= TIM_CR1_CEN;             //Abilitazione il bit CR1_CEN che si trova al bit 0, serve per abilitare il conteggio
  while(1)
  {
    GPIOE->ODR |= (1<<8);
    for(int i = 0; i < 20000000; i++);
    GPIOE->ODR &= ~(1<<8);
    for(int i = 0; i < 20000000; i++);
  }
  return 0;
}

void TIM2_IRQHandler ()
{
  TIM2->SR &= ~TIM_SR_UIF;
  if(!acceso)
  {
    GPIOE->ODR  |= (1<<9);
    acceso = true;
  }
  else
  {
    GPIOE->ODR &= ~(1<<9);
    acceso = false;
  }
}

