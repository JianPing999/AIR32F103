#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"
#include "math.h"

#define PRINTF_LOG printf
int LTR = 50;	   //��������
int HTRHTR = 4095; //��������
void UART_Configuration(void);
void ADC_Configuration(void);
void GPIO_Configuration(void);
/********************************************************************************/
// ADCģ�⿴�Ź��ж�ʾ����PA2ΪADC�������ţ��ӵ�ʱ������������PB11�����ӵ�LED��˸
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;
	Delay_Init(); //��ʱ��ʼ��

	UART_Configuration();		//Ĭ��Ϊ����1��������115200
	ADC_Configuration();		// ADC��ʼ��
	GPIO_Configuration();		// GPIO��ʼ��
	RCC_GetClocksFreq(&clocks); //��ȡϵͳʱ��Ƶ��
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("ADC Analog Watchdog Test\n");
	PRINTF_LOG("LTR: %d, HTR��%d\n", LTR, HTRHTR);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); //ʹ��ADC���ת��
	while (1)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_11);
		Delay_Ms(200);
		ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE); //ʹ��ADC�ж�
		Delay_Ms(200);
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
	// GPIO�˿�����
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
	ADC_InitTypeDef ADC_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE); //ʹ��ADC1ͨ��ʱ��

	RCC_ADCCLKConfig(RCC_PCLK2_Div6); //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M
	ADC_DeInit(ADC1);				  //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;					// ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;						// ADC�����ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;					//����ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				// ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;								//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);									//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���

	ADC_Cmd(ADC1, ENABLE); //ʹ��ָ����ADC1

	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;		  //����ADC1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //ʹ��ADC1�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);							  //����ָ���Ĳ�����ʼ��VIC�Ĵ���

	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5); // ADC1,ADCͨ��2,����ʱ��239.5����
	ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_2);					 //����ADC1��ģ�⿴�Ź�ͨ��
	ADC_AnalogWatchdogThresholdsConfig(ADC1, HTRHTR, LTR);						 //����ADC���Ź�����ֵ��ADC���Ÿ���һ�㲻Ϊ�㣬��PA2�ӵ�ʱADCֵ����LTR������ADC1��ģ�⿴�Ź��ж�
	ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);			 // ADC1,ADCͨ��2,ģ�⿴�Ź�ʹ��

	ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE); //ʹ��ADC1��ģ�⿴�Ź��ж�

	ADC_ResetCalibration(ADC1);					// ADC1��λУ׼
	while (ADC_GetResetCalibrationStatus(ADC1)) //�ȴ���λУ׼����
		;
	ADC_StartCalibration(ADC1);			   //��ʼADC1У׼
	while (ADC_GetCalibrationStatus(ADC1)) //�ȴ�У׼����
		;
}

void ADC1_2_IRQHandler(void)
{
	ADC_ITConfig(ADC1, ADC_IT_AWD, DISABLE);		  //�ر�ADC1��ģ�⿴�Ź��ж�
	if (SET == ADC_GetFlagStatus(ADC1, ADC_FLAG_AWD)) //���ADC1��ģ�⿴�Ź���־λ
	{
		ADC_ClearFlag(ADC1, ADC_FLAG_AWD);								   //���ADC1��ģ�⿴�Ź���־λ
		ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);						   //���ADC1��ģ�⿴�Ź��жϱ�־λ
		PRINTF_LOG("ADC Awd is Happened. Code Value = %d \r\n", ADC1->DR); //��ӡADC1��ģ�⿴�Ź��ж�
		GPIO_SetBits(GPIOB, GPIO_Pin_11);
	}
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
// Retarget Printf
int SER_PutChar(int ch)
{
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TC))
		;
	USART_SendData(USART1, (uint8_t)ch);

	return ch;
}
// Retarget Getchar
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
