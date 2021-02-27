#include <stm32f30x.h>

int acceso = 0;                         //flag utile per indicare se tutti i led sono accesi oppure no.
int accesoB = 0;                        //flag utile per indicare se il led PE8 è acceso oppure no.

int main()
{

  //----------------------ABILITAZIONI---------------
  RCC->AHBENR  |= RCC_AHBENR_GPIOEEN;           //Abilitazione GPIOE per i LED
  RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;           //Abilitazione GPIOA per il pulsante USER
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;           //Abilitazione TIM2 (Timer collegato al BUS APB1) 
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;         //Abilitazione SYSCFG (collegato al BUS APB2)
  
    
  //--------CONFIGURAZIONE MODER GPIOE e GPIOA-------------
  GPIOE->MODER |= 0x55550000;                   //Pongo a 1 tutti i LED da accendere e spegnere      
  GPIOA->MODER &= ~GPIO_MODER_MODER0;           //Setto il MODER0 come input e dunque devo azzerare tutti i bit (input digitale 00)

  //--------ABILITAZIONE INTERRUPT DEL TIM2----------------
  TIM2->DIER |= TIM_DIER_UIE;                   //Abilitare l'INTERRUPT sull'evento di update del TIMER, quindi si deve alzare il bit UIE
  NVIC->ISER[0] |= (1<<28);                     //Abilitare la 28esima posizione della tabella di NVIC
  
  /*
  * L'uso di NVIC è legato a ISER (Interrupt Set Enable Register) che è la tabella di tutti gli interrup
  * gestibili dal Microcontrollore.
  * Si usa ISER[0] perchè ISER0 racchiude le prime 32 posizioni della tabella, dunque
  * quindi ISER1 le altre 32 posizioni e così via...
  */
  
  /* CALCOLO N CONTEGGI
  *  fck       = 8MHz*9 = 72MHz (Per via del pll) fck (frequenza del clock)
  *  Tck       = 1/fck = 1/72MHz (Periodo)
  *  Deltat    = 10s (tempo che vogliamo)
  *  N         = Deltat/Tck = 720.000.000 (Numero dei conteggi)
  *  dunque TIM2_ARR = 720000000. 
  */

  //--------GESTIONE DEL TIMER----------------------------
  TIM2->ARR = 720000000;                //Quando N raggiunge 720000000, si alza l'UIE (update)
  TIM2->CNT = 0;                        //Inizializzo il contatore a partire da 0
  TIM2->CR1 |= TIM_CR1_CEN;             //Abilitazione il bit CR1_CEN che si trova al bit 0, serve per abilitare il conteggio
  
  //--------ABILITAZIONE INTERRUPT DEL PA0----------------
  EXTI->IMR |= EXTI_IMR_MR0;                     //Maschero la linea 0 come interrupt
  EXTI->RTSR |= EXTI_RTSR_TR0;                   //Imposto il fronte di salita sulla linea 0 
  SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;  //Configuro l'interrupt esterno con SYSCFG e abilito l'EXTI0 che è collegato a PA0
  NVIC->ISER[0] |= (1<<6);                       //Abilitare la 6ta posizione della tabella di NVIC per l'interrupt di EXTI0
  
  while(1);
  
  return 0;
}

//---------INTERRUPT DEL TIMER2 e LA FUNZIONE CHE DEVE ESSERE ESEGUITA-------
void TIM2_IRQHandler ()
{
  TIM2->SR &= ~TIM_SR_UIF;              //Azzero l'UIF
  if(acceso == 0)                       //Se spento
  {
    GPIOE->ODR  |= 0x0000FF00;          //Accendi tutti i LED
    acceso = 1;
    accesoB = 1;    
  }
  else
  {
    GPIOE->ODR &= ~(0x0000FF00);        //Spegni tutti i LED
    acceso = 0;
  }
}

//---------INTERRUPT DELL' EXTI0 e LA FUNZIONE CHE DEVE ESSERE ESEGUITA-------
void EXTI0_IRQHandler ()
{
  EXTI->PR |= EXTI_PR_PR0;              //Individua la presenza della richiesta di un interruzione esterna
    
  //SE il LEDBLUE è acceso e premo, spegni il LED
  if(accesoB == 1 && ((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0 ))
  {
     while((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0); 
     GPIOE->ODR &= ~(1<<8);            //Spegni il LED BLU
     accesoB = 0;
  }
  
  //SE il LEDBLUE è spento e premo, accendi il LED
  if(accesoB == 0 && ((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0 ))
  {
     while((GPIOA->IDR & GPIO_IDR_0) == GPIO_IDR_0); 
     GPIOE->ODR |= (1<<8);
     accesoB = 1;
  }

}

