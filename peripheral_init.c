#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"



void RCC_Init (void){
	
	RCC->CR |= ((uint32_t)RCC_CR_HSEON); 												// Enable HSE
	while (!(RCC->CR & RCC_CR_HSERDY));													// Ready start HSE		
	
	FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;					// Cloclk Flash memory
	
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;														// AHB = SYSCLK/1
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;														// APB1 = HCLK/1
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;														// APB2 = HCLK/1
	
	RCC->CFGR &= ~RCC_CFGR_PLLMULL;               							// clear PLLMULL bits
	RCC->CFGR &= ~RCC_CFGR_PLLSRC;															// clearn PLLSRC bits
	RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;														// clearn PLLXTPRE bits
	
	RCC->CFGR |= RCC_CFGR_PLLSRC_PREDIV1; 											// source HSE
	RCC->CFGR |= RCC_CFGR_PLLXTPRE_PREDIV1_Div2; 								// source HSE/2 = 4 MHz
	RCC->CFGR |= RCC_CFGR_PLLMULL6; 														// PLL x6: clock = 4 MHz * 6 = 24 MHz
	
	RCC->CR |= RCC_CR_PLLON;                      							// enable PLL
	while((RCC->CR & RCC_CR_PLLRDY) == 0) {}      							// wait till PLL is ready
	
	RCC->CFGR &= ~RCC_CFGR_SW;                   							 	// clear SW bits
  RCC->CFGR |= RCC_CFGR_SW_PLL;                 							// select source SYSCLK = PLL
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1) {} 			// wait till PLL is used
	
}

void MCO_out (void){
	
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;                       	// enable clock for port A
	
	GPIOA->CRH &= ~GPIO_CRH_CNF8_0;															// setting out alternative push-pull for PA8
	GPIOA->CRH |= GPIO_CRH_CNF8_1;
	GPIOA->CRH |= (GPIO_CRH_MODE8_0 | GPIO_CRH_MODE8_1);				// gpio speed 50 MHz
	
	RCC->CFGR |= RCC_CFGR_MCO_SYSCLK;														// select source clokc SYSCLK
	
}

void GPIO_Init (void){
	
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;                       	// enable clock for port A

	GPIOC->CRH &= ~GPIO_CRH_CNF8;
	GPIOC->CRH |= GPIO_CRH_MODE8_0;
	
	GPIOC->CRH &= ~GPIO_CRH_CNF9;
	GPIOC->CRH |= GPIO_CRH_MODE9_0;
	
	GPIOC->CRL &= ~GPIO_CRL_CNF0;
	GPIOC->CRL &= ~GPIO_CRL_MODE0;
	GPIOC->CRL |= GPIO_CRL_CNF0_0;
	
}

void UART_Init (void){
	
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;    
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	
	GPIOA->CRH &= ~GPIO_CRH_CNF9;
	GPIOA->CRH |= GPIO_CRH_CNF9_1;
	GPIOA->CRH |= GPIO_CRH_MODE9;
	
	GPIOA->CRH |= GPIO_CRH_CNF10_0;
	GPIOA->CRH &= ~GPIO_CRH_MODE10;
	
	USART1->BRR = 0x9C4;
	
	USART1->CR1 |= USART_CR1_TE;
	USART1->CR1 |= USART_CR1_RE;
	USART1->CR1 |= USART_CR1_UE;
	
	USART1->CR1 |= USART_CR1_RXNEIE;	
	NVIC_EnableIRQ(USART1_IRQn);
	
}

void RTC_Init (void){
	
	// switch on clock of PA, alternative functios and timer6
  //RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
	
  RCC->APB1ENR |= RCC_APB1ENR_BKPEN; 
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR|=PWR_CR_DBP;
	//RCC->BDCR|=RCC_BDCR_BDRST;
	RCC->BDCR|=RCC_BDCR_LSEON;
	while(!(RCC->BDCR & RCC_BDCR_LSERDY)) {};
	//RCC->BDCR|=RCC_BDCR_BDRST;		
	//RCC->BDCR&=~RCC_BDCR_RTCSEL;	
	RCC->BDCR|=RCC_BDCR_RTCSEL_0;
	RCC->BDCR|=RCC_BDCR_RTCEN;
  RTC->CRH|=RTC_CRH_SECIE;		
	while(!(RTC->CRL & RTC_CRL_RSF)) {};
  while(!(RTC->CRL & RTC_CRL_RTOFF)) {};
	RTC->CRL|=RTC_CRL_CNF;
	RTC->PRLL|=32000-1;
	RTC->CRL&=~RTC_CRL_CNF;
	NVIC_EnableIRQ(RTC_IRQn);	

}

void InitADC (void){
	
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
		RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	
		//GPIOA->CRL &= ~GPIO_CRL_CNF0;
	  //GPIOA->CRL &= ~GPIO_CRL_MODE0;
		
		RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
		RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2;
	
		ADC1->CR2 |= ADC_CR2_CAL; 														// start calibration
		while (!(ADC1->CR2 & ADC_CR2_CAL));                   // waiting end calibration
	    
		ADC1->CR2 |= ADC_CR2_ADON;     												// enable ADC
	  ADC1->CR2|=ADC_CR2_TSVREFE;														// wake up sensor from power down mode
		ADC1->CR2 &= ~ADC_CR2_CONT;														// 0 - single conversion, 1 - continuous conversion	
		ADC1->CR2 |= ADC_CR2_EXTSEL;      										// event start conversion SWSTART
		ADC1->CR2 |= ADC_CR2_EXTTRIG;                         // enable start conversion external signal
		ADC1->SMPR1 |= ADC_SMPR1_SMP16; 											// sempling 239.5 cycle
		ADC1->SQR3 |= ADC_SQR3_SQ1;    											  // selection channel
	  ADC1->SQR3 &=~ADC_SQR3_SQ1_4; 
		
		ADC1->CR1 |= ADC_CR1_EOCIE; 
		
		NVIC_EnableIRQ(ADC1_IRQn);
}

void Timer2_Capture_Init (void){
	
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 
	
	// echo input PA1
	GPIOA->CRL &= ~GPIO_CRL_CNF1;      // erase
	GPIOA->CRL |= GPIO_CRL_CNF1_0;    //  port like floating input
	GPIOA->CRL &= ~GPIO_CRL_MODE1;    //erase
	//*****************
	
	// PA2 output for triger like general push pull
	GPIOA->CRL &= ~GPIO_CRL_CNF2;           // erase 00
	GPIOA->CRL &= ~GPIO_CRL_MODE2;          // erase mode
	GPIOA->CRL |= GPIO_CRL_MODE2_0;         //  mode output  
  GPIOA->BSRR |= GPIO_BSRR_BR2;	     // 0
	//************************
	
  TIM2->PSC = 24 - 1;

  TIM2->CCMR1 |= TIM_CCMR1_CC2S_0;  // choise TI2 for input
  TIM2->CCMR1 &= ~(TIM_CCMR1_IC2F | TIM_CCMR1_IC2PSC);  // no use filter and prescaler
	
  TIM2->CCER |= TIM_CCER_CC2P;  // capt low front
  TIM2->CCER |= TIM_CCER_CC2E;  // capt mode for 2 channel     

  TIM2->DIER |= TIM_DIER_CC2IE;    // interrupt bit
	NVIC_EnableIRQ(TIM2_IRQn);
	
 // TIM2->CR1 |= TIM_CR1_CEN;     // go timer

}



