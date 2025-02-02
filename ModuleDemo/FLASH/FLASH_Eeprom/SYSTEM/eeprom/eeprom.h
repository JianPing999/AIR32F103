#ifndef __EEPROM_H__
#define __EEPROM_H__
#include "air32f10x.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//用户根据自己的需要设置
#define air_FLASH_SIZE 512 	 		//所选mh的FLASH容量大小(单位为K)
#define air_FLASH_WREN 1              //使能FLASH写入(0，不是能;1，使能)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define air_FLASH_BASE 0x08000000 	//mh FLASH的起始地址
//FLASH解锁键值
 

u16 AIRFLASH_ReadHalfWord(u32 faddr);		  //读出半字  
void AIRFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//指定地址开始写入指定长度的数据
u32 AIRFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//指定地址开始读取指定长度数据
void AIRFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//从指定地址开始写入指定长度的数据
void AIRFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//从指定地址开始读出指定长度的数据

//测试写入
void Test_Write(u32 WriteAddr,u16 WriteData);								   
#endif

















