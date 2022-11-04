#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

#define GPIO_GROUP_TEST GPIOA
#define GPIO_MODE_TEST GPIO_Mode_IPU
#define GPIO_SPEED_TEST GPIO_Speed_50MHz
#define GPIO_PIN1_TEST GPIO_Pin_1

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void GPIO_Configuration(void);
/********************************************************************************/
// GPIO�ж�������ԣ���־ͨ������1���ͣ�������Ϊ115200��PA2Ϊ�ж����룬�½��ش���
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;
	Delay_Init();				//��ʱ��ʼ��
	UART_Configuration(115200); //Ĭ��Ϊ����1��������115200
	RCC_GetClocksFreq(&clocks); //��ȡϵͳʱ��Ƶ��

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);
	PRINTF_LOG("AIR32F103 GPIO IO Input Tset.\n");

	GPIO_Configuration();

	while (1)
	{
	}
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //ʹ��GPIOAʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN1_TEST;
	GPIO_InitStructure.GPIO_Speed = GPIO_SPEED_TEST;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_TEST;
	GPIO_Init(GPIO_GROUP_TEST, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1); // GPIOA1Ϊ�ⲿ�ж�1�Ĵ���Դ

	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;		  // EXTI0�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  //����ָ���Ĳ�����ʼ��VIC�Ĵ���

	EXTI_InitStructure.EXTI_Line = EXTI_Line1;				// EXTI1�ж���
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// EXTI1�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				//ʹ��EXTI1�ж�
	EXTI_Init(&EXTI_InitStructure);							//����ָ���Ĳ�����ʼ��EXTI�Ĵ���
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

// EXTI1�жϷ������
void EXTI1_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line1) == SET) //���EXTI1�ϵ��ж�
	{
		Delay_Ms(10);
		if (GPIO_ReadOutputDataBit(GPIOA, GPIO_PIN1_TEST) == Bit_SET) //���GPIOA�ϵ�GPIO_Pin_1���ŵĵ�ƽ
		{
			PRINTF_LOG("The key is pressed\n");
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line1); //���EXTI1�ϵ��жϱ�־λ
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
