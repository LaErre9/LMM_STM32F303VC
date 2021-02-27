#include <stm32f30x.h>

int main()
{
  //ABILITAZIONE DEL RCC
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //Abilita PA
  RCC->AHBENR |= RCC_AHBENR_GPIOEEN; //Abilita PE
 
  //ABILITAZIONE GPIOA ed GPIOE
  GPIOE->MODER = 1<<16;
  GPIOA->MODER &= ~GPIO_MODER_MODER0;
  
  while(1){
  
    if((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0)
    {
      //Accendi LED BLU
      GPIOE->ODR = 1<<8;
    }
    else
    {
      //Spegni LED BLU
      GPIOE->ODR &= ~(1<<8);
    }
  
  }
  return 0;
}
