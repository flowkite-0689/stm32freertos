#ifndef  SPI_H
#define  SPI_H

#include "stm32f4xx.h"
#include "sys.h"

#define SPI_NSS_H 	PBout(14) = 1
#define SPI_NSS_L 	PBout(14) = 0
#define W25X_Dummy  0xFF

// W25Q128 常量定义
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
#define W25X_JedecDeviceID   0x9F        // 读ID的指令
#define W25X_JEDECID         0xEF4018    // 芯片ID




void SPI1_Init(void);
uint8_t SPI1_ReadWriteByte(uint8_t txData);
uint32_t W25Q128_ReadID(void);
void W25Q128_WaitForWriteEnd(void);
void W25Q128_WriteEnable(void);
void W25Q128_SectorErase(uint32_t SectorAddr);
void W25Q128_WritePage(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void W25Q128_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void W25Q128_ReadData(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);

#endif

