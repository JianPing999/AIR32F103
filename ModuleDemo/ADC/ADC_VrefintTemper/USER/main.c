#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"
#include "math.h"

#define PRINTF_LOG printf

#define CONV_CHANNEL_NUM 2
#define VREF (3300)

void UART_Configuration(void);
void ADC_Configuration(void);
void DMA_Configuration(void);

uint8_t ADC_CovChannel[2] = {ADC_Channel_16, ADC_Channel_17};//Channel_16为内部温度传感器，Channel_17为外部温度传感器
uint8_t ADC_SampleTIME[2] = {ADC_SampleTime_239Cycles5, ADC_SampleTime_239Cycles5};
uint32_t DAM_ADC_Value[2];
/********************************************************************************/
// ADC内部基准与温度传感器采集示例，采集数据通过串口1发送，波特率为115200
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;
	Delay_Init(); //初始化延时函数

	UART_Configuration(); //初始化串口
	ADC_Configuration();  //初始化ADC

	RCC_GetClocksFreq(&clocks); //获取系统时钟

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("ADC温度传感器和VREFINT通道, Channel_16、Channel_17\n");

	while (1)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE); //开始转换
		Delay_Ms(1000);
	}
}

uint8_t GetCmd(void)
{
	uint8_t tmp = 0;

	if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
	{
		tmp = USART_ReceiveData(USART1);
	}
	return tmp;
}

void UART_Configuration(void)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}

void ADC_Configuration(void)
{
	uint32_t i;
	ADC_InitTypeDef ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE); //使能ADC1,GPIOA时钟

	RCC_ADCCLKConfig(RCC_PCLK2_Div4); // ADC时钟使能，ADCCLK = PCLK2/4 = HCLK/4 = SystemCoreClock/4
	ADC_DeInit(ADC1);				  //复位ADC1

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;					// ADC工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;						//扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;					//转换连续模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //触发模式，不触发
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				//右对齐
	ADC_InitStructure.ADC_NbrOfChannel = CONV_CHANNEL_NUM;				//需要转换的通道数
	ADC_Init(ADC1, &ADC_InitStructure);									//根据指定的参数初始化外设ADC1

	for (i = 0; i < CONV_CHANNEL_NUM; i++)
	{
		ADC_RegularChannelConfig(ADC1, ADC_CovChannel[i], i + 1, ADC_SampleTIME[i]); //设置ADC1的转换通道
	}

	ADC_TempSensorVrefintCmd(ENABLE);		//使能温度传感器和VREFINT通道
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); //开始转换

	ADC_Cmd(ADC1, ENABLE); //使能ADC1

	ADC_ResetCalibration(ADC1); //复位校准寄存器
	while (ADC_GetResetCalibrationStatus(ADC1))
		;						//等待复位校准寄存器完成
	ADC_StartCalibration(ADC1); //开始校准
	while (ADC_GetCalibrationStatus(ADC1))
		; //等待校准完成

	DMA_Configuration(); //	DMA配置
}

void DMA_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);						//使能DMA1时钟
	DMA_DeInit(DMA1_Channel1);												//	复位DMA1通道1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;			// ADC1数据寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)DAM_ADC_Value;			// DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// DMA方向：外设到内存
	DMA_InitStructure.DMA_BufferSize = CONV_CHANNEL_NUM;					// DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //外设数据宽度为32位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;			//内存数据宽度为32位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							//工作在循环缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							//非内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);							//根据DMA_InitStruct中指定的参数初始化DMA的通道1的寄存器
	DMA_Cmd(DMA1_Channel1, ENABLE);											//使能DMA1通道1
	ADC_DMACmd(ADC1, ENABLE);												//使能ADC1的DMA支持

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;  // DMA1通道1中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
	DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, ENABLE);		  //使能DMA1通道1中断
}

void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC1) != RESET) //检查DMA1通道1传输完成中断
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1); //清除DMA1通道1传输完成中断标志
		DMA_ClearFlag(DMA1_FLAG_TC1);		//清除DMA1通道1传输完成标志

		PRINTF_LOG("温度传感器AD值 = %d , 电压值 = %2.4f , VREFINT = %d , 电压值 = %2.4f\n", DAM_ADC_Value[0],
				   (float)VREF * DAM_ADC_Value[0] / 4095 / 1000, DAM_ADC_Value[1], (float)VREF * DAM_ADC_Value[1] / 4095 / 1000);
	}
}

// Retarget Printf
int SER_PutChar(int ch)
{
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TC))
		;
	USART_SendData(USART1, (uint8_t)ch);

	return ch;
}

int fputc(int c, FILE *f)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	if (c == '\n')
	{
		SER_PutChar('\r');
	}
	return (SER_PutChar(c));
}
