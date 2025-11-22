#include "step.h"
#include "ui/alarm_all.h"
#include "oled.h"
#include "oled_print.h"
#include "MPU6050.h"
#include "MPU6050/eMPL/inv_mpu_dmp_motion_driver.h"
#include "key.h"
#include "simple_pedometer.h"
#include "code/spi.h"

// 步数数据在W25Q128中的存储地址
#define STEP_DATA_BASE_ADDR     0x001000   // 从4KB偏移开始，避免与闹钟数据冲突
#define STEP_DATA_SECTOR_SIZE   4096       // 步数数据占用一个扇区

// 步数数据结构体
typedef struct {
    unsigned long step_count;      // 步数
    uint32_t last_update_time;     // 最后更新时间戳
    uint32_t total_active_seconds; // 累计活跃时间（秒）
    uint16_t checksum;             // 数据校验和
} StepData_TypeDef;

// 引用全局函数
extern uint32_t get_systick(void);
extern unsigned long g_step_count;

/**
 * @brief 计算数据校验和
 * @param data 数据指针
 * @param length 数据长度
 * @return 校验和
 */
static uint16_t Calculate_Checksum(uint8_t *data, uint16_t length)
{
    uint16_t checksum = 0;
    for (uint16_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum;
}

/**
 * @brief 保存步数数据到W25Q128 Flash
 */
void Steps_Save(void)
{
    StepData_TypeDef step_data;
    uint32_t write_addr = STEP_DATA_BASE_ADDR;
    
    printf("Saving step data to W25Q128 Flash\r\n");
    
    // 检查W25Q128是否存在
    uint32_t flash_id = W25Q128_ReadID();
    if (flash_id != W25X_JEDECID) {
        printf("W25Q128 not available, skipping step save\r\n");
        return;
    }
    
    // 准备步数数据
    step_data.step_count = g_step_count;
    step_data.last_update_time = get_systick() / 1000;  // 转换为秒
    step_data.total_active_seconds = step_data.last_update_time;  // 简化处理
    
    // 计算校验和（不包括checksum字段）
    uint8_t *data_ptr = (uint8_t*)&step_data;
    step_data.checksum = Calculate_Checksum(data_ptr, sizeof(StepData_TypeDef) - sizeof(uint16_t));
    
    // 擦除存储区域
    W25Q128_SectorErase(write_addr);
    
    // 写入步数数据
    W25Q128_BufferWrite(data_ptr, write_addr, sizeof(StepData_TypeDef));
    
    printf("Successfully saved step data: %lu steps, checksum=0x%04X\r\n", 
           step_data.step_count, step_data.checksum);
}

/**
 * @brief 从W25Q128 Flash加载步数数据
 */
void Steps_Load(void)
{
    StepData_TypeDef step_data;
    uint32_t read_addr = STEP_DATA_BASE_ADDR;
    
    printf("Loading step data from W25Q128 Flash\r\n");
    
    // 检查W25Q128是否存在
    uint32_t flash_id = W25Q128_ReadID();
    if (flash_id != W25X_JEDECID) {
        printf("W25Q128 not available, using default step count (0)\r\n");
        g_step_count = 0;
        return;
    }
    
    // 读取步数数据
    W25Q128_ReadData((uint8_t*)&step_data, read_addr, sizeof(StepData_TypeDef));
    
    // 计算并验证校验和
    uint8_t *data_ptr = (uint8_t*)&step_data;
    uint16_t calculated_checksum = Calculate_Checksum(data_ptr, sizeof(StepData_TypeDef) - sizeof(uint16_t));
    
    if (step_data.checksum == calculated_checksum) {
        // 校验通过，使用保存的数据
        g_step_count = step_data.step_count;
        printf("Successfully loaded step data: %lu steps (valid checksum)\r\n", g_step_count);
        printf("Last update time: %lu seconds ago\r\n", step_data.last_update_time);
    } else {
        // 校验失败，使用默认值
        printf("Step data checksum mismatch (stored: 0x%04X, calculated: 0x%04X)\r\n", 
               step_data.checksum, calculated_checksum);
        printf("Using default step count (0)\r\n");
        g_step_count = 0;
    }
}

void step(void)
{
    delay_ms(10);
    OLED_Clear();
    OLED_Printf_Line(0, "Step Counter");
    OLED_Printf_Line(1, "KEY0-Reset KEY2-Back");
    OLED_Printf_Line(2, "Steps: %lu", g_step_count);
    OLED_Printf_Line(3, "Time: 0s");
    OLED_Refresh();
    
    // 不再初始化计步器，保持使用全局计数值
    
    unsigned long last_count = g_step_count;
    
    while(1)
    {
        // 全局闹钟处理 - 在计步器界面也能处理闹钟
        if (Alarm_GlobalHandler()) {
            continue; // 如果正在处理闹钟提醒，跳过计步器循环的其他部分
        }
        
        // 读取加速度数据
        short ax, ay, az;
        MPU_Get_Accelerometer(&ax, &ay, &az);
        
        // 使用简单计步器更新步数
        unsigned long count = simple_pedometer_update(ax, ay, az);
        
        OLED_Printf_Line(2, "Steps: %lu", count);
        OLED_Printf_Line(3, "Simple mode");
        OLED_Refresh_Dirty();
        
        // 检查步数变化
        if(count != last_count)
        {
            printf("!!! STEP DETECTED: %ld -> %ld !!!\r\n", last_count, count);
            last_count = count;
        }
        
        // 检查按键
        u8 key = KEY_Get();
        if(key != 0) 
        {
            switch(key)
            {
                case KEY0_PRES: // 短按KEY0重置步数
                    simple_pedometer_reset();
                    OLED_Printf_Line(2, "step reset!");
                    OLED_Printf_Line(3, "time reset!");
                    OLED_Refresh();
                    delay_ms(1000);
                    OLED_Printf_Line(2, "Steps: %lu", g_step_count);
                    OLED_Printf_Line(3, "Simple mode");
                    OLED_Refresh();
                    last_count = g_step_count;
                    break;
                    
                case KEY2_PRES: // 按KEY2返回菜单页面
                    OLED_Clear();
                    return;
                    
                default:
                    break;
            }
        }
        
        delay_ms(50); // 每50ms更新一次显示，提高响应速度
    }
}
