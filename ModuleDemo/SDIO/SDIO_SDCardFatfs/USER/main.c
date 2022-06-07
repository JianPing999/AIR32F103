#include "air32f10x.h"
#include <stdio.h>
#include "bsp_sdio_sdcard.h"
#include "sdio_test.h"
#include "delay.h"
#include "ff.h"

void USART_Config(uint32_t bound);
uint8_t GetCmd(void);
void TestList(void);
void ShowCardInfo(void);
void FatfsTest(void);
void WriteFileTest(void);
void CreateDir(void);
void DeleteDirFile(void);
void ViewRootDir(void);

extern uint32_t CSD_Tab[4], CID_Tab[4];
extern SD_CardInfo SDCardInfo;
/**
����FATFS��Ҫ�ı���
*/
FATFS fs;																 // FATFS�ļ�ϵͳ����
FIL fnew;																 //�ļ�����
FRESULT res_sd;															 //�ļ��������
UINT fnum;																 //�ļ��ɹ���д����
BYTE ReadBuffer[1024] = {0};											 //������
BYTE WriteBuffer[] = "AirM2M �Ϻ�����  ------>  �½��ļ�ϵͳ�����ļ�\n"; //д������

/**
���ڲ��Զ�д�ٶ�
**/
#define TEST_FILE_LEN (2 * 1024 * 1024) //���ڲ��Ե��ļ�����
#define BUF_SIZE (4 * 1024)				//ÿ�ζ�дSD��������ݳ���
uint8_t TestBuf[BUF_SIZE];

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);

int main(void)
{
	uint8_t cmd = 0;
	RCC_ClocksTypeDef clocks;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE); //ʹ��CRCʱ��
	Delay_Init();									  //��ʱ��ʼ��
	UART_Configuration(115200);						  //���ڳ�ʼ��
	RCC_GetClocksFreq(&clocks);						  //��ȡʱ��Ƶ��

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 SDIO SD Card Fatfs Test.\n");
	TestList(); //��ʾ�����б�

	while (1)
	{
		cmd = GetCmd();
		switch (cmd)
		{
		case '1':
		{
			PRINTF_LOG("1.--->>>FatfsTest\r\n");
			FatfsTest(); // FATFS����
			TestList();
			break;
		}
		case '2':
		{
			PRINTF_LOG("2.--->>>ViewRootDir\r\n");
			ViewRootDir(); //�鿴��Ŀ¼
			TestList();
			break;
		}
		case '3':
		{
			PRINTF_LOG("3.--->>>CreateDir\r\n");
			CreateDir(); //����Ŀ¼
			TestList();
			break;
		}
		case '4':
		{
			PRINTF_LOG("4.--->>>DeleteDirFile\r\n");
			DeleteDirFile(); //ɾ��Ŀ¼���ļ�
			TestList();
			break;
		}
		}
	}
}

