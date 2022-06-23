#include "st7735v.h" 
#include "delay.h"

u8 SPI_WriteByte(u8 TxData) //TxData ������8λ��16λ�ģ�������SPI֮ǰ��ȷ��������֡��ʽ
{		
	u8 retry = 0;				 	      
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //0�����ͻ���ǿ�  �ȴ����ͻ��������
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(SPI1, TxData);
	retry=0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//�ȴ������������
	{
		retry++;
		if(retry>200)return 0;
	}	  						    
	return SPI_I2S_ReceiveData(SPI1); //����������յ����ݣ�SPI_DR�Ĵ��������					    
}

//��Ҫ�ֶ�����cs
void lcd_write_cmd(const uint8_t cmd)
{
	SCREEN_DC_0;
	SPI_WriteByte(cmd);
	SCREEN_DC_1;
}
//��Ҫ�ֶ�����cs
void lcd_write_data(const uint8_t cmd)
{
	SPI_WriteByte(cmd);
}

int luat_lcd_set_address(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2) {
    lcd_write_cmd(0x2a);
    lcd_write_data((x1+LCD_OFFSET_X)>>8);
    lcd_write_data(x1+LCD_OFFSET_X);
    lcd_write_data((x2+LCD_OFFSET_X)>>8);
    lcd_write_data(x2+LCD_OFFSET_X);
    lcd_write_cmd(0x2b);
    lcd_write_data((y1+LCD_OFFSET_Y)>>8);
    lcd_write_data(y1+LCD_OFFSET_Y);
    lcd_write_data((y2+LCD_OFFSET_Y)>>8);
    lcd_write_data(y2+LCD_OFFSET_Y);
    lcd_write_cmd(0x2C);
    return 0;
}

int luat_lcd_fill(uint16_t color) {
	uint32_t i;
	uint8_t c1 = *((uint8_t*)&color), c2 = *((uint8_t*)&color + 1);
	SCREEN_CS_0;
    luat_lcd_set_address(0,0,LCD_W-1,LCD_H-1);
	for(i = 0; i < LCD_W * LCD_H; i++)
	{
		lcd_write_data(c1);
		lcd_write_data(c2);
	}
	SCREEN_CS_1;
    return 0;
}

void SPI_InitTest(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORTʱ��ʹ�� 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1,  ENABLE );//SPIʱ��ʹ�� 	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //����������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO

	GPIO_SetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//����ͬ��ʱ�ӵĵ�һ�������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 0;
	SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

	SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����
}   
//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   
//SPI_BaudRatePrescaler_8   8��Ƶ   
//SPI_BaudRatePrescaler_16  16��Ƶ  
//SPI_BaudRatePrescaler_256 256��Ƶ 
void SPI_SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI1->CR1&=0XFFC7;
	SPI1->CR1|=SPI_BaudRatePrescaler;
	SPI_Cmd(SPI1,ENABLE); 
}


//��Ļ��ʼ��
void ST7735V_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6);

	SCREEN_CS_1;
	SPI_InitTest();		   	//��ʼ��SPI
	SPI_SetSpeed(SPI_BaudRatePrescaler_8);

	SCREEN_BL_0;

	SCREEN_DC_1;
	SCREEN_RST_0;
	Delay_Ms(100);
	SCREEN_RST_1;
	Delay_Ms(120);

	SCREEN_CS_0;
    lcd_write_cmd(0x11);

    Delay_Ms(120);//ms

    lcd_write_cmd(0x21);//luat_lcd_inv_off

    lcd_write_cmd(0xB1);
    lcd_write_data(0x05);
    lcd_write_data(0x3A);
    lcd_write_data(0x3A);

    lcd_write_cmd(0xB2);
    lcd_write_data(0x05);
    lcd_write_data(0x3A);
    lcd_write_data(0x3A);

    lcd_write_cmd(0xB3);
    lcd_write_data(0x05);
    lcd_write_data(0x3A);
    lcd_write_data(0x3A);
    lcd_write_data(0x05);
    lcd_write_data(0x3A);
    lcd_write_data(0x3A);

    lcd_write_cmd(0xB4);//Dotinversion
    lcd_write_data(0x03);

    lcd_write_cmd(0xC0);
    lcd_write_data(0x62);
    lcd_write_data(0x02);
    lcd_write_data(0x04);

    lcd_write_cmd(0xC1);
    lcd_write_data(0xC0);

    lcd_write_cmd(0xC2);
    lcd_write_data(0x0D);
    lcd_write_data(0x00);

    lcd_write_cmd(0xC3);
    lcd_write_data(0x8D);
    lcd_write_data(0x6A);

    lcd_write_cmd(0xC4);
    lcd_write_data(0x8D);
    lcd_write_data(0xEE);

    lcd_write_cmd(0xC5);//VCOM
    lcd_write_data(0x0E);

    lcd_write_cmd(0xE0);
    lcd_write_data(0x10);
    lcd_write_data(0x0E);
    lcd_write_data(0x02);
    lcd_write_data(0x03);
    lcd_write_data(0x0E);
    lcd_write_data(0x07);
    lcd_write_data(0x02);
    lcd_write_data(0x07);
    lcd_write_data(0x0A);
    lcd_write_data(0x12);
    lcd_write_data(0x27);
    lcd_write_data(0x37);
    lcd_write_data(0x00);
    lcd_write_data(0x0D);
    lcd_write_data(0x0E);
    lcd_write_data(0x10);

    lcd_write_cmd(0xE1);
    lcd_write_data(0x10);
    lcd_write_data(0x0E);
    lcd_write_data(0x03);
    lcd_write_data(0x03);
    lcd_write_data(0x0F);
    lcd_write_data(0x06);
    lcd_write_data(0x02);
    lcd_write_data(0x08);
    lcd_write_data(0x0A);
    lcd_write_data(0x13);
    lcd_write_data(0x26);
    lcd_write_data(0x36);
    lcd_write_data(0x00);
    lcd_write_data(0x0D);
    lcd_write_data(0x0E);
    lcd_write_data(0x10);

    lcd_write_cmd(0x3A);
    lcd_write_data(0x05);

	//direction
    lcd_write_cmd(0x36);
    lcd_write_data(0x78);

    lcd_write_cmd(0x29);

    Delay_Ms(100);

	SCREEN_CS_1;

    luat_lcd_fill(0x00);
	SCREEN_BL_1;
}  



























