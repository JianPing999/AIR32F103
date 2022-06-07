#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void PVD_Configuration(void);
/********************************************************************************/
// PVD������ʾ������־ͨ������1���ͣ�������Ϊ115200������ѹ����2.9���ߵ���2.9Vʱ���ᴥ��PVD�ж�
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;

	Delay_Init();				//��ʱ��ʼ��
	UART_Configuration(115200); //Ĭ�ϴ���1��������115200
	RCC_GetClocksFreq(&clocks); //��ȡϵͳʱ��Ƶ��

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 PWR PVD.\n");

	PVD_Configuration(); // PVD����
	while (1)
		;
}

void PVD_Configuration(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); //ʹ��PWR��BKPʱ��

	EXTI_ClearITPendingBit(EXTI_Line16);						   //���EXTI��16���жϱ�־
	EXTI_InitStructure.EXTI_Line = EXTI_Line16;					   // EXTI��16
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			   //�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //�����غ��½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					   //ʹ��EXTI��
	EXTI_Init(&EXTI_InitStructure);								   //��ʼ��EXTI��

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); //����NVIC�����ȼ�����Ϊ1

	/* Enable the PVD Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;			  // PVD�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //ʹ��PVD�ж�
	NVIC_Init(&NVIC_InitStructure);							  //��ʼ��PVD�ж�

	PWR_PVDLevelConfig(PWR_PVDLevel_2V9); //����PVD��ѹΪ2.9V
	PWR_PVDCmd(ENABLE);					  //ʹ��PVD
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

void PVD_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line16) != RESET) // PVD�ж�
	{
		if (PWR_GetFlagStatus(PWR_FLAG_PVDO) == SET) // �ж��ж�ԭ��
		{
			PRINTF_LOG("VDD ����ѡ����PVD��ֵ\n");
		}
		else
		{
			PRINTF_LOG("VDD ����ѡ����PVD��ֵ\n");
		}

		EXTI_ClearITPendingBit(EXTI_Line16); //���PVD�жϱ�־
	}
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
