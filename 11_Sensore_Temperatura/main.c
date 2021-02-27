#include <stm32f30x.h>  //libreria per i registri
#include <stdio.h>      

float temp;
float tensione;

int main()
{
  //**********************ABILITAZIONE ADC12
  RCC->AHBENR |= RCC_AHBENR_ADC12EN;
   
  //**********************ABILITAZIONE SENSORE DELLA TEMPERATURA
  ADC1_2->CCR |= ADC12_CCR_TSEN;
  
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
  while((ADC1->ISR & ADC_ISR_ADRD) != ADC_ISR_ADRD); //attesa dell' ADRDY = 1;
  
  //-----Configurazione ADC
  ADC1->CFGR |= ADC_CFGR_CONT;          
  ADC1->SQR1 &= ~ADC_SQR1_L;            //Singolo canale
  ADC1->SQR1 = ADC_SQR1_SQ1_4;          //Sensore della temperatura sul canale 16
  ADC1->SMPR2 |= ADC_SMPR2_SMP16_1;     //t di sampling del cancale 16 a 2.5 microsecondi (richiesti 2.2 microsecondi)
  
  
  //******************INIZIO CONVERSIONE
  ADC1->CR |= ADC_CR_ADSTART;
  
  while(1){
  
      //attesa di EOC (End of Conversion) == 1
      while((ADC1->ISR & ADC_ISR_EOC) != ADC_ISR_EOC);
      
      tensione = ADC1->DR * (3.0/4096);
  
      //Calcolo della temperatura
      temp = ((1.43 - tensione)/4.3) + 25;
      printf("Temperatura intorno ai %f gradi \n",temp);
  }
  
}


