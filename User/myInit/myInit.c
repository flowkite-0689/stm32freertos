/**
 * @file myInit.c
 * @brief STM32F4xx GPIO初始化及延时函数实现
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.11.8
 */

#include "./myInit.h"

/**
 * @brief 简单的循环延时函数
 * @param count 延时循环次数
 * @note 这是一个简单的软件延时，延时时间取决于CPU频率
 */
void delay(uint32_t count)
{
  while (count--)
    ;
}

/**
 * @brief 毫秒级延时函数
 * @param ms 延时时间（单位：毫秒）
 * @note 基于168MHz系统时钟的简单延时，约1ms=32000个循环周期
 * @warning 此函数为软件延时，精度较低，不适合需要精确计时的场景
 */
void Delay_ms(uint32_t ms)
{
  uint32_t i;
  for (i = 0; i < ms; i++)
  {
    delay(32000); // 168MHz 下大约1ms
  }
}

/**
 * @brief GPIO通用初始化函数
 * @param GPIOx GPIO端口（如GPIOA、GPIOB等）
 * @param GPIO_Pin GPIO引脚（如GPIO_Pin_0、GPIO_Pin_1等）
 * @param GPIO_Mode GPIO模式（输入、输出、复用、模拟）
 * @param GPIO_Speed GPIO速度（2MHz、25MHz、50MHz、100MHz）
 * @param GPIO_OType GPIO输出类型（推挽、开漏）
 * @param GPIO_PuPd GPIO上下拉配置（上拉、下拉、无上下拉）
 * @note 函数会根据传入的GPIOx参数自动使能对应的AHB1时钟
 */
void GPIO_MyInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode,
                 GPIOSpeed_TypeDef GPIO_Speed, GPIOOType_TypeDef GPIO_OType,
                 GPIOPuPd_TypeDef GPIO_PuPd)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // 根据实际使用的GPIO端口使能对应时钟
  if (GPIOx == GPIOA)
  {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  }
  else if (GPIOx == GPIOB)
  {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  }
  else if (GPIOx == GPIOC)
  {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  }
  else if (GPIOx == GPIOD)
  {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  }
  else if (GPIOx == GPIOE)
  {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  }
  else if (GPIOx == GPIOF)
  {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
  }
  else if (GPIOx == GPIOG)
  {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
  }
  else if (GPIOx == GPIOH)
  {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
  }
  else if (GPIOx == GPIOI)
  {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
  }

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode;
  GPIO_InitStructure.GPIO_OType = GPIO_OType;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd;
  GPIO_Init(GPIOx, &GPIO_InitStructure);
}

/**
 * @brief 按键GPIO初始化函数
 * @param KEY_PIN 按键引脚（如GPIO_Pin_0等）
 * @param GPIOx 按键所在GPIO端口（如GPIOA等）
 * @note 配置为高速输入模式，推挽输出类型，无上下拉
 * @note 建议根据实际硬件设计配置上下拉电阻
 */
void KEY_Init(uint32_t KEY_PIN, GPIO_TypeDef *GPIOx)
{
  GPIO_MyInit(GPIOx, KEY_PIN, GPIO_Mode_IN, GPIO_High_Speed,
              GPIO_OType_PP, GPIO_PuPd_NOPULL);
}

/**
 * @brief LED GPIO初始化函数
 * @param LED_PIN LED引脚（如GPIO_Pin_9等）
 * @param GPIOx LED所在GPIO端口（如GPIOF等）
 * @note 配置为高速输出模式，推挽输出，无上下拉
 * @note 适用于驱动LED等低功耗设备
 */
void LED_Init(uint32_t LED_PIN, GPIO_TypeDef *GPIOx)
{
  GPIO_MyInit(GPIOx, LED_PIN, GPIO_Mode_OUT, GPIO_High_Speed, GPIO_OType_PP, GPIO_PuPd_NOPULL);
}

/**
 * @brief 蜂鸣器GPIO初始化函数
 * @param BEEP_PIN 蜂鸣器引脚（如GPIO_Pin_8等）
 * @param GPIOx 蜂鸣器所在GPIO端口（如GPIOA等）
 * @note 配置为50MHz输出模式，推挽输出，无上下拉
 * @note 适用于驱动蜂鸣器等中等频率设备
 */
void BEEP_Init(uint32_t BEEP_PIN, GPIO_TypeDef *GPIOx)
{
  GPIO_MyInit(GPIOx, BEEP_PIN, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL);
}
/**
 * @brief 按键GPIO初始化二次封装函数实现
 * @param x 按键编号(0-3)
 * @note 通过switch-case语句根据编号选择对应的按键GPIO进行初始化
 * @note 该函数简化了按键初始化调用，只需要传入编号即可
 * @note 不支持0-3以外的编号，传入其他值将不执行任何操作
 */
void KEY_Initx(uint32_t x)
{
  switch (x)
  {
  case 0:
    KEY_Init(KEY0_PIN, KEY0_PORT);
    break;
  case 1:
    KEY_Init(KEY1_PIN, KEY1_PORT);
    break;
  case 2:
    KEY_Init(KEY2_PIN, KEY2_PORT);
    break;
  case 3:
    KEY_Init(KEY3_PIN, KEY3_PORT);
    break;

  default:
    break;
  }
}

