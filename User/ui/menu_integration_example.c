/**
 * @file menu_integration_example.c
 * @brief 展示如何在现有项目中集成统一菜单架构
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.11.27
 */

#include "unified_menu.h"
#include "menu_examples.h"

// ==================================
// 集成示例：修改main.c中的菜单调用
// ==================================

/**
 * @brief 示例：如何在main.c中替换原有的menu函数调用
 * 
 * 原有代码：
 * ```c
 * case KEY3_PRES:
 *     printf("cd menu\r\n");
 *     cho = menu(cho);  // 原有menu函数
 *     printf("out menu\r\n");
 *     break;
 * ```
 * 
 * 替换为：
 * ```c
 * case KEY3_PRES:
 *     printf("cd menu\r\n");
 *     cho = unified_menu(cho);  // 新的统一菜单函数
 *     printf("out menu\r\n");
 *     break;
 * ```
 */

// ==================================
// 渐进式迁移策略
// ==================================

/**
 * @brief 阶段1：保留原有菜单，添加新菜单选项
 * 
 * 可以在testlist.c中添加一个"新菜单系统"选项：
 */
void new_menu_system_test(void)
{
    printf("Testing new unified menu system\r\n");
    
    // 初始化统一菜单系统
    if (init_unified_menu_system() == 0) {
        // 触发主菜单
        trigger_main_menu();
        
        printf("New menu system started successfully\r\n");
    } else {
        printf("Failed to initialize new menu system\r\n");
        OLED_Printf_Line(1, "Menu Init Failed!");
        OLED_Refresh_Dirty();
        delay_ms(2000);
    }
}

/**
 * @brief 在testlist.c的test_opt数组中添加：
 * 
 * ```c
 * char *test_opt[] = {
 *     "SPI_test",
 *     "2048_oled", 
 *     "frid_test",
 *     "iwdg_test",
 *     "air_level",
 *     "new_menu_system"  // 新增选项
 * };
 * ```
 * 
 * 在test_enter_select函数中添加对应的case：
 * ```c
 * case 5:
 *     new_menu_system_test();
 *     break;
 * ```
 */

// ==================================
// 完整替换示例
// ==================================

/**
 * @brief 完整替换main.c中的app_main_task函数的菜单相关部分
 * 
 * 替换原有菜单初始化和按键处理逻辑
 */

// 在main.c的app_main_task函数开始处添加：
void menu_system_setup(void)
{
    // 初始化统一菜单系统
    if (init_unified_menu_system() != 0) {
        printf("Failed to initialize unified menu system\r\n");
        return;
    }
    
    printf("Unified menu system ready\r\n");
}

// 替换原有的菜单按键处理：
void handle_menu_key_with_new_system(uint8_t key, uint8_t *cho)
{
    static uint8_t menu_mode = 0; // 0=主界面，1=菜单模式
    
    switch (key) {
        case KEY3_PRES:
            if (!menu_mode) {
                printf("Entering menu mode\r\n");
                trigger_main_menu();
                menu_mode = 1;
            } else {
                printf("Exiting menu mode\r\n");
                exit_current_menu();
                menu_mode = 0;
            }
            break;
            
        // 其他按键可以根据当前模式处理
        default:
            if (!menu_mode) {
                // 主界面模式的原有处理逻辑
                // ... 保留原有的KEY0_PRES, KEY1_PRES等处理
            }
            break;
    }
}

// ==================================
// 高级配置示例
// ==================================

/**
 * @brief 自定义菜单布局配置
 */
void setup_custom_menu_layout(void)
{
    // 自定义横向菜单布局
    g_menu_sys.layout.horizontal.visible_count = 3;
    g_menu_sys.layout.horizontal.item_width = 40;
    g_menu_sys.layout.horizontal.item_height = 40;
    g_menu_sys.layout.horizontal.spacing = 12;
    g_menu_sys.layout.horizontal.start_x = 4;
    g_menu_sys.layout.horizontal.start_y = 12;
    
    // 自定义竖向菜单布局
    g_menu_sys.layout.vertical.items_per_page = 5;
    g_menu_sys.layout.vertical.item_height = 12;
    g_menu_sys.layout.vertical.start_x = 2;
    g_menu_sys.layout.vertical.start_y = 0;
    g_menu_sys.layout.vertical.indent_spaces = 1;
    g_menu_sys.layout.vertical.selected_char = '>';
    g_menu_sys.layout.vertical.unselected_char = ' ';
}

/**
 * @brief 添加动画效果的菜单显示
 */
void animated_menu_display_horizontal(menu_item_t *menu)
{
    static uint8_t animation_step = 0;
    static uint32_t last_animation_time = 0;
    
    uint32_t current_time = xTaskGetTickCount();
    
    // 每200ms更新一次动画
    if (current_time - last_animation_time > pdMS_TO_TICKS(200)) {
        animation_step = (animation_step + 1) % 4;
        last_animation_time = current_time;
    }
    
    if (menu == NULL || menu->child_count == 0) {
        return;
    }
    
    // 计算可见范围
    uint8_t center_index = menu->selected_child;
    uint8_t left_index = (center_index == 0) ? menu->child_count - 1 : center_index - 1;
    uint8_t right_index = (center_index + 1) % menu->child_count;
    
    // 根据动画步骤调整透明度/亮度
    uint8_t left_brightness = (animation_step == 0) ? 1 : 0;
    uint8_t right_brightness = (animation_step == 2) ? 1 : 0;
    
    // 显示图标（带动画效果）
    if (menu->children[left_index].content.icon.icon_data) {
        OLED_ShowPicture(0, 16, 32, 32, 
                        menu->children[left_index].content.icon.icon_data, left_brightness);
    }
    
    if (menu->children[center_index].content.icon.icon_data) {
        OLED_ShowPicture(48, 16, 32, 32, 
                        menu->children[center_index].content.icon.icon_data, 0);
    }
    
    if (menu->children[right_index].content.icon.icon_data) {
        OLED_ShowPicture(96, 16, 32, 32, 
                        menu->children[right_index].content.icon.icon_data, right_brightness);
    }
    
    OLED_Refresh();
}

