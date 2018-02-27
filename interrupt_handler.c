#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "main.h"

extern unsigned int Timer2_Value;
extern T_TIME Time;
extern char TimeStr[4];
extern uint16_t ADC_Result;
//uint16_t ADC_meashurement[10];
//uint8_t ADC_Counter;

const float V_25=1.41;
const float Avg_Slope=0.0043;
const float V_ref=2.5;
float V_tempsensor;
extern float Temperature_of_sensor;


void USART1_Send (char chr);
void USART1_Send_String (char* str);
void RTC_Time_Processor (uint16_t rtc_time_sec);
// uart1 recieve interrupt
void USART1_IRQHandler (void){
	
	if (USART1->SR & USART_SR_RXNE){
		
		USART1->SR &= ~USART_SR_RXNE;
		
		if (USART1->DR == '0'){
			
			USART1_Send_String ("LED OFF\r\n");
			GPIOC->BSRR |= GPIO_BSRR_BR8;
			
		}
		
		if (USART1->DR == '1'){
			
			USART1_Send_String ("LED ON\r\n");
			GPIOC->BSRR |= GPIO_BSRR_BS8;
			
		}
		
	}
	
}

void RTC_IRQHandler (void){
	RTC->CRL&=~RTC_CRL_SECF;
	RTC_Time_Processor (RTC->CNTL);
	//sprintf(TimeStr, "%u", Time.Second);
	//Timer_Value++;
}

void ADC1_IRQHandler (void){
	
	static uint8_t adc_counter=0;
	static uint16_t ADC_meashurement[10];
	
	if (adc_counter<10)	ADC_meashurement[adc_counter++]=ADC1->DR;
	else {
			for (uint8_t i=0; i<adc_counter; i++) {		
				ADC_meashurement[0]+= ADC_meashurement[i+1];
			}  
			ADC_Result=ADC_meashurement[0]/adc_counter;
			V_tempsensor=(V_ref/4096)*ADC_Result;
			Temperature_of_sensor=((V_25-V_tempsensor)/Avg_Slope)+25;
			adc_counter=0;
	}
	//ADC1->CR2 |= ADC_CR2_SWSTART;
}

void TIM2_IRQHandler  (void){
	
	Timer2_Value=TIM2->CCR2;
	
}
