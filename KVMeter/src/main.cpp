
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include <stm32f10x_adc.h>
#include "misc.h"
#include "s6d0164.h"
#include "objects.h"
#include "delay.h"


ADC_MODE adc_mode = VOLTAGE;

float voltage = 0.0f;
float cpuTemp = 0.0f;

S6D0164 display;

void Faults_Configuration()
{

	SCB->CCR |= SCB_CCR_DIV_0_TRP;	
}


void RCC_Configuration()
{
	SysTick_Config(SystemCoreClock / 1000);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1, ENABLE);
	
	//clock DMA1 controller
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
}

void GPIO_Configuration()
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); //disable JTAG
}


void NVIC_Configuration()
{

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(ADC1_2_IRQn);
}


void ADC_Configuration()
{
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	ADC_DeInit(ADC1);
	ADC_InitTypeDef  ADC_InitStructure;

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_Cmd(ADC1, ENABLE);
	
	//calibrate ADC
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));
	
	ADC_Cmd(ADC1, DISABLE);
}



int main()
{
	Faults_Configuration();
	RCC_Configuration();
	GPIO_Configuration();
	ADC_Configuration();
	NVIC_Configuration();
	
	
	while (true)
	{
		GPIOC->BSRR = GPIO_Pin_8;	
		GPIOC->BRR = GPIO_Pin_9;
		DelayManager::Delay(1000);
		GPIOC->BSRR = GPIO_Pin_9;	
		GPIOC->BRR = GPIO_Pin_8;
		DelayManager::Delay(1000);
	}
}



#pragma region Helpers
void readADCValue()
{
	ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	uint16_t val = ADC_GetConversionValue(ADC1);
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	if (adc_mode == VOLTAGE)
	{
		voltage = 2 * (val * 3.06f) / 4095;	
	}
	else if (adc_mode == TEMPERATURE)
	{
		ADC_TempSensorVrefintCmd(DISABLE);
		float v25 = 1.43f;
		float avgSlope = 4.3f;
		cpuTemp = ((v25 - ((val * 3.3f) / 4095)) / avgSlope) + 25;
	}
	ADC_Cmd(ADC1, DISABLE);
}

void startVoltageMeasure()
{
	adc_mode = VOLTAGE;
	ADC_Cmd(ADC1, ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void startTemperatureMeasure()
{
	adc_mode = TEMPERATURE;
	ADC_Cmd(ADC1, ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_41Cycles5);
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

#pragma endregion

