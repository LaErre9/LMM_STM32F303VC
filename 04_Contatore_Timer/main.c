#include <stdio.h>
#include <stm32f30x.h>
#include <math.h>

int main()
{
  //ABILITAZIONE CLOCK LED GPIOA e TIMER (TIM2)
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;    //Abilitazione GPIOA per l'user
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;   //Abilitazione TIM2 (Timer collegato al BUS APB1)

  //Variabili utili per il calcolo
  int N;                                //N conteggi
  int conteggio;                        //numero di volte che premo il pulsante user
  float Tck = 125 * pow(10,-9);         //Calcolo Tck, Tck = 125 * 0.000000001
  float deltat = 0;                     //Inizializzo deltat
  
  /* Calcolo Deltat
  *  fck = 8MHz (essendo scheda con clock da 8 milioni di conteggi al secondo) fck = frequenza del clock
  *  Tck = 1/fck = 1/8MHz = 125 ns (nano secondi, 10^-9) Periodo del clock
  *  Deltat = N * Tck
  *  SI USA IL TIMER2 TIM2 proprio perchè la rappresentazione è a 2^(32-1) che è circa 4miliardi di rappresentazioni, con TIM3 e TIM4 non poteva essere possibile poichè la rappresentazione è di 2^(16-1)
  */
  
  while(1){
    
    //se premo il pulsante USER
    if((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0){
    
      TIM2->CR1 |= TIM_CR1_CEN;         //Abilitazione conteggio
      TIM2->CNT = 0;                    //Conteggio che partirà da 0
      
      while((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0); //attendo fino a quando tengo premuto
      
      TIM2->CR1 &= ~TIM_CR1_CEN;
      N = TIM2->CNT;                    //Prendo il valore che è stato contato dal contatore e lo metto in N
      deltat = N * Tck;                 //faccio il calcolo del Deltat
      conteggio++;                      //Conto le volte 
      
      //Stampa dei risultati
      printf("Il conteggio n_%d e' stato di %f secondi \n", conteggio, deltat); 
        
    }
    
  }
  return 0;
}