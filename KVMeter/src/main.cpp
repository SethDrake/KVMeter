
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include <stm32f10x_adc.h>
#include "misc.h"
#include "s6d0164.h"
#include "objects.h"
#include "periph_config.h"
#include "fonts.h"
#include "delay.h"
#include <string.h>
#include "stm32f10x_dma.h"


volatile uint32_t adcValue = 0;

float voltage = 0.0f;

uint16_t adcData[220];

S6D0164 display;


void Faults_Configuration()
{

	SCB->CCR |= SCB_CCR_DIV_0_TRP;	
}


void RCC_Configuration()
{
	SysTick_Config(SystemCoreClock / 1000);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1, ENABLE);
	
	//clock DMA1 controller
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
}

void GPIO_Configuration()
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = V_MEASURE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(V_MEASURE_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = 0x00FF; //lower 8 bit
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(S6D0164_DATA_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = S6D0164_CTRL_PIN_RD | S6D0164_CTRL_PIN_WR | S6D0164_CTRL_PIN_CS | S6D0164_CTRL_PIN_RS | S6D0164_CTRL_PIN_RESET;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(S6D0164_CTRL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; // indication leds
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); //disable JTAG
}


void NVIC_Configuration()
{

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(ADC1_2_IRQn);*/
}

void DMA_Configuration()
{
	DMA_DeInit(DMA1_Channel1);
	DMA_InitTypeDef DMA_InitStructure;

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	//DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adcValue;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adcData;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 220;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

void ADC_Configuration()
{
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	ADC_DeInit(ADC1);
	ADC_InitTypeDef  ADC_InitStructure;

	ADC_InitStructure.ADC_Mode = ADC_Mode_FastInterl;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_DeInit(ADC2);
	ADC_InitStructure.ADC_Mode = ADC_Mode_FastInterl;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC2, &ADC_InitStructure);
	ADC_ExternalTrigConvCmd(ADC2, ENABLE);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_8, 1, ADC_SampleTime_55Cycles5);
	
	ADC_DMACmd(ADC1, ENABLE);
	ADC_DMACmd(ADC2, ENABLE);

	ADC_Cmd(ADC1, ENABLE);
	ADC_Cmd(ADC2, ENABLE);
	
	//calibrate ADC
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));

	//calibrate ADC
	ADC_ResetCalibration(ADC2);
	while (ADC_GetResetCalibrationStatus(ADC2));
	ADC_StartCalibration(ADC2);
	while (ADC_GetCalibrationStatus(ADC2));

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void drawVoltageGraph(const uint8_t y, const uint16_t barColor, const uint16_t bkgColor);

int main()
{
	Faults_Configuration();
	RCC_Configuration();
	DMA_Configuration();
	GPIO_Configuration();
	ADC_Configuration();
	NVIC_Configuration();
	
	display.setupHw(S6D0164_DATA_PORT, S6D0164_CTRL_PORT, S6D0164_CTRL_PIN_RD, S6D0164_CTRL_PIN_WR, S6D0164_CTRL_PIN_RS, S6D0164_CTRL_PIN_CS, S6D0164_CTRL_PIN_RESET);
	display.init();
	
	display.setRotation(LANDSCAPE);
	
	display.clear(BLUE);
	display.setColor(WHITE, BLUE);
	display.setFont(Consolas17x25);

	const uint32_t r1 = 100000;
	const uint32_t r2 = 7610000;

	const uint32_t divCoeff = (r1 + r2) / r1;

	while (true)
	{			
		voltage = ((adcData[0] * 2.97f) / 4095);	
		const uint16_t intPart = voltage;
		const uint16_t floatPart = ((voltage - intPart) * 100);
		display.printf(5, 10, "%04u.%02u\x3B", intPart, floatPart);
		/*intPart = cpuTemp;
		floatPart = ((cpuTemp - intPart) * 10);
		display.printf(5, 10, "%02u.%01u\x3C\x3A", intPart, floatPart);
		display.drawBorder(3, 2, 145, 60, 1, WHITE);*/
		drawVoltageGraph(40, WHITE, BLUE);
		
		/*GPIOC->BSRR = GPIO_Pin_8;	
		GPIOC->BRR = GPIO_Pin_9;
		DelayManager::Delay(10);
		GPIOC->BSRR = GPIO_Pin_9;	
		GPIOC->BRR = GPIO_Pin_8;
		DelayManager::Delay(10);*/
	}
}

void drawVoltageGraph(const uint8_t y, const uint16_t barColor, const uint16_t bkgColor) 
{
	uint8_t i, j;
	uint8_t barWidth = 1;
	uint8_t maxHeight = 120;
	uint16_t buf[barWidth * maxHeight];
	uint16_t t;
	uint16_t* counts = adcData; 
	for (int16_t r = 219; r >= 0; r--)
	{
		uint16_t lineHeight = counts[r]/40 + 1;
		if (lineHeight > maxHeight)
		{
			lineHeight = maxHeight;
		}
		t = 0;
		for (i = maxHeight; i > 0; i--)
		{
			for (j = 0; j < barWidth; j++)
			{
				if (i <= lineHeight)
				{
					buf[t++] = barColor;
				}
				else
				{
					buf[t++] = bkgColor;
				}
			}		
		}
		display.bufferDraw(r * barWidth, y, barWidth, maxHeight, buf);
	}
}