/**
 * @brief LED GPIO初始化二次封装函数实现
 * @param x LED编号(0-3)
 * @note 通过switch-case语句根据编号选择对应的LED GPIO进行初始化
 * @note 该函数简化了LED初始化调用，只需要传入编号即可
 * @note 不支持0-3以外的编号，传入其他值将不执行任何操作
 */
void LED_Initx(uint32_t x)
{
  switch (x)
  {
  case 0:
    LED_Init(LED0_PIN, LED0_PORT);
    break;
  case 1:
    LED_Init(LED1_PIN, LED1_PORT);
    break;
  case 2:
    LED_Init(LED2_PIN, LED2_PORT);
    break;
  case 3:
    LED_Init(LED3_PIN, LED3_PORT);
    break;
  default:
    break;
  }
}

/**
 * @brief 蜂鸣器GPIO初始化二次封装函数实现
 * @param x 蜂鸣器编号
 * @note 通过switch-case语句根据编号选择对应的蜂鸣器GPIO进行初始化
 * @note 当前仅支持编号0，为后续扩展预留接口
 * @note 不支持编号0以外的值，传入其他值将不执行任何操作
 */
void BEEP_Initx(uint32_t x)
{
  switch (x)
  {
  case 0:
    BEEP_Init(BEEP0_PIN, BEEP0_PORT);
    break;

  default:
    break;
  }
}


//位带操作

/**
 * @defgroup LED_Control_Functions LED控制函数实现
 * @brief 提供LED控制的标准函数接口实现
 * @note LED硬件设计为低电平点亮，高电平熄灭
 * @{
 */
/**
 * @brief LED0控制函数
 * @param state LED状态：0-亮，1-灭
 * @note 使用位带操作控制LED0 (PF9)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
void LED0_Set(uint8_t state)
{
  LED0(state);
}

/**
 * @brief LED1控制函数
 * @param state LED状态：0-亮，1-灭
 * @note 使用位带操作控制LED1 (PF10)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
void LED1_Set(uint8_t state)
{
  LED1(state);
}

/**
 * @brief LED2控制函数
 * @param state LED状态：0-亮，1-灭
 * @note 使用位带操作控制LED2 (PE13)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
void LED2_Set(uint8_t state)
{
  LED2(state);
}

/**
 * @brief LED3控制函数
 * @param state LED状态：0-亮，1-灭
 * @note 使用位带操作控制LED3 (PE14)
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
void LED3_Set(uint8_t state)
{
  LED3(state);
}

/**
 * @brief LED0状态读取函数
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作读取LED0 (PF9)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
uint8_t LED0_GetState(void)
{
  return LED0_STATE();
}

/**
 * @brief LED1状态读取函数
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作读取LED1 (PF10)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
uint8_t LED1_GetState(void)
{
  return LED1_STATE();
}

/**
 * @brief LED2状态读取函数
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作读取LED2 (PE13)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
uint8_t LED2_GetState(void)
{
  return LED2_STATE();
}

/**
 * @brief LED3状态读取函数
 * @return LED状态：0-亮，1-灭
 * @note 使用位带操作读取LED3 (PE14)当前状态
 * @note 硬件设计：低电平点亮，高电平熄灭
 */
uint8_t LED3_GetState(void)
{
  return LED3_STATE();
}
/** @} */

/**
 * @defgroup BEEP_Control_Functions 蜂鸣器控制函数实现
 * @brief 提供蜂鸣器控制的标准函数接口实现
 * @{
 */
/**
 * @brief 蜂鸣器0控制函数
 * @param state 蜂鸣器状态：1-开启，0-关闭
 * @note 使用位带操作控制蜂鸣器0 (PF8)
 */
void BEEP0_Set(uint8_t state)
{
  BEEP0(state);
}

/**
 * @brief 蜂鸣器0状态读取函数
 * @return 蜂鸣器状态：1-开启，0-关闭
 * @note 使用位带操作读取蜂鸣器0 (PF8)当前状态
 */
uint8_t BEEP0_GetState(void)
{
  return BEEP0_STATE();
}
/** @} */

/**
 * @defgroup KEY_Read_Functions 按键读取函数实现
 * @brief 提供按键读取的标准函数接口实现
 * @note 按键硬件设计为下拉模式，按下时为低电平(0)，释放时为高电平(1)
 * @{
 */
/**
 * @brief 按键0状态读取函数
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作读取按键0 (PA0)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
uint8_t KEY0_GetState(void)
{
  return KEY0_STATE();
}

/**
 * @brief 按键1状态读取函数
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作读取按键1 (PE2)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
uint8_t KEY1_GetState(void)
{
  return KEY1_STATE();
}

/**
 * @brief 按键2状态读取函数
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作读取按键2 (PE3)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
uint8_t KEY2_GetState(void)
{
  return KEY2_STATE();
}

/**
 * @brief 按键3状态读取函数
 * @return 按键状态：0-按下，1-释放
 * @note 使用位带操作读取按键3 (PE4)当前状态
 * @note 硬件设计：按键按下时为低电平，释放时为高电平
 */
uint8_t KEY3_GetState(void)
{
  return KEY3_STATE();
}
/** @} */

