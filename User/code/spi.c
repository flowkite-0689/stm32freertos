#include "spi.h"

// 配置SPI参数
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;
	
	// 1) 时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	// 2) 配置引脚
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;        
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;    	// 复用
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed; 	// 高速
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;   	// 推挽
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;   
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;    		// 片选引脚    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;    	// 复用
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed; 	// 高速
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;   	// 推挽
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;   
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
	SPI_NSS_H;
	
	// 3) 配置引脚复用功能
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
	
	// 4) 配置SPI参数 
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	// 双线全双工
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;		// 主模式
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;	// 数据宽度8位
	SPI_InitStruct.SPI_CPOL =SPI_CPOL_Low; 			// SCK空闲低电平.spi模式0
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;		// 第1边沿采集数据
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;			// 软件模式
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // 频率84M/4 = 21MHz (优化后)
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;	// 高位先发
	SPI_InitStruct.SPI_CRCPolynomial = 0x7;			// 暂时没有用硬件CRC,该参数设置了也没用
	SPI_Init(SPI1, &SPI_InitStruct);
	
	// 5) 使能SPI
	SPI_Cmd(SPI1, ENABLE);
}

uint8_t SPI1_ReadWriteByte(uint8_t txData)
{
	// 等待发送缓冲区为空
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	
	// 发送数据
	SPI_I2S_SendData(SPI1, txData);
	
	// 等待接收缓冲区非空
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	
	return SPI_I2S_ReceiveData(SPI1);	// 读取并返回接收的数据
}

// 批量写入数据（优化后的函数）
void SPI1_WriteBytes(uint8_t *pData, uint16_t size)
{
    for(uint16_t i = 0; i < size; i++)
    {
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData(SPI1, pData[i]);
    }
    // 等待最后一个字节传输完成
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
}

// 批量读取数据
void SPI1_ReadBytes(uint8_t *pData, uint16_t size)
{
    for(uint16_t i = 0; i < size; i++)
    {
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData(SPI1, 0xFF); // 发送dummy数据
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
        pData[i] = SPI_I2S_ReceiveData(SPI1);
    }
}

uint32_t W25Q128_ReadID(void)
{
	uint8_t manufacturer_id = 0;
    uint8_t memory_type_id = 0;
    uint8_t capacity_id = 0;
    uint32_t JedecDeviceID = 0;
	
    printf("Reading W25Q128 ID...\r\n");
    SPI_NSS_L;	// 片选
    SPI1_ReadWriteByte(W25X_JedecDeviceID);
    manufacturer_id = SPI1_ReadWriteByte(W25X_Dummy);
    memory_type_id = SPI1_ReadWriteByte(W25X_Dummy);
    capacity_id = SPI1_ReadWriteByte(W25X_Dummy);
    SPI_NSS_H;
    JedecDeviceID = manufacturer_id << 16 | memory_type_id << 8 | capacity_id;
    
    printf("W25Q128 bytes: Mfg=0x%02X, Type=0x%02X, Cap=0x%02X\r\n", 
           manufacturer_id, memory_type_id, capacity_id);
    printf("W25Q128 ID: 0x%06X\r\n", JedecDeviceID);
    
    return JedecDeviceID;
}

// 等待不忙
uint8_t W25Q128_WaitForWriteEnd(void)
{
    SPI_NSS_L;
    uint8_t Temp0 = 0;
    uint32_t timeout = W25Q128_TIMEOUT_VALUE;
    
    SPI1_ReadWriteByte(W25X_ReadStatusReg1);
    do
    {
        Temp0 = SPI1_ReadWriteByte(0xFF);
        timeout--;
    } while ((Temp0 & 0x01) && timeout); // 等待WIP(BUSY)标志位清零
    
    SPI_NSS_H;
    
    if (timeout == 0) {
        return W25Q128_TIMEOUT_ERROR;
    }
    
    return W25Q128_RESULT_OK;
}

// 写使能
void W25Q128_WriteEnable(void)
{
    SPI_NSS_L;
    SPI1_ReadWriteByte(W25X_WriteEnable);
    SPI_NSS_H;
}

// 扇区擦除
uint8_t W25Q128_SectorErase(uint32_t SectorAddr)
{
    uint8_t result;
    
    result = W25Q128_WaitForWriteEnd(); // 等待写操作完成
    if (result != W25Q128_RESULT_OK) {
        return result;
    }
    
    W25Q128_WriteEnable();
    SPI_NSS_L;
    SPI1_ReadWriteByte(W25X_SectorErase);
    SPI1_ReadWriteByte((SectorAddr >> 16) & 0xFF);
    SPI1_ReadWriteByte((SectorAddr >> 8) & 0xFF);
    SPI1_ReadWriteByte(SectorAddr & 0xFF);
    SPI_NSS_H;
    
    result = W25Q128_WaitForWriteEnd(); // 等待写操作完成
    return result;
}

// 页写入
uint8_t W25Q128_WritePage(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t result;
    
    if (pBuffer == NULL || NumByteToWrite == 0 || NumByteToWrite > W25Q128_PAGE_SIZE) {
        return W25Q128_RESULT_ERROR;
    }
    
    result = W25Q128_WaitForWriteEnd(); // 等待写操作完成
    if (result != W25Q128_RESULT_OK) {
        return result;
    }
    
    W25Q128_WriteEnable();
    SPI_NSS_L;
    SPI1_ReadWriteByte(W25X_PageProgram);
    SPI1_ReadWriteByte((WriteAddr >> 16) & 0xFF);
    SPI1_ReadWriteByte((WriteAddr >> 8) & 0xFF);
    SPI1_ReadWriteByte(WriteAddr & 0xFF);
    
    // 使用批量写入优化性能
    SPI1_WriteBytes(pBuffer, NumByteToWrite);
    
    SPI_NSS_H;
    
    result = W25Q128_WaitForWriteEnd(); // 等待写操作完成
    return result;
}

