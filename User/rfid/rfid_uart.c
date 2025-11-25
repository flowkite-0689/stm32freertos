#include "rfid_uart.h"

static uint32_t rx_flag = 0; // 第17位：1:收到数据，0未收到数据。低16位：接收到的数据项个数
static uint16_t dma_len = 0; // dma一共传输数据项个数

void USART2_IRQHandler(void)
{
    uint16_t len = 0; // dma实际搬了len个数据
    // 判断接收标志位是否置1
    if (USART_GetITStatus(USART2, USART_IT_IDLE) == SET)
    {
        // 正确的IDLE中断清除方式：先读SR，再读DR
        volatile uint16_t temp;
        temp = USART2->SR; // 读状态寄存器
        temp = USART2->DR; // 读数据寄存器 - 这是清除IDLE标志的关键！

        // 暂停DMA
        DMA_Cmd(DMA1_Stream5, DISABLE);

        // DMA_GetCurrDataCounter(DMA1_Stream5) 剩余多少未搬数据项
        len = dma_len - DMA_GetCurrDataCounter(DMA1_Stream5); // 接收了多少个数据项
        len = len ? len : dma_len;                            // 如果len=0，证明接收的数据是刚好存满到数组里面

        // 重置DMA接收
        DMA_SetCurrDataCounter(DMA1_Stream5, dma_len);                                                                  // 重置数据项
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5 | DMA_IT_HTIF5 | DMA_IT_TEIF5 | DMA_IT_DMEIF5 | DMA_IT_FEIF5); // 清除所有可能的中断标志(如果开启dma中断）
        DMA_Cmd(DMA1_Stream5, ENABLE);

        rx_flag = (1 << 17) | len;
    }
}

// PA2-UART2_TX, PA3-UART2_RX
static void RFID_Uart2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1）使能RX和TX引脚GPIO时钟和USART时钟；
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // 2）初始化GPIO，并将GPIO复用到USART上
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;     // 复用模式
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed; // 高速
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;   // 输出模式时有用
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    // 3）配置USART参数；
    USART_InitStruct.USART_BaudRate = 9600;                                      // 波特率
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;                     // 8位有效数据位
    USART_InitStruct.USART_StopBits = USART_StopBits_1;                          // 1位停止位
    USART_InitStruct.USART_Parity = USART_Parity_No;                             // 无校验
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 接收和发送数据模式
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件控制流
    USART_Init(USART2, &USART_InitStruct);

    // 4）配置中断控制器并使能USART空闲中断
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

    // 5）设置中断优先级（如果需要开启串口中断才需要这个步骤）
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn; // 中断通道(中断源)
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 6）使能USART；
    USART_Cmd(USART2, ENABLE);
}

// usartt2_rx 对应的是DMA1，通道4，数据流5, rx_buffer内存地址，len数据项个数
void RFID_Uart2_Rx_DMA_Init(uint8_t *rx_buffer, uint16_t len)
{
    DMA_InitTypeDef DMA_InitStruct;
    dma_len = len;
    // 1）DMA时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    // 2）启用并配置连接到DMA的外设
    RFID_Uart2_Init();

    // 3）DMA参数配置
    DMA_StructInit(&DMA_InitStruct); // 初始化DMA配置结构体为默认值(可选但推荐)

    DMA_InitStruct.DMA_Channel = DMA_Channel_4;                          // 通道4
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;       // 外设地址
    DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)rx_buffer;            // 内存地址
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;                 // 外设到内存
    DMA_InitStruct.DMA_BufferSize = len;                                 // 传输外设的数据项个数
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // 外设地址不递增
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // 内存地址递增
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据宽度8位
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // 内存数据宽度8位
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;                           // 正常模式只接收一次
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;                   // 优先级中
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;                  // 禁用fifo（这里数据宽度一样，因此禁用）
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;       // fifo阈值
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;             // 突发：单次传输
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;     // 突发：单次传输
    DMA_Init(DMA1_Stream5, &DMA_InitStruct);                             // DMA参数配置

    // 6）使能DMA
   DMA_Cmd(DMA1_Stream5, ENABLE); // 开启dma接收数据

    // 7）激活外设所需的流请求（内部SRAM / FLASH存储器除外）
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
}

// 通过串口2，单片机发送指定长度数据
void RFID_Uart2_SendBytes(uint8_t *data, uint16_t len)
{
    while (len--)
    {
        USART_SendData(USART2, *data++); // 库函数
        // 等待发送数据寄存器空
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
            ;
    }
    // 等待发送完成
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
        ;
}

//  判断是否接收完数据，返回成功接收数据个数
uint16_t RFID_Uart2_Recv_Len(void)
{
    uint16_t ret = 0;
    if (rx_flag & (1 << 17))
    {
        ret = rx_flag & 0xFFFF; // 低16位保存接收数据个数
        rx_flag = 0;            // 记录下次接收是否完毕
        return ret;
    }
    else
    {
        return 0;
    }
}
