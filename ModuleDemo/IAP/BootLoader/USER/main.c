#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "common.h"
//IAP����ʾ����ʹ�ô���1��������115200
//����bootloader�������ڽ��������ļ���������ɺ���ת��IAP����
//IAP������0x08000000��ַ����СΪ12K
//�����ļ���0x08003000��ַ����СΪ128K-12K=116K
//�����ļ��ĸ�ʽΪymodemЭ��
//ymodem���Թ����Ƽ���https://github.com/kingToolbox/WindTerm
//�����ļ��Ĵ�С���ܳ���115K����ΪIAP����ռ�������1Kд������־

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
int main(void)
{
	FLASH_Unlock();//����FLASH
	UART_Configuration(115200);//���ڳ�ʼ��
	IAP();//��ת��IAP����
	while (1)
	{
	}
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