// 优化的页写入函数（更高性能）
uint8_t W25Q128_WritePage_Optimized(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t result;
    uint8_t cmd_addr[4];
    
    if (pBuffer == NULL || NumByteToWrite == 0 || NumByteToWrite > W25Q128_PAGE_SIZE) {
        return W25Q128_RESULT_ERROR;
    }
    
    result = W25Q128_WaitForWriteEnd();
    if (result != W25Q128_RESULT_OK) {
        return result;
    }
    
    W25Q128_WriteEnable();
    
    // 准备命令和地址
    cmd_addr[0] = W25X_PageProgram;
    cmd_addr[1] = (WriteAddr >> 16) & 0xFF;
    cmd_addr[2] = (WriteAddr >> 8) & 0xFF;
    cmd_addr[3] = WriteAddr & 0xFF;
    
    SPI_NSS_L;
    
    // 一次性发送命令和地址
    SPI1_WriteBytes(cmd_addr, 4);
    
    // 批量写入数据
    SPI1_WriteBytes(pBuffer, NumByteToWrite);
    
    SPI_NSS_H;
    
    result = W25Q128_WaitForWriteEnd();
    return result;
}

// 设置W25Q128为高速模式
void W25Q128_SetHighSpeedMode(void)
{
    uint8_t status;
    
    // 读取当前状态寄存器
    SPI_NSS_L;
    SPI1_ReadWriteByte(W25X_ReadStatusReg1);
    status = SPI1_ReadWriteByte(W25X_Dummy);
    SPI_NSS_H;
    
    // 如果已经设置为高速模式，则跳过
    if ((status & 0x01) == 0) {
        return;
    }
    
    // 写使能
    W25Q128_WriteEnable();
    
    // 写入状态寄存器，设置为高速模式
    SPI_NSS_L;
    SPI1_ReadWriteByte(W25X_WriteStatusReg1);
    SPI1_ReadWriteByte(0x00); // 设置为高速模式
    SPI_NSS_H;
    
    // 等待写操作完成
    W25Q128_WaitForWriteEnd();
}

// 任意地址写入，页满则下一页写入，到芯片最后地址则停止写入
uint8_t W25Q128_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t bytes_remaining = NumByteToWrite;
    uint16_t bytes_to_write;
    uint32_t current_addr = WriteAddr;
    uint8_t result;

    if (pBuffer == NULL || NumByteToWrite == 0) // 空指针和零长度检查
    {
        return W25Q128_RESULT_ERROR;
    }

    // 检查写入地址是否超出芯片容量
    if (current_addr >= W25Q128_CAPACITY)
    {
        return W25Q128_RESULT_ERROR; // 起始地址已超出芯片范围
    }

    while (bytes_remaining > 0)
    {
        // 计算当前页内剩余空间
        uint16_t page_offset = current_addr % W25Q128_PAGE_SIZE;   // 页内偏移量
        uint16_t page_remaining = W25Q128_PAGE_SIZE - page_offset; // 当前页剩余空间

        // 确定本次写入的字节数
        bytes_to_write = (bytes_remaining <= page_remaining) ? bytes_remaining : page_remaining;

        // 执行页写入
        result = W25Q128_WritePage(pBuffer, current_addr, bytes_to_write);
        if (result != W25Q128_RESULT_OK) {
            return result;
        }

        // 更新指针和计数器
        pBuffer += bytes_to_write;
        current_addr += bytes_to_write;
        bytes_remaining -= bytes_to_write;

        // 如果已经写到芯片末尾，退出循环
        if (current_addr >= W25Q128_CAPACITY)
        {
            break;
        }
    }
    
    return W25Q128_RESULT_OK;
}

// 读取数据
void W25Q128_ReadData(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    uint16_t bytes_to_read = 0;                                                                                 // 剩余待读取的字节数
    bytes_to_read = NumByteToRead + ReadAddr >= W25Q128_CAPACITY ? W25Q128_CAPACITY - ReadAddr : NumByteToRead; // 读取字节数不能超过芯片容量
    if (pBuffer == NULL || bytes_to_read == 0)                                                                  // 空指针和零长度检查
    {
        return;
    }

    SPI_NSS_L;
    SPI1_ReadWriteByte(W25X_ReadData);
    SPI1_ReadWriteByte((ReadAddr >> 16) & 0xFF);
    SPI1_ReadWriteByte((ReadAddr >> 8) & 0xFF);
    SPI1_ReadWriteByte(ReadAddr & 0xFF);
    for (uint16_t i = 0; i < bytes_to_read; i++)
    {
        pBuffer[i] = SPI1_ReadWriteByte(0xFF);
    }
    SPI_NSS_H;
}

uint8_t W25Q128_IsBusy(void)
{
    uint8_t status = 0;
    uint32_t timeout = W25Q128_TIMEOUT_VALUE;  // 用你定义的超时值

    do {
        SPI_NSS_L;
        SPI1_ReadWriteByte(W25X_ReadStatusReg1);
        status = SPI1_ReadWriteByte(W25X_Dummy);
        SPI_NSS_H;
        if (--timeout == 0) {
            return 1;  // 超时，返回忙（防止死循环）
        }
    } while (status & 0x01);  // 只要 BUSY 位是 1 就继续等

    return 0;  // 不忙了
}
