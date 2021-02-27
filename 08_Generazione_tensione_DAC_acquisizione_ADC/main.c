#include <stdio.h>
#include <stm32f30x.h>

/*
ATTENZIONE
Collegamento PONTE tra il PIN PA4 e il PA2 sulla SCHEDA
*/

int codice_DAC_ADC = 100;
int codice_ADC_DAC;

float tensione_DAC;
float tensione_ADC;

int main()
{
  //ABILITAZIONE GPIOA - ADC12 - DAC1
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
  RCC->AHBENR |= RCC_AHBENR_ADC12EN;
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;  
  
  //ABILITO PA4 e PA2 in MODALITA' ANALOGICA (ponte)
  GPIOA->MODER |= GPIO_MODER_MODER4; //modalità analogica 
  GPIOA->MODER |= GPIO_MODER_MODER2; //modalità analogica
  
  //***********************************CONFIGURAZIONE DAC
  DAC->CR |= DAC_CR_EN1;                //Abilitazione CANALE 1 del DAC
                                        //Se non si attivava il PA4, non funzionava
  DAC->DHR12R1 = codice_DAC_ADC;        //Scrittura codice nel DHR
  for(int i = 0; i < 1000; i++);        //attesa generazione di tensione
    
  
  //******************************CONFIGURAZIONE ADC*******************************************
  //------Abilitazione regolatore di tensione per rendere più stabile il segnale
  ADC1->CR &= ~ADC_CR_ADVREGEN_1;      //advregen da 10 --> 00 (inizio sequenza)
  ADC1->CR |= ADC_CR_ADVREGEN_0;       //advregen da 00 --> 01 (fine sequenza)
  for(int i = 0; i < 1000; i++);       //attesa di 10 us (microsecondi) (tempo di start up)
  
  //------Configurazione del Clock
  ADC1_2->CCR |= ADC12_CCR_CKMODE_0;    //CKMODE=01 per sincro con clock del bus AHB
                                        //così ho frequenza a 72 MHz
  
  //------Calibrazione dell ADC
  ADC1->CR |= ADC_CR_ADCAL;             //Avvio calibrazione ad 1
  while((ADC1->CR & ADC_CR_ADCAL) == ADC_CR_ADCAL); //attesa dell' ADCAL=0, calibrazione finita
  
  //------Abilitazione dell'ADC
  ADC1->CR |= ADC_CR_ADEN;              //Avvio abilitazione ADC ad 1
  while((ADC1->ISR & ADC_ISR_ADRD) != ADC_ISR_ADRD); //attesa dell' ADRDY ad 1;
  
  //Configurazione ADC
  ADC1->CFGR &= ~ADC_CFGR_CONT;         //cont = 1 conversione singola
  ADC1->SQR1 = (3<<6);                  //SQ1=00011: canale 3 (PA2)
  ADC1->SQR1 &= ~ADC_SQR1_L;            
  ADC1->SMPR1 |= ADC_SMPR1_SMP3;        
  


  ADC1->CR |= ADC_CR_ADSTART;           //avvio la conversione
  while(1)
  {
    //Risultato DAC
    tensione_DAC = codice_DAC_ADC * (3.0/4095.0);
    //CONVERSIONE
    while((ADC1->ISR & ADC_ISR_EOC) != ADC_ISR_EOC);    //attesa EOC = 1
    
    //-------------CONFIGURAZIONE INTERRUPT EOC------------------------------
    //ADC1->IER |= ADC_ISR_EOC;
    ADC1->IER |= ADC_IER_EOC;      //Abilitare l'INTERRUPT sull'evento di EndOfConversion
    NVIC->ISER[0] |= 1<<18;       //Abilitare la 18esima posizione della tabella di NVIC
    //Vai in ADC1_2_IRQHandler()
  }
  
}

void ADC1_2_IRQHandler()
{
  //lettura del risultato ADC
  tensione_ADC = ADC1->DR * (3.0/4096.0);
  codice_ADC_DAC = ADC1->DR;
}
