#include <math.h>       //utile per il sin
#include <stm32f30x.h>  //libreria per i registri
#include <stdio.h>      

#define N 100
#define PIgreco 3.14

short int j = 0;

//vettore per la discretizzazione del segnale
short int vett[N];

int main()
{
  //-------------------ABILITAZIONE PERIFERICHE------------------------
  RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;    //Abilitazione GPIOA 
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;     //Abilitazione DAC (Digitale -> Analogico)
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;    //Abilitazione TIM2(Timer2)

  
  //--------------CONFIGURAZIONE PA4 collegato al DAC1_OUT1------------
  GPIOA->MODER |= GPIO_MODER_MODER4;
  
  
  //--------------CONFIGURAZIONE DAC e TRIGGER----------------------------
  DAC->CR |= DAC_CR_TEN1;                //Abilitazione TRIGGER (Trigger ENable)      
  DAC->CR |= DAC_CR_TSEL1_2;             /*Selezione del TRIGGER HARDWARE e non SOFTWARE, 
                                         * TSEL(3 bit) e indico 100 come TIMER2 TRGO event (Tab_TSEL)
                                         * altrimenti si usa il trigger SOFTWARE*/
  
  /* TRIGGER SOFTWARE -- Nella Tabella TSEL_SWTRIG è (111)
  DAC->CR |= DAC_CR_TSEL1_0;
  DAC->CR |= DAC_CR_TSEL1_1;
  DAC->CR |= DAC_CR_TSEL1_2; 
  */
  
  //-------------ABILITAZIONE DAC----------------------------------------
  DAC->CR |= DAC_CR_EN1;                //Abilitazione CANALE 1 del DAC
                                        //Se non si attivava il PA4, non funzionava
  
  //-------------GENERAZIONE SINUSOIDE---------------------------------------
  //Sapendo che il DAC puà generare solo tensioni da 0 a 3V
  float off = 1.5;           //Offset = 1.5 così i punti della sinusoide sono tutti nel primo quadrante
  float DACoutput;           //Tensione
  //Calcolo dei CAMPIONI in un periodo 2*PIgreco. 
  for(int i = 0; i < N; i++)
  {
    DACoutput = off + off*sin(2*PIgreco*i/N);           //Calcolo i valori della tensione per ogni punto i-esimo
    vett[i] = (short int)(DACoutput*4095.0/3.0);        //Codifico i valori della DACoutput per inseririrli nel vettore
                                                        /* il calcolo dei DOR (DataOutputRegister)
                                                        * DOR = DACoutput * 4095 / VDDA dove VDDA = 3V
                                                        */
  }
  
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

  
  //Inserisco il primo valore del DAC nel Data Holding Register
  DAC->DHR12R1 = vett[j];
  
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
  TIM2->SR &= ~TIM_SR_UIF;      //Azzero lo Status Register

  //DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1; //In caso di Trigger SOFTWARE
  
  //Scansione del vettore, se J == N, riniziare il conteggio.
  j++;
  if(j == N) {
    j = 0;
  }
  DAC->DHR12R1 = vett[j];       //Carico sul Data Holding Register i punti ricavati nella generazione del segnale
  
  //Una prova di stampa a video per verificare che l'interrupt venga evocato
  printf("X = [%d], Y = [%d]\n", j, vett[j]);   
}

