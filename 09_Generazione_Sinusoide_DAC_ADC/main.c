#include <math.h>       //utile per il sin
#include <stm32f30x.h>  //libreria per i registri
#include <stdio.h>      

/*
ATTENZIONE
COLLEGAMENTO PONTE tra il PIN4 e il PIN2 sulla STM
*/

#define N 100
#define PIgreco 3.14

short int j = 0;
short int s = 0;

//vettore per la discretizzazione del segnale
short int vett_DAC[N];
short int vett_ADC[N];


int main()
{
    
  //----------------GENERAZIONE SINUSOIDE---------------------------------------
  //Sapendo che il DAC puà generare solo tensioni da 0 a 3V
  float off = 1.5;           //Offset = 1.5 così i punti della sinusoide sono tutti nel primo quadrante
  float DACoutput;           //Tensione
  //Calcolo dei CAMPIONI in un periodo 2*PIgreco. 
  for(int i = 0; i < N; i++)
  {
    DACoutput = off + off*sin(2*PIgreco*i/N);           //Calcolo i valori della tensione per ogni punto i-esimo
    vett_DAC[i] = (short int)(DACoutput*4095.0/3.0);        //Codifico i valori della DACoutput per inseririrli nel vettore
                                                        /* il calcolo dei DOR (DataOutputRegister)
                                                        * DOR = DACoutput * 4095 / VDDA dove VDDA = 3V
                                                        */
  }
  
  //ABILITAZIONE GPIOA - ADC12 - DAC1- TIM2
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
  RCC->AHBENR |= RCC_AHBENR_ADC12EN;
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;  
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;    //Abilitazione TIM2(Timer2)
  
  //ABILITO PA4 e PA2 in MODALITA' ANALOGICA (ponte)
  GPIOA->MODER |= GPIO_MODER_MODER4; //modalità analogica 
  GPIOA->MODER |= GPIO_MODER_MODER2; //modalità analogica
   
  
  //--------------CONFIGURAZIONE DAC e TRIGGER----------------------------
  DAC->CR |= DAC_CR_TEN1;                //Abilitazione TRIGGER (Trigger ENable)      
  DAC->CR |= DAC_CR_TSEL1_2;             /*Selezione del TRIGGER HARDWARE e non SOFTWARE, 
                                         * TSEL(3 bit) e indico 100 come TIMER2 TRGO event (Tab_TSEL)
                                         * altrimenti si usa il trigger SOFTWARE*/
  
  DAC->CR |= DAC_CR_EN1;                //Abilitazione CANALE 1 del DAC
                                        //Se non si attivava il PA4, non funzionava

    
  
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
  
  //Configurazione ADC
  ADC1->CFGR &= ~ADC_CFGR_CONT;          //cont = 1, conversione singola
  ADC1->SQR1 = (3<<6);                   //SQ1=00011: canale 3 (PA2)
  ADC1->SQR1 &= ~ADC_SQR1_L;             //L=0: 1 conv.
  ADC1->SMPR1 |= ADC_SMPR1_SMP3;        //SMP1=111, 601.5 CK
  
  
  //-------------CONFIGURAZIONE INTERRUPT TIM2------------------------------
  TIM2->DIER |= TIM_DIER_UIE;   //Abilitare l'INTERRUPT sull'evento di UPDATE del TIMER, quindi si deve alzare il bit UIE
  NVIC->ISER[0] |= 1<<28;       //Abilitare la 28esima posizione della tabella di NVIC
     
  /* CALCOLO CONTEGGI ARR
  *  fck       = 8MHz*9 = 72MHz (Per via del pll) fck (frequenza del clock)
  *  Tck       = 1/fck = 1/72MHz (Periodo)
  *  Deltat    = 1ms = 1*(10^-3)(tempo che vogliamo)
  *  Deltat    = N * tupdate (N = numero campioni) 
  *  tupdate   = Deltat/Ncampioni = (10^-3)/(100) = (10^-5) (tempo di refresh)
  *  N         = tupdate/Tck = 7200 (Numero dei conteggi)
  *  dunque TIM2_ARR = 7200. 
  */
  
  TIM2->ARR = 7200; 
  TIM2->CNT = 0;                //Abilitazione il Contatore a partire da 0

  
  //----------------------ABILITO IL CONTEGGIO 
  TIM2->CR1 = TIM_CR1_CEN;      //Abilitazione del conteggio 
  TIM2->CR2 = TIM_CR2_MMS_1;    /*Essendo che si è scelto di utilizzare il trigger Hardware del TIMER2
                                * ho la necessità di usare il Control Register 2 (CR2), fissando come
                                * 3bit 010, in questo modo, il TriggerOut viene mandato in corrispondenza
                                * di un evento di UPDATE (quando il TIM2_ARR arriva a 7200, scatta l'update)
                                * e così si attiva l'interrupt*/

  while(1);
  
}

void TIM2_IRQHandler()
{
  DAC->DHR12R1 = vett_DAC[j];       //Carico sul Data Holding Register i punti ricavati nella generazione del segnale
  TIM2->SR &= ~TIM_SR_UIF;      //Azzero lo Status Register

  //DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1; //In caso di Trigger SOFTWARE
  
  for(int k = 0; k < 1000; k++);       //attesa generazione di tensione
  
  //CONVERSIONE
  ADC1->CR |= ADC_CR_ADSTART;           //ADSTART=1, avvio la conversione
  while((ADC1->ISR & ADC_ISR_EOC) != ADC_ISR_EOC); //ATTESA EOC=1;
  
  //-------------CONFIGURAZIONE INTERRUPT EOC------------------------------
  //ADC1->IER |= ADC_ISR_EOC;
  ADC1->IER |= ADC_IER_EOC;      //Abilitare l'INTERRUPT sull'evento di EndOfConversion
  NVIC->ISER[0] |= 1<<18;       //Abilitare la 18esima posizione della tabella di NVIC
 
    
  //Una prova di stampa a video per verificare che l'interrupt venga evocato
  printf("X = [%d], Y = [%d]\n", j, vett_DAC[j]);   
  
  //Scansione del vettore, se J == N, riniziare il conteggio.
  j++;
  if(j == N) {
    j = 0;
  }
}


void ADC1_2_IRQHandler()
{
  //Lettura dall'ADC_DR
  vett_ADC[s] = ADC1->DR;
  //Scansione del vettore, se s == N, riniziare il conteggio.
  s++;
  if(s == N) {
    s = 0;
  }
}

