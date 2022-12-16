/*
1.ϵͳʱ������Ϊ T1 = 1/72000000
2.��ʱ��TIM2 �������� T2 = (20)*(1*T1)
3.���Ҳ���������ʱ��T3 = T2*32
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

#define DAC1_DHR12RD_ADDRESS (DAC_BASE + 0x00000008 + DAC_Align_12b_R)
#define DAC2_DHR12RD_ADDRESS (DAC_BASE + 0x00000014 + DAC_Align_12b_R)

// ���Ҳ��������ڵĵ���
#define POINT_NUM 32

/* �������� */
uint16_t Sine12bit[POINT_NUM] = {
	2048, 2460, 2856, 3218, 3532, 3786, 3969, 4072,
	4093, 4031, 3887, 3668, 3382, 3042, 2661, 2255,
	1841, 1435, 1054, 714, 428, 209, 65, 3,
	24, 127, 310, 564, 878, 1240, 1636, 2048};

uint32_t DualSine12bit[POINT_NUM];

USART_TypeDef *USART_TEST = USART1;
RCC_ClocksTypeDef clocks;

void UART_Configuration(uint32_t bound);
void GPIO_Configuration(void);
void DAC_Configuration(void);
void TIM_Configuration(void);
void DMA_Configuration(void);

int main(void)
{
	uint32_t i;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
	Delay_Init();
	UART_Configuration(115200);
	RCC_GetClocksFreq(&clocks);

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 DAC Out Software Sine Wave.\n");

	GPIO_Configuration();
	DAC_Configuration();
	TIM_Configuration();

	for (i = 0; i < POINT_NUM; i++)
	{
		DualSine12bit[i] = (Sine12bit[i] << 16) + (Sine12bit[i]);// ����ͨ��������
	}

	DMA_Configuration();

	while (1)
		;
}

void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE); // ʹ��DMA2ʱ��

	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC2_DHR12RD_ADDRESS;		// DAC2ͨ��DMA��ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&DualSine12bit;		// �ڴ��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// ������Ϊ���ݴ����Ŀ�ĵ�
	DMA_InitStructure.DMA_BufferSize = POINT_NUM;							// DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// �����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// �ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // ���ݿ��Ϊ32λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;			// ���ݿ��Ϊ32λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						// DMAͨ�� xӵ�и����ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��

	DMA_Init(DMA2_Channel4, &DMA_InitStructure); // ����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��
	DMA_Cmd(DMA2_Channel4, ENABLE);				 // ʹ��DMA2ͨ��4

	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC1_DHR12RD_ADDRESS; // DAC1ͨ��DMA��ַ
	DMA_Init(DMA2_Channel3, &DMA_InitStructure);					 // ����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��
	DMA_Cmd(DMA2_Channel3, ENABLE);									 // ʹ��DMA2ͨ��3
}

void TIM_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // ʹ��TIM2ʱ��

	TIM_TimeBaseStructure.TIM_Period = (20 - 1);				// �Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					// Ԥ��Ƶ��
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;				// ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);				// ����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); // ѡ��ʱ��2�Ĵ������:�����¼�

	TIM_Cmd(TIM2, ENABLE); // ʹ��TIM2
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // ʹ��GPIOAʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5; // PA4,PA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		   // ģ��
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   // IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				   // ����GPIO_InitStruct��ָ���Ĳ�����ʼ��GPIOx��ÿ��ͨ��
}

void DAC_Configuration(void)
{
	DAC_InitTypeDef DAC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE); // ʹ��DACʱ��

	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;							 // ������ʽ��TIM2����
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;					 // ���η�����ʽ,�޲��η���
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_4095; // ���ǲ�����
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;					 // DAC�������ʹ��
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);									 // ��ʼ��DACͨ��1
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);									 // ��ʼ��DACͨ��2

	DAC_Cmd(DAC_Channel_1, ENABLE); // ʹ��DACͨ��1
	DAC_Cmd(DAC_Channel_2, ENABLE); // ʹ��DACͨ��2

	DAC_DMACmd(DAC_Channel_1, ENABLE); // ʹ��DACͨ��1 DMA
	DAC_DMACmd(DAC_Channel_2, ENABLE); // ʹ��DACͨ��2 DMA
}

void UART_Configuration(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART_TEST, &USART_InitStructure);
	USART_Cmd(USART_TEST, ENABLE);
}

int SER_PutChar(int ch)
{
	while (!USART_GetFlagStatus(USART_TEST, USART_FLAG_TC))
		;
	USART_SendData(USART_TEST, (uint8_t)ch);

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
