#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "main.h"

unsigned int Timer2_Value;
T_TIME Time;
char TimeStr[4];
char Temp_sensorStr[4];
uint16_t ADC_Result;
float Temperature_of_sensor;

void RCC_Init (void);
void MCO_out (void);
void GPIO_Init (void);
void UART_Init (void);
void USART1_Send (char chr);
void USART1_Send_String (char* str);
void RTC_Init (void);
void InitADC (void);
void Timer2_Capture_Init (void);

void RTC_Time_Processor (uint16_t rtc_time_sec);

void vTaskButtonUser (void *argument);
void vTaskTransmitDataUser (void *argument);
void vTaskADCUser (void *argument);
void vTaskTimerTrigerSensorUser (void *argument);
void vTaskSensorLedIndication (void *argument);
	
int main(void){		
	
	RCC_Init();
	MCO_out();
	GPIO_Init();
	UART_Init();
	RTC_Init ();
	InitADC();
	Timer2_Capture_Init ();
	
		xTaskCreate(vTaskTimerTrigerSensorUser, "LED1", 128, NULL, 1, NULL);
		xTaskCreate(vTaskSensorLedIndication, "LED2", 128, NULL, 1, NULL);
	xTaskCreate(vTaskButtonUser, "LED1", 128, NULL, 1, NULL);
	xTaskCreate(vTaskTransmitDataUser, "LED2", 128, NULL, 1, NULL);
	xTaskCreate(vTaskADCUser, "LED2", 128, NULL, 1, NULL);
	vTaskStartScheduler();
	
	while(1)
	{
	//while (!(USART1->SR & USART_SR_TC));
	//USART1->DR = 0x32;			
	}
	
}

/*******************************************************************************************************/

void vTaskButtonUser (void *argument){
	
	while(1)
	{
		
		  if ((GPIOA->IDR & GPIO_IDR_IDR0) != 0){
			vTaskDelay(100);
      USART1->CR1 ^= USART_CR1_TE;
	    }	
		
		  vTaskDelay(200);
		
	}
	
}


void vTaskADCUser (void *argument){
	
	while(1)
	{
		  if ((ADC1->SR & ADC_SR_EOC));
		  else	ADC1->CR2 |= ADC_CR2_SWSTART;
		  vTaskDelay(50);
		
	}
	
}

void vTaskTransmitDataUser (void *argument){
	
	while(1)
	{  
      sprintf(TimeStr, "%u", Time.Day);
			USART1_Send_String (TimeStr);
			USART1_Send_String (" Day\n ");
			sprintf(TimeStr, "%u", Time.Hour);
			USART1_Send_String (TimeStr);
			USART1_Send_String (" Hour\n ");
			sprintf(TimeStr, "%u", Time.Minute);
			USART1_Send_String (TimeStr);
			USART1_Send_String (" Minute\n ");
			sprintf(TimeStr, "%u", Time.Second);
			USART1_Send_String (TimeStr);
			USART1_Send_String (" Second\n ");	
		  sprintf(Temp_sensorStr, "%.3f", Temperature_of_sensor);
		  USART1_Send_String (Temp_sensorStr);
			USART1_Send_String (" Temperature\r\n");
		  vTaskDelay(1000);	
	}
}

void vTaskTimerTrigerSensorUser (void *argument){
	
	while(1)
	{
		GPIOA->BSRR |= GPIO_BSRR_BS2;
		vTaskDelay(10);
		GPIOA->BSRR |= GPIO_BSRR_BR2;
		TIM2->CNT=0;
		TIM2->CR1 |= TIM_CR1_CEN;     // go timer
		vTaskDelay(100);	
	}
}

void vTaskSensorLedIndication (void *argument){
	
	while(1)
	{
		
		GPIOC->BSRR |= GPIO_BSRR_BS8;
		vTaskDelay(Timer2_Value/4);
		GPIOC->BSRR |= GPIO_BSRR_BR8;
		vTaskDelay(Timer2_Value/4);
		
	}
}




void USART1_Send (char chr){
	
	while (!(USART1->SR & USART_SR_TC));
	USART1->DR = chr;
	
}

void USART1_Send_String (char* str){
	
	uint8_t i = 0;
	
	while(str[i])
	USART1_Send (str[i++]);
	
}

void RTC_Time_Processor (uint16_t rtc_time_sec) {
uint16_t temp;	
	
Time.Day=rtc_time_sec/86400;
temp=rtc_time_sec%86400;
Time.Hour=temp/3600;
temp=temp%3600;
Time.Minute=temp/60;
Time.Second=temp%60;	
	
}

