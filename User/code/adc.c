#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
#include "code/delay.h"

/**
  * @brief  ADC初始化函数
  * @param  无
  * @retval 无
  */
void ADC1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    
    // 1）使能时钟 
    // 使能GPIOA时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    // 使能ADC1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    
    // 2）配置PA5引脚为模拟输入模式 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;           // 选择引脚5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;        // 模拟模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // 无上下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
   // 3）配置ADC公共参数 
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;        // 独立模式
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;     // ADC时钟=84MHz/4=21MHz
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; // 禁用DMA
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 采样延迟
    ADC_CommonInit(&ADC_CommonInitStructure);
    
   // 4）配置ADC1参数
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;          // 12位分辨率
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                   // 禁用扫描模式（单通道）
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;             // 禁用连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;          // 数据右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;                      // 1个转换通道
    ADC_Init(ADC1, &ADC_InitStructure);
    
    // 5）配置ADC通道
    // 配置通道5（对应PA5），第1个转换，采样时间为56个周期，一次采样时间为(56 + 12 )*1/21MHz = 3.3us
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_56Cycles);
    
    // 6）使能ADC
    ADC_Cmd(ADC1, ENABLE);
}

/**
  * @brief  获取电压值（单位：伏特）
  * @param  adcValue: ADC原始值
  * @param  vref: 参考电压（通常为3.3V）
  * @retval 计算出的电压值
  */
float ADC_ConvertToVoltage(float vref)
{
    // 7）软件启动ADC转换 */
    ADC_SoftwareStartConv(ADC1);
    
    // 等待转换完成
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
   // 8）读取ADC值(12位值，0-4095)
    uint16_t adcValue = ADC_GetConversionValue(ADC1);

    // 9）计算电压值
    // 公式：电压 = (ADC值 / 4095) * 参考电压
    return ((float)adcValue / 4095.0f) * vref;
}


/**
  * @brief  ADC3 初始化函数（用于 PF7/ADC3_IN5）
  * @param  无
  * @retval 无
  */
void ADC3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    
    // 1）使能时钟 
    // 使能 GPIOF 时钟（PF7 属于 GPIOF）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    // 使能 ADC3 时钟 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);  // ←  ADC3
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;            // ← Pin_7 (PF7)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;         // 模拟模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // 无上下拉
    GPIO_Init(GPIOF, &GPIO_InitStructure);               // ← GPIOF
    
   
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;     // 84MHz / 4 = 21MHz
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);
    
    // 4）配置 ADC3 参数 
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;        // 单通道
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  // 单次转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC3, &ADC_InitStructure);                  // ← ADC3
    
    // 5）配置 ADC3 通道：通道5（PF7），第1个转换，采样时间 
    ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_56Cycles);  // ← ADC3, ADC_Channel_5
    
    // 6）使能 ADC3 
    ADC_Cmd(ADC3, ENABLE);
}

/**
  * @brief  测试读取 ADC3 不同通道的电压值
  * @param  channel: ADC通道 (0-15)
  * @param  vref: 参考电压（如 3.3f）
  * @retval 电压值（单位：V）
  */
float ADC3_ReadChannelVoltage(uint8_t channel, float vref)
{
    uint32_t timeout = 1000000; // 超时计数器
    
    // 清除之前的配置
    ADC_RegularChannelConfig(ADC3, channel, 1, ADC_SampleTime_56Cycles);
    
    // 等待一点时间让配置生效
    delay_ms(1);
    
    // 软件启动转换
    ADC_SoftwareStartConv(ADC3);
    
    // 等待转换完成，添加超时保护
    while(ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET && timeout--);
    
    if(timeout == 0) {
        return -1.0f; // 超时，返回-1表示错误
    }
    
    // 读取转换结果
    uint16_t adcValue = ADC_GetConversionValue(ADC3);
    
    // 计算电压
    return ((float)adcValue / 4095.0f) * vref;
}

/**
  * @brief  读取 ADC3 电压值（PF7 - 通道5）
  * @param  vref: 参考电压（如 3.3f）
  * @retval 电压值（单位：V）
  */
float ADC3_ReadVoltage(float vref)
{
    return ADC3_ReadChannelVoltage(ADC_Channel_5, vref);
}