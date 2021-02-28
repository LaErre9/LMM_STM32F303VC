# Generazione della Sinusoide con l'utilizzo trasferimento dei dati tra memoria e DAC / ADC e memoria mediante DMA
Generazione della sinusoide calcolando i campioni da inserire nella LUT (Memoria) e trasferimento dei dati (DMA2_ch3 direzione MEMORIA->PERIFERICA) nel DHR del DAC. Acquisire i campioni tramite ADC (Collegamento PA4-PA2) e generare una parola codice e trasferirli in memoria (DMA1_ch1 direzione PERIFERICA->MEMORIA)
- Utilizzo della libreria stm32f303x.h
![ESEMPIO_10](https://github.com/LaErre9/LMM_STM32F303VC/blob/main/10_Generazione_Sinusoide_DAC_ADC_DMA/10_esempio_di_funzionamento.png)
