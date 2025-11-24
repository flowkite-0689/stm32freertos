#ifndef  SPI_H
#define  SPI_H

#include "stm32f4xx.h"
#include "sys.h"

#define SPI_NSS_H 	PBout(14) = 1
#define SPI_NSS_L 	PBout(14) = 0
#define W25X_Dummy  0xFF

// W25Q128 参数定义
#define W25Q128_CAPACITY     0x1000000   // 16MB = 128Mbit
#define W25Q128_PAGE_SIZE    256         // 页大小256字节
#define W25Q128_SECTOR_SIZE  4096        // 扇区大小4KB

// W25Q128 指令定义
#define W25X_WriteEnable     0x06        // 写使能
#define W25X_WriteDisable    0x04        // 写禁止
#define W25X_ReadStatusReg1  0x05        // 读状态寄存器1
#define W25X_WriteStatusReg1 0x01        // 写状态寄存器1
#define W25X_ReadData        0x03        // 读数据
#define W25X_FastRead        0x0B        // 快速读
#define W25X_PageProgram     0x02        // 页编程
#define W25X_SectorErase     0x20        // 扇区擦除
#define W25X_BlockErase32    0x52        // 32KB块擦除
#define W25X_BlockErase64    0xD8        // 64KB块擦除
#define W25X_ChipErase       0xC7        // 芯片擦除
#define W25X_JedecDeviceID   0x9F        // 读ID指令
#define W25X_JEDECID         0xEF4018    // 芯片ID
#define W25X_PowerDown       0xB9        // 进入低功耗模式
#define W25X_ReleasePowerDown 0xAB       // 退出低功耗模式
#define W25X_ReadStatusReg2  0x35        // 读取状态寄存器2
#define W25X_WriteStatusReg2 0x31        // 写入状态寄存器2

// 错误代码定义
#define W25Q128_RESULT_OK       0
#define W25Q128_RESULT_ERROR    1
#define W25Q128_TIMEOUT_ERROR   2

// 超时定义
#define W25Q128_TIMEOUT_VALUE   1000000

void SPI1_Init(void);
uint8_t SPI1_ReadWriteByte(uint8_t txData);
void SPI1_WriteBytes(uint8_t *pData, uint16_t size);
void SPI1_ReadBytes(uint8_t *pData, uint16_t size);
uint32_t W25Q128_ReadID(void);
uint8_t W25Q128_WaitForWriteEnd(void);
void W25Q128_WriteEnable(void);
uint8_t W25Q128_SectorErase(uint32_t SectorAddr);
uint8_t W25Q128_WritePage(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
uint8_t W25Q128_WritePage_Optimized(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
uint8_t W25Q128_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void W25Q128_ReadData(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint8_t W25Q128_IsBusy(void);
void W25Q128_SetHighSpeedMode(void);


#endif
