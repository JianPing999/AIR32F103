#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

#define BUFF_SIZE 256
#define FLAG_TIMEOUT ((uint32_t)0x1000)
#define LONG_TIMEOUT ((uint32_t)(10 * FLAG_TIMEOUT))
void UART_Configuration(uint32_t bound);
void NVIC_Configuration(void);
void IIC_Configuration(void);
uint32_t IIC_MasterTest(void);
uint32_t IIC_SlaveTest(void);
void FillData(void);
void DataPrintf(void *buf, uint32_t bufsize);
uint32_t TIMEOUT_UserCallback(void);
uint8_t GetCmd(void);

USART_TypeDef *USART_TEST = USART1;

uint8_t SendBuff[BUFF_SIZE];
uint8_t RecvBuff[BUFF_SIZE];
uint8_t RecvFlag = 0;
__IO uint32_t Timeout = LONG_TIMEOUT; //��ʱʱ��
/********************************************************************************/
// I2C����ģʽ�ӻ�ģʽʾ������־ͨ������1���ͣ�������Ϊ115200��������ʾ������������I2C����ģʽ��ӻ�ģʽ
/********************************************************************************/
int main(void)
{
	uint8_t cmd;
	RCC_ClocksTypeDef clocks;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE); //ʹ��CRCʱ��
	Delay_Init();									  //��ʱ��ʼ��
	UART_Configuration(115200);						  //Ĭ�ϴ���1��������115200
	RCC_GetClocksFreq(&clocks);						  //��ȡϵͳʱ��Ƶ��

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 IIC Transmit.\n");

	PRINTF_LOG("m: IIC master polling send\n");
	PRINTF_LOG("r: IIC slave int receive\n");

	IIC_Configuration(); // IIC����

	while (1)
	{
		cmd = GetCmd();
		switch (cmd)
		{
		case 'm':
			PRINTF_LOG("IIC polling master send data:\n");
			IIC_MasterTest(); // IIC��������
			break;

		case 'r':
			PRINTF_LOG("IIC slave receive data...\n");
			NVIC_Configuration(); // IIC�ӻ��жϽ���
			IIC_SlaveTest();	  // IIC�ӻ�����
			break;
		}
	}
}
uint32_t TIMEOUT_UserCallback(void)
{
	// I2C��ʱ������������Զ��崦��ʽ�������·������һ�����ݵȣ�����ֻ�Ǽ򵥵Ĵ�ӡ��ʱ��Ϣ��Ȼ�������ȴ�
	PRINTF_LOG("IIC TIMEOUT\n");
	while (1)
	{
	}
}
void IIC_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);					   //ʹ��GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_I2C2, ENABLE); //ʹ��I2C1,I2C2ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);					   //ʹ�� AFIO ʱ��
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);							   //������ӳ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; // PB8,PB9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   //�ٶ�50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;		   //���ÿ�©���
	GPIO_Init(GPIOB, &GPIO_InitStructure);				   //��ʼ��

	GPIO_ForcePuPdCmd(GPIOB, ENABLE);		   //�����ڲ�����
	GPIO_ForcePullUpConfig(GPIOB, GPIO_Pin_8); // PB8����
	GPIO_ForcePullUpConfig(GPIOB, GPIO_Pin_9); // PB9����

	I2C_DeInit(I2C1);														  //��λI2C1
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;								  // I2Cģʽ
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;						  //ָ��ʱ��ռ�ձȣ���ѡ low/high = 2:1 �� 16:9 ģʽ
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // 7λ��ַ
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;								  //����I2C�豸��ַ
	I2C_InitStructure.I2C_ClockSpeed = 100000;								  // 100kHz
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;								  //ʹ��Ӧ��

	I2C_Init(I2C1, &I2C_InitStructure); //��ʼ��I2C1
	I2C_Cmd(I2C1, ENABLE);				//ʹ��I2C1
}

uint32_t IIC_MasterTest(void)
{
	uint32_t i = 0, j;

	FillData(); //�������
	j = BUFF_SIZE;
	DataPrintf(SendBuff, BUFF_SIZE);

	I2C_GenerateSTART(I2C1, ENABLE); //���Ϳ�ʼ�ź�
	Timeout = FLAG_TIMEOUT;
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if ((Timeout--) == 0)
			return TIMEOUT_UserCallback();
	} //�ȴ����������豸��ַ

	I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter); //�����豸��ַ
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if ((Timeout--) == 0)
			return TIMEOUT_UserCallback();
	} //�ȴ��豸��ַ�������

	while (j--)
	{
		I2C_SendData(I2C1, SendBuff[i]); //��������
		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			if ((Timeout--) == 0)
				return TIMEOUT_UserCallback();
		} //�ȴ����ݷ������
		i++;
	}

	I2C_GenerateSTOP(I2C1, ENABLE); //����ֹͣ�ź�
	while ((I2C1->CR1 & 0x200) == 0x200)
	{
		if ((Timeout--) == 0)
			return TIMEOUT_UserCallback();
	} //�ȴ�ֹͣ�źŷ������
	return 0;
}

uint32_t IIC_SlaveTest(void)
{
	FillData();											 //�������
	I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, ENABLE); //�����ж�
	I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);				 //�����ж�
	while (1)
	{
		if (RecvFlag == 1) //�������
		{
			DataPrintf(RecvBuff, BUFF_SIZE);				//��ӡ����
			if (memcmp(RecvBuff, SendBuff, BUFF_SIZE) == 0) //�Ƚ�����
			{
				PRINTF_LOG("IIC slave int receive data success\n");
			}
			memset(RecvBuff, 0, BUFF_SIZE); //�������
			RecvFlag = 0;
		}
	}
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;		  // I2C1�¼��ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  //�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //ʹ���ж�
	NVIC_Init(&NVIC_InitStructure);							  //��ʼ��

	NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;		  // I2C1�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //�����ȼ�
	NVIC_Init(&NVIC_InitStructure);							  //��ʼ��
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
//��ӡ����
void DataPrintf(void *buf, uint32_t bufsize)
{
	uint32_t i = 0;
	uint8_t *pBuf = (uint8_t *)buf;

	if (0 != bufsize)
	{
		for (i = 0; i < bufsize; i++)
		{
			if (0 != i && 0 == i % 16)
			{
				PRINTF_LOG(" \n");
			}

			PRINTF_LOG("%02X ", pBuf[i]);
		}
	}
	PRINTF_LOG("\n");
}
//�������
void FillData(void)
{
	uint32_t i = 0;
	for (i = 0; i < BUFF_SIZE; i++)
	{
		SendBuff[i] = 0x01 + i;
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
