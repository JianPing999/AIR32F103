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

uint32_t DAM_ADC_Value[1];

int main(void)
{
	RCC_ClocksTypeDef clocks;
	Delay_Init();

	UART_Configuration();
	RCC_GetClocksFreq(&clocks);

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("ADC Double Test(ADC1 & ADC2)\n");

	ADC_Configuration();//ADC����

	while (1)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE); // ����ADC1���ת��
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

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); // ʹ��USART1��GPIOAʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		  // PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO���ٶ�Ϊ50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  // �����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			  // GPIOA.10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // ��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;									// ����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// �ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								// ����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// �շ�ģʽ

	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}

void ADC_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE); // ʹ��ADC1��ADC2��GPIOAʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;		  // PA.0
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO���ٶ�Ϊ50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	  // ģ������
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  // ��ʼ��GPIOA.0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;		  // PA.1
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  // ��ʼ��GPIOA.1

	RCC_ADCCLKConfig(RCC_PCLK2_Div8); // ADCCLK=PCLK2/8=9Mhz
	ADC_DeInit(ADC1);				  // ��λADC1
	ADC_DeInit(ADC2);				  // ��λADC2

	ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;					// ͬ������ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;						// ��ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;					// �ر�����ת��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				// �Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;								// 1��ת���ڹ��������� Ҳ����ֻת����������1
	ADC_Init(ADC1, &ADC_InitStructure);									// ADC1��ʼ��
	ADC_Init(ADC2, &ADC_InitStructure);									// ADC2��ʼ��

	/* ͨ������ */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5); // ADC1ͨ��0,�������˳��Ϊ1,����ʱ��Ϊ239.5����
	ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5); // ADC2ͨ��1,�������˳��Ϊ1,����ʱ��Ϊ239.5����

	/* ģ��ʹ�� */
	ADC_Cmd(ADC1, ENABLE); // ʹ��ָ����ADC1
	ADC_Cmd(ADC2, ENABLE); // ʹ��ָ����ADC2

	/* ADCУ׼ */
	ADC_ResetCalibration(ADC1);					// ʹ�ܸ�λУ׼
	while (ADC_GetResetCalibrationStatus(ADC1)) // ���ָ����ADC1�Ĵ����Ƿ�λ���
		;
	ADC_StartCalibration(ADC1);			   // ����ADУ׼
	while (ADC_GetCalibrationStatus(ADC1)) // ���ָ����ADC1�Ƿ���У׼
		;
	ADC_ResetCalibration(ADC2);					// ʹ�ܸ�λУ׼
	while (ADC_GetResetCalibrationStatus(ADC2)) // ���ָ����ADC2�Ĵ����Ƿ�λ���
		;
	ADC_StartCalibration(ADC2);			   // ����ADУ׼
	while (ADC_GetCalibrationStatus(ADC2)) // ���ָ����ADC2�Ƿ���У׼
		;

	DMA_Configuration(); // DMA����
}

void DMA_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);						// ʹ��DMA1ʱ��
	DMA_DeInit(DMA1_Channel1);												// ��λDMA1ͨ��1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;			// DMA����ADC����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)DAM_ADC_Value;			// DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// ���ݴ��䷽�򣬴������ȡ���͵��ڴ�
	DMA_InitStructure.DMA_BufferSize = CONV_CHANNEL_NUM;					// DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// �����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// �ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // ���ݿ��Ϊ32λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;			// ���ݿ��Ϊ32λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// ������ѭ������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						// DMAͨ�� xӵ�и����ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);							// ����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��
	DMA_Cmd(DMA1_Channel1, ENABLE);											// ʹ��DMA1ͨ��1
	ADC_DMACmd(ADC1, ENABLE);												// ʹ��ָ����ADC1��DMA����

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;  // DMA1ͨ��1�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // �����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, ENABLE);
}
// DMA1ͨ��1�жϷ������
void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC1) != RESET) // ���ָ����DMA1�жϷ������:DMA1_IT_TC1
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1); // ���DMA1ͨ��1���жϴ�����λ:DMA1_IT_TC1
		DMA_ClearFlag(DMA1_FLAG_TC1);		// ���DMA1ͨ��1���жϱ�־λ:DMA1_FLAG_TC1

		PRINTF_LOG("ADC1 Code Value = %d , ��ѹֵ = %2.4f\n", DAM_ADC_Value[0] & 0xFFFF,
				   (float)VREF * (DAM_ADC_Value[0] & 0xFFFF) / 4095 / 1000);//ǰ16λ��ADC1��ֵ����16λ��ADC2��ֵ����ѹ����VREF*ADC/4095/1000
		PRINTF_LOG("ADC2 Code Value = %d , ��ѹֵ = %2.4f\n", (DAM_ADC_Value[0] >> 16) & 0xFFFF,
				   (float)VREF * ((DAM_ADC_Value[0] >> 16) & 0xFFFF) / 4095 / 1000);
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