void TestList(void)
{
	PRINTF_LOG("/***************************SD Card Test*******************************/\n");
	PRINTF_LOG("==========================List==========================\n");
	PRINTF_LOG("1: ����һ�����ļ�(FatFs��д�����ļ�.txt),���ж�д����\n");
	PRINTF_LOG("2: ��ʾSD Card ��Ŀ¼�µ��ļ�����\n");
	PRINTF_LOG("3: ����Ŀ¼(/Dir1,/Dir1/Die1_1,/Dir2)\n");
	PRINTF_LOG("4: ɾ���ļ���Ŀ¼(/Dir1,/Dir1/Dir1_1,/Dir2,FatFs��д�����ļ�.txt)\n");
	PRINTF_LOG("****************************************************************************/\n");
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

void FatfsTest(void)
{
	res_sd = f_mount(&fs, "0:", 1);

	/***************************��ʽ������**************************/
	PRINTF_LOG("\n��ʽ������\n");
	if (res_sd == FR_NO_FILESYSTEM)
	{
		PRINTF_LOG("SD ��û���ļ�ϵͳ, �������и�ʽ��\r\n");

		res_sd = f_mkfs("0:", 0, 0);

		if (res_sd == FR_OK)
		{
			PRINTF_LOG("SD ���ɹ������ļ�ϵͳ\r\n");
			res_sd = f_mount(NULL, "0:", 1);
			res_sd = f_mount(&fs, "0:", 1);
		}
		else
		{
			PRINTF_LOG("SD ����ʽ��ʧ��\r\n");
			while (1)
				;
		}
	}
	else if (res_sd != FR_OK)
	{
		PRINTF_LOG("SD ������ʧ�� (%d), ������SD����ʼ��ʧ��\r\n", res_sd);
		while (1)
			;
	}
	else
	{
		PRINTF_LOG("�ļ�ϵͳ���س�, ���Խ��ж�д����\r\n");
	}

	/***************************�ļ�ϵͳ���� --->>> д����*********************/
	PRINTF_LOG("\n�ļ�ϵͳ�� --->>> д����\n");
	res_sd = f_open(&fnew, "0:FatFs��д�����ļ�.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("��/���� FatFs��д�����ļ�.txt �ɹ�, ���ļ���д������\r\n");

		res_sd = f_write(&fnew, WriteBuffer, sizeof(WriteBuffer), &fnum);
		if (res_sd == FR_OK)
		{
			PRINTF_LOG("�ļ�д��ɹ�, д���ֽ�����%d д�������Ϊ: \n%s\r\n", fnum, WriteBuffer);
		}
		else
		{
			PRINTF_LOG("�ļ�д��ʧ�� (%d)\n", res_sd);
		}
		f_close(&fnew);
	}
	else
	{
		PRINTF_LOG("��/���� �ļ�ʧ��\r\n");
	}

	/*************************�ļ�ϵͳ���� --->>> ������**************************/
	PRINTF_LOG("\n�ļ�ϵͳ�� --->>> ������\n");
	res_sd = f_open(&fnew, "0:FatFs��д�����ļ�.txt", FA_OPEN_ALWAYS | FA_READ);
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("���ļ��ɹ�\r\n");
		res_sd = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum);
		if (res_sd == FR_OK)
		{
			PRINTF_LOG("�ļ���ȡ�ɹ�. ��ȡ���ֽ���:%d ��ȡ������Ϊ: \n%s\r\n", fnum, ReadBuffer);
		}
		else
		{
			PRINTF_LOG("�ļ���ȡʧ�� (%d)\n", res_sd);
		}
	}
	else
	{
		PRINTF_LOG("�ļ���ʧ��\n");
	}

	f_close(&fnew);

	f_mount(NULL, "0:", 1);
}

void CreateDir(void)
{
	res_sd = f_mount(&fs, "0:", 1);
	if (res_sd != FR_OK)
	{
		PRINTF_LOG("�����ļ�ϵͳʧ�� (%d)\r\n", res_sd);
	}

	res_sd = f_mkdir("/Dir1");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("f_mkdir Dir1 OK\r\n");
	}
	else if (res_sd == FR_EXIST)
	{
		PRINTF_LOG("Dir1 Ŀ���Ѵ���(%d)\r\n", res_sd);
	}
	else
	{
		PRINTF_LOG("f_mkdir Dir1 ʧ��(%d)\r\n", res_sd);
		return;
	}

	res_sd = f_mkdir("/Dir2");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("f_mkdir Dir2 OK\r\n");
	}
	else if (res_sd == FR_EXIST)
	{
		PRINTF_LOG("Dir2 Ŀ���Ѵ���(%d)\r\n", res_sd);
	}
	else
	{
		PRINTF_LOG("f_mkdir Dir2 ʧ�� (%d)\r\n", res_sd);
		return;
	}

	res_sd = f_mkdir("/Dir1/Dir1_1");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("f_mkdir Dir1_1 OK\r\n");
	}
	else if (res_sd == FR_EXIST)
	{
		PRINTF_LOG("Dir1_1 Ŀ���Ѵ���(%d)\r\n", res_sd);
	}
	else
	{
		PRINTF_LOG("f_mkdir Dir1_1 ʧ�� (%d)\r\n", res_sd);
		return;
	}

	f_mount(NULL, "0:", 1);
}

