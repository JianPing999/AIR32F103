#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf
#define TIM1_CCR1_Address ((uint32_t)0x40012C34)
USART_TypeDef *USART_TEST = USART1;
TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;
uint16_t SRC_Buffer[3] = {0, 0, 0};
uint16_t TimerPeriod = 0;

void RCC_Configuration(void);
void GPIO_Configuration(void);
void DMA_Configuration(void);
void TIM_Configuration(void);
void UART_Configuration(uint32_t bound);
// TIM_PWM_DMA ��demo��ʹ��DMA�������ݣ�ʵ��TIM��PWM�����ʹ��TIM1_CH1���PWM���Σ�PA8��PB13�������PWM���Σ�Ƶ��Ϊ16KHz��ռ�ձȷֱ�Ϊ50%��37.5%��25%
RCC_ClocksTypeDef clocks;
int main(void)
{

	Delay_Init();				// ��ʼ����ʱ����
	UART_Configuration(115200); // ��ʼ������
	RCC_GetClocksFreq(&clocks); // ��ȡϵͳʱ��Ƶ��

	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 TIM PWM DMA Test.\n");

	RCC_Configuration(); // ��ʼ��ʱ��

	GPIO_Configuration(); // ��ʼ��GPIO

	DMA_Configuration();													// ��ʼ��DMA
	TimerPeriod = (SystemCoreClock / 16000) - 1;							// ��ʱ������Ϊ16KHz
	SRC_Buffer[0] = (uint16_t)(((uint32_t)5 * (TimerPeriod - 1)) / 10);		// ռ�ձ�Ϊ50%
	SRC_Buffer[1] = (uint16_t)(((uint32_t)375 * (TimerPeriod - 1)) / 1000); // ռ�ձ�Ϊ37.5%
	SRC_Buffer[2] = (uint16_t)(((uint32_t)25 * (TimerPeriod - 1)) / 100);	// ռ�ձ�Ϊ25%
	TIM_Configuration();													// ��ʼ��TIM
	while (1)
		;
}
//TIM���ã�ʹ��TIM1_CH1���PWM���Σ�PA8��PB13�������PWM���Σ�Ƶ��Ϊ16KHz
void TIM_Configuration(void)
{
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					// Ԥ��Ƶ��
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;				// �Զ���װ��ֵ����ʱ������Ϊ17570Hz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				// ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 2;			// �ظ�������

	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); // ��ʼ����ʱ��1

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;				// ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	// �Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; // �������ʹ��
	TIM_OCInitStructure.TIM_Pulse = SRC_Buffer[0];					// ���ô�װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;		// �������:TIM����Ƚϼ��Ե�
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;		// �����������:TIM����Ƚϼ��Ե�
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;		// ����״̬�µ������ƽ
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;	// ����״̬�µĻ��������ƽ

	TIM_OC1Init(TIM1, &TIM_OCInitStructure); // ��ʼ��TIM1 Channel1 PWMģʽ
	TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE); // ʹ��ָ����TIM1 DMA����

	TIM_Cmd(TIM1, ENABLE); // ʹ��TIM1

	TIM_CtrlPWMOutputs(TIM1, ENABLE); // MOE �����ʹ��
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
void RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA |
							   RCC_APB2Periph_GPIOB,
						   ENABLE);					   // ʹ��TIM1��GPIOA��GPIOBʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // ʹ��DMA1ʱ��
}
// GPIO���ã�TIM1_CH1(PA8)��TIM1_CH1N(PB13)�����������
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;		  // TIM1_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  // �����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  // ��ʼ��GPIOA.8

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; // TIM1_CH1N
	GPIO_Init(GPIOB, &GPIO_InitStructure);	   // ��ʼ��GPIOB.13
}
// DMA���ã�DMA1ͨ��5�������ַΪTIM1_CCR1���ڴ��ַΪSRC_Buffer�����ݴ��䷽��Ϊ�ڴ浽����
void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	DMA_DeInit(DMA1_Channel5);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)TIM1_CCR1_Address;		// �����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SRC_Buffer;				// �ڴ��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;							// ���ݴ��䷽�򣬴��ڴ浽����
	DMA_InitStructure.DMA_BufferSize = 3;										// DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			// �����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						// �ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // ���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			// ���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								// ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;							// DMAͨ�� xӵ�и����ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								// DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��

	DMA_Init(DMA1_Channel5, &DMA_InitStructure); // ����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��

	DMA_Cmd(DMA1_Channel5, ENABLE); // ʹ��DMA1ͨ��5
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
