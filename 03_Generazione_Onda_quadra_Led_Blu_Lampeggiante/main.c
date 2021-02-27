#include <stm32f30x.h>

int main()
{
  //ABILITAZIONE CLOCK LED GPIOE e TIMER (TIM2)
  RCC->AHBENR |= RCC_AHBENR_GPIOEEN;    //Abilitazione GPIOE per il LED
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;   //Abilitazione TIM2 (Timer collegato al BUS APB1)
    
  
  //ABILITAZIONE PORTA LED BLU PE8
  GPIOE->MODER = 1<<16;                 //Abilitazione PE8 per LED BLU
  TIM2->CR1 |= TIM_CR1_CEN;             //Abilitazione il bit CR1_CEN che si trova al bit 0, serve per abilitare il conteggio
    
  /* Calcolo N conteggi
  *  fck = 8MHz (essendo scheda con clock da 8 milioni di conteggi al secondo) fck = frequenza del clock
  *  Tck = 1/fck = 1/8MHz = 125 ns (nano secondi, 10^-9) Periodo del clock
  *  Deltat = 1s
  *  N = Deltat/Tck = 8.000.000 ovvero 1/(125 * 10^-9) N conteggi 
  *  dunque TIM2_ARR = 8000000. 
  *  SI USA IL TIMER2 TIM2 proprio perche la rappresentazione è a 2^(32-1) che è circa 4miliardi di rappresentazioni, con TIM3 e TIM4 non poteva essere possibile poichè la rappresentazione è di 2^(16-1)
  */

  //CALCOLO N CONTEGGI
  TIM2->CNT = 0x00000000;               //Conteggio che parte da 0, (TIM2_CNT è il Timer2 counter, il contatore.)
  TIM2->ARR = 0x007A1200;               /*[Converto 8000000 (decimale) in 007A1200 (esadecimale)]
                                        //e arriva ad 8000000 dopo il Deltat = 1s, TIM2_ARR è il Timer2 Auto-Reload Register, contiene il valore limite a cui deve arrivare il 
                                        //conteggio prima che il contatore si azzeri e riprendi a contare */

  int conteggioraggiunto = 1;           //fisso un FLAG per capire il comportamento del led, ovvero se è acceso o spento
                                        //0 se ACCESO, 1 se SPENTO 
  while(1){
    
    //TIMx_SR (Status Register) mi indica se si sono verificati determinati eventi
    //TIM_SR_UIF mi indica se il CNT è arrivato al valore imposto da ARR
    //Se CNT arriva a 8 milioni che è uguale ad ARR allora si alza il bit di UIF dello SR
    //faccio la & dello SR con SR_UIF, e se è uguale a SR_UIF ed è UGUALE a conteggioraggiunto ALLORA ...
    if((TIM2->SR & TIM_SR_UIF)==TIM_SR_UIF && conteggioraggiunto) 
    {
      GPIOE->ODR = 1<<8;                //Accendi LED8
      TIM2->SR &= ~TIM_SR_UIF;          //Azzera l'UIF
      conteggioraggiunto = 0;           //Fisso 0 al conteggioraggiunto
    }
    
    //faccio la & dello SR con SR_UIF, e se è uguale a SR_UIF ed è DIVERSO a conteggioraggiunto ALLORA ...
    if((TIM2->SR & TIM_SR_UIF)==TIM_SR_UIF && !conteggioraggiunto)
    {
      GPIOE->ODR &= ~(1<<8);            //Spegni LED8
      TIM2->SR &= ~TIM_SR_UIF;          //Azzera l'UIF
      conteggioraggiunto = 1;           //Fisso 1 al conteggioraggiunto
    }
  }
  
  return 0;
}



