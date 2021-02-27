#include <stm32f30x.h>

/* CONTATORE BINARIO A 8 BIT */

int main()
{
  //ATTIVAZIONE RCC PER ABILITAZIONE GPIOA (USER) e GPIOE (LED)
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN; 
  RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    
  
  
  //ABILITAZIONE MODER LED e USER
  GPIOE->MODER = 0x55550000; //Abilitazione GPIOE_MODER e Abilitazione di tutti i led da PE8 a PE15 
  GPIOA->MODER &= ~GPIO_MODER_MODER0; //Abilitazione GPIOA_MODER e Abilitazione di PA0 come input
  //setto il moder0 comr input e dunque devo azzerare tutti i bit (input digitale 00)
  
  /*
  CALCOLO IN BINARIO >> 
  00000000000000000000000000000000 + 00000000000000000000000100000000 =  00000000000000000000000100000000 e così via ...
  CALCOLO IN ESADECIMALE >> 
  00000000 + 00000100 = 00000100 e così via ...
  */
  
  int cont = 0x00000000;
  while(1){
       if((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0){ //SE USER Viene cliccato, FAI
        //solo dopo aver rilasciato, conti
         while((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0); 
         //conta
         cont = cont + 0x00000100; 
         GPIOE->ODR = cont; //Accedi al registro di GPIOE ODR e ACCENDI I LED indicati da CONT
       }
  }

  /*
  int cont = 0;
  
  while(1){
      if((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0){
        while((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0);
          cont++;
          GPIOE->ODR = (cont<<8);    
      }
  }  
  */
  
  return 0;
}