void DeleteDirFile(void)
{
	res_sd = f_mount(&fs, "0:", 1);
	if (res_sd != FR_OK)
	{
		PRINTF_LOG("�����ļ�ϵͳʧ�� (%d)\r\n", res_sd);
	}

	res_sd = f_unlink("/Dir1/Dir1_1");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("ɾ����Ŀ¼/Dir1/Dir1_1�ɹ�\r\n");
	}
	else if ((res_sd == FR_NO_FILE) || (res_sd == FR_NO_PATH))
	{
		PRINTF_LOG("û�з����ļ���Ŀ¼: %s\r\n", "/Dir1/Dir1_1");
	}
	else
	{
		PRINTF_LOG("ɾ����Ŀ¼/Dir1/Dir1_1ʧ��(������� = %d) �ļ�ֻ������Ŀ¼�ǿ�\r\n", res_sd);
	}

	res_sd = f_unlink("/Dir1");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("ɾ��Ŀ¼/Dir1�ɹ�\r\n");
	}
	else if ((res_sd == FR_NO_FILE) || (res_sd == FR_NO_PATH))
	{
		PRINTF_LOG("û�з����ļ�����Ŀ¼ : %s\r\n", "/Dir1");
	}
	else
	{
		PRINTF_LOG("ɾ����Ŀ¼/Dir1/Dir1_1ʧ��(������� = %d) �ļ�ֻ������Ŀ¼�ǿ�\r\n", res_sd);
	}

	res_sd = f_unlink("/Dir2");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("ɾ��/Dir2�ɹ�\r\n");
	}
	else if ((res_sd == FR_NO_FILE) || (res_sd == FR_NO_PATH))
	{
		PRINTF_LOG("û�з����ļ���Ŀ¼ : %s\r\n", "/Dir2");
	}
	else
	{
		PRINTF_LOG("ɾ����Ŀ¼/Dir1/Dir1_1ʧ��(������� = %d) �ļ�ֻ������Ŀ¼�ǿ�\r\n", res_sd);
	}

	res_sd = f_unlink("FatFs��д�����ļ�.txt");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("ɾ��FatFs��д�����ļ�.txt�ɹ�\r\n");
	}
	else if ((res_sd == FR_NO_FILE) || (res_sd == FR_NO_PATH))
	{
		PRINTF_LOG("û�з����ļ���Ŀ¼ : %s\r\n", "/FatFs��д�����ļ�.txt");
	}
	else
	{
		PRINTF_LOG("ɾ��FatFs��д�����ļ�.txtʧ��(������� = %d) �ļ�ֻ������Ŀ¼�ǿ�\r\n", res_sd);
	}

	f_mount(NULL, "0:", 1);
}

void ViewRootDir(void)
{
	DIR dirinf;
	FILINFO fileinf;
	uint32_t cnt = 0;
	char name[256];

	res_sd = f_mount(&fs, "0:", 1);
	if (res_sd != FR_OK)
	{
		PRINTF_LOG("�����ļ�ϵͳʧ�� (%d)\r\n", res_sd);
	}

	res_sd = f_opendir(&dirinf, "/");
	if (res_sd != FR_OK)
	{
		PRINTF_LOG("�򿪸�Ŀ¼ʧ�� (%d)\r\n", res_sd);
		return;
	}

	fileinf.lfname = name;
	fileinf.lfsize = 256;

	PRINTF_LOG("����		|	�ļ���С	|	���ļ���	|	���ļ���\r\n");
	for (cnt = 0;; cnt++)
	{
		res_sd = f_readdir(&dirinf, &fileinf);
		if (res_sd != FR_OK || fileinf.fname[0] == 0)
		{
			break;
		}

		if (fileinf.fname[0] == '.')
		{
			continue;
		}

		if (fileinf.fattrib & AM_DIR)
		{
			PRINTF_LOG("(0x%02d)Ŀ¼", fileinf.fattrib);
		}
		else
		{
			PRINTF_LOG("(0x%02d)����", fileinf.fattrib);
		}

		PRINTF_LOG("%10d	", fileinf.fsize);
		PRINTF_LOG("	%s |", fileinf.fname);
		PRINTF_LOG("	%s\r\n", (char *)fileinf.lfname);
	}

	f_mount(NULL, "0:", 1);
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