// ==================================
// 性能监控示例
// ==================================

/**
 * @brief 菜单性能监控结构
 */
typedef struct {
    uint32_t menu_enter_count;
    uint32_t menu_exit_count;
    uint32_t key_press_count;
    uint32_t refresh_count;
    uint32_t avg_refresh_time;
    uint32_t max_refresh_time;
} menu_performance_stats_t;

static menu_performance_stats_t g_menu_stats = {0};

/**
 * @brief 性能监控函数
 */
void menu_performance_monitor_start(void)
{
    g_menu_stats.menu_enter_count++;
}

void menu_performance_monitor_end(void)
{
    g_menu_stats.menu_exit_count++;
}

void menu_performance_monitor_key(void)
{
    g_menu_stats.key_press_count++;
}

void menu_performance_monitor_refresh(uint32_t refresh_time)
{
    g_menu_stats.refresh_count++;
    
    if (refresh_time > g_menu_stats.max_refresh_time) {
        g_menu_stats.max_refresh_time = refresh_time;
    }
    
    // 简单的移动平均
    g_menu_stats.avg_refresh_time = (g_menu_stats.avg_refresh_time * 7 + refresh_time) / 8;
}

void menu_print_performance_stats(void)
{
    printf("=== Menu Performance Stats ===\r\n");
    printf("Menu Enter: %lu\r\n", g_menu_stats.menu_enter_count);
    printf("Menu Exit: %lu\r\n", g_menu_stats.menu_exit_count);
    printf("Key Press: %lu\r\n", g_menu_stats.key_press_count);
    printf("Refresh Count: %lu\r\n", g_menu_stats.refresh_count);
    printf("Avg Refresh Time: %lu ms\r\n", g_menu_stats.avg_refresh_time);
    printf("Max Refresh Time: %lu ms\r\n", g_menu_stats.max_refresh_time);
    printf("==============================\r\n");
}

// ==================================
// 调试和测试功能
// ==================================

/**
 * @brief 菜单系统自检函数
 */
int8_t menu_system_self_test(void)
{
    printf("Starting menu system self test...\r\n");
    
    // 1. 测试菜单项创建
    menu_content_t test_content = {.text = {"Test", 20}};
    menu_item_t *test_item = menu_item_create("TestItem", MENU_TYPE_VERTICAL_LIST, test_content);
    if (test_item == NULL) {
        printf("ERROR: Failed to create menu item\r\n");
        return -1;
    }
    printf("✓ Menu item creation OK\r\n");
    
    // 2. 测试子菜单添加
    menu_item_t *child_item = menu_item_create("ChildItem", MENU_TYPE_VERTICAL_LIST, test_content);
    if (child_item == NULL) {
        printf("ERROR: Failed to create child menu item\r\n");
        return -2;
    }
    
    if (menu_add_child(test_item, child_item) != 0) {
        printf("ERROR: Failed to add child menu item\r\n");
        return -3;
    }
    printf("✓ Child menu addition OK\r\n");
    
    // 3. 测试FreeRTOS资源
    if (g_menu_sys.event_queue == NULL || g_menu_sys.display_mutex == NULL) {
        printf("ERROR: FreeRTOS resources not initialized\r\n");
        return -4;
    }
    printf("✓ FreeRTOS resources OK\r\n");
    
    // 4. 测试事件处理
    menu_event_t test_event = {
        .type = MENU_EVENT_REFRESH,
        .timestamp = xTaskGetTickCount(),
        .param = 0
    };
    
    if (xQueueSend(g_menu_sys.event_queue, &test_event, pdMS_TO_TICKS(100)) != pdPASS) {
        printf("ERROR: Failed to send event to queue\r\n");
        return -5;
    }
    printf("✓ Event queue OK\r\n");
    
    // 5. 测试显示互斥量
    if (xSemaphoreTake(g_menu_sys.display_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        printf("ERROR: Failed to take display mutex\r\n");
        return -6;
    }
    xSemaphoreGive(g_menu_sys.display_mutex);
    printf("✓ Display mutex OK\r\n");
    
    // 清理测试数据
    vPortFree(test_item);
    vPortFree(child_item);
    
    printf("Menu system self test PASSED ✓\r\n");
    return 0;
}

/**
 * @brief 菜单系统状态转储
 */
void menu_dump_status(void)
{
    printf("=== Menu System Status ===\r\n");
    printf("Current Menu: %s\r\n", 
           g_menu_sys.current_menu ? g_menu_sys.current_menu->name : "NULL");
    printf("Root Menu: %s\r\n", 
           g_menu_sys.root_menu ? g_menu_sys.root_menu->name : "NULL");
    printf("Menu Active: %s\r\n", g_menu_sys.menu_active ? "YES" : "NO");
    printf("Need Refresh: %s\r\n", g_menu_sys.need_refresh ? "YES" : "NO");
    printf("Current Page: %d/%d\r\n", g_menu_sys.current_page, g_menu_sys.total_pages);
    printf("Items Per Page: %d\r\n", g_menu_sys.items_per_page);
    printf("Blink State: %d\r\n", g_menu_sys.blink_state);
    printf("===========================\r\n");
}