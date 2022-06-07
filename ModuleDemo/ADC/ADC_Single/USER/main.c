#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"
#include "math.h"

#define PRINTF_LOG printf

#define CONV_CHANNEL_NUM 1
#define VREF (3300)

void UART_Configuration(void);
void ADC_Configuration(void);
void DMA_Configuration(void);

#define ADC_TEST_CHANNEL_PIN (GPIO_Pin_2)
uint8_t ADC_CovChannel[1] = {ADC_Channel_2};
uint8_t ADC_SampleTIME[1] = {ADC_SampleTime_239Cycles5};
uint32_t DAM_ADC_Value[1];
/********************************************************************************/
// ADC��ͨ��ģ��ת��ʾ��������DMA��ʽ��ADC1ת������洢��DMA_ADC_Value�����У�PA2Ϊ�ɼ����ţ��ɼ�����ͨ������1���ͣ�������Ϊ115200
/********************************************************************************/

int main(void)
{
	RCC_ClocksTypeDef clocks;
	Delay_Init(); //��ʼ����ʱ����

	UART_Configuration();		//��ʼ������
	RCC_GetClocksFreq(&clocks); //��ȡϵͳʱ��

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("ADC Single Test\n");

	ADC_Configuration(); //��ʼ��ADC

	while (1)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE); //��ʼת��
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
	uint32_t i;
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE); //ʹ��ADC1,GPIOAʱ��

	GPIO_InitStructure.GPIO_Pin = ADC_TEST_CHANNEL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //����GPIO���ٶ�Ϊ50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	  //ģ������
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  //��ʼ��GPIOA2

	RCC_ADCCLKConfig(RCC_PCLK2_Div8); // ADC���ʱ��ΪPCLK2��8��Ƶ
	ADC_DeInit(ADC1);				  //��λADC1

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;					// ADC�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;						//ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;					//����ת��ģʽ�رգ�ͨ���������
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //ת�����������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				//�Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = CONV_CHANNEL_NUM;				//��Ҫת����ͨ������
	ADC_Init(ADC1, &ADC_InitStructure);									//��ʼ��ADC1

	for (i = 0; i < CONV_CHANNEL_NUM; i++)
	{
		ADC_RegularChannelConfig(ADC1, ADC_CovChannel[i], i + 1, ADC_SampleTIME[i]); //����ADC1��ͨ��
	}

	ADC_SoftwareStartConvCmd(ADC1, ENABLE); //��ʼת��

	ADC_Cmd(ADC1, ENABLE); //ʹ��ADC1

	ADC_ResetCalibration(ADC1); //��λУ׼�Ĵ���
	while (ADC_GetResetCalibrationStatus(ADC1))
		;						//�ȴ���λУ׼�Ĵ������
	ADC_StartCalibration(ADC1); //��ʼУ׼
	while (ADC_GetCalibrationStatus(ADC1))
		; //�ȴ�У׼���

	DMA_Configuration(); // DMA����
}

void DMA_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);						//ʹ��DMA1ʱ��
	DMA_DeInit(DMA1_Channel1);												//��λDMA1ͨ��1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;			// DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)DAM_ADC_Value;			// DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// DMA�������赽�ڴ�
	DMA_InitStructure.DMA_BufferSize = CONV_CHANNEL_NUM;					// DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//�����ַ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //�������ݿ��Ϊ32λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;			//�ڴ����ݿ��Ϊ32λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							//������ѭ������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						// DMAͨ��x�����ȼ�Ϊ��
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// ���ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);							//����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��x�ļĴ���
	DMA_Cmd(DMA1_Channel1, ENABLE);											//ʹ��DMA1ͨ��1
	ADC_DMACmd(ADC1, ENABLE);												//ʹ��ADC1��DMA֧��

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;  // DMA1ͨ��1�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
	DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, ENABLE);		  //ʹ��DMA1ͨ��1�ж�
}

void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC1) != RESET) //���DMA1ͨ��1���������־
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1); //���DMA1ͨ��1���������־
		DMA_ClearFlag(DMA1_FLAG_TC1);		//���DMA1ͨ��1������ɱ�־

		PRINTF_LOG("Code Value = %d , ��ѹֵ = %2.4f\n", DAM_ADC_Value[0],
				   (float)VREF * DAM_ADC_Value[0] / 4095 / 1000); //���ADCֵ
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
