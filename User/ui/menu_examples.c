/**
 * @file menu_examples.c
 * @brief 统一菜单架构使用示例
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.11.27
 */

#include "unified_menu.h"
#include "logo.h"
#include "stopwatch.h"
#include "setting.h"
#include "ui/TandH.h"
#include "flashlight.h"
#include "ui/alarm_all.h"
#include "step.h"
#include "testlist.h"

// ==================================
// 外部函数声明
// ==================================

extern void SPI_test(void);
extern void menu_2048_oled(void);
extern void frid_test(void);
extern void iwdg_test(void);
extern void air_level_test(void);

// ==================================
// 主菜单功能回调函数
// ==================================

static void stopwatch_on_select(menu_item_t *item)
{
    printf("Entering stopwatch function\r\n");
    stopwatch();
}

static void setting_on_select(menu_item_t *item)
{
    printf("Entering setting function\r\n");
    setting();
}

static void tandh_on_select(menu_item_t *item)
{
    printf("Entering T&H function\r\n");
    TandH();
}

static void flashlight_on_select(menu_item_t *item)
{
    printf("Entering flashlight function\r\n");
    flashlight();
}

static void alarm_on_select(menu_item_t *item)
{
    printf("Entering alarm menu\r\n");
    alarm_menu();
}

static void step_on_select(menu_item_t *item)
{
    printf("Entering step function\r\n");
    step();
}

static void test_on_select(menu_item_t *item)
{
    printf("Entering test list\r\n");
    testlist();
}

// ==================================
// 测试菜单功能回调函数
// ==================================

static void spi_test_on_select(menu_item_t *item)
{
    printf("Starting SPI test\r\n");
    SPI_test();
}

static void game_2048_on_select(menu_item_t *item)
{
    printf("Starting 2048 game\r\n");
    menu_2048_oled();
}

static void frid_test_on_select(menu_item_t *item)
{
    printf("Starting FRID test\r\n");
    frid_test();
}

static void iwdg_test_on_select(menu_item_t *item)
{
    printf("Starting IWDG test\r\n");
    iwdg_test();
}

static void air_level_test_on_select(menu_item_t *item)
{
    printf("Starting air level test\r\n");
    air_level_test();
}

// ==================================
// 菜单进入和退出回调
// ==================================

static void main_menu_on_enter(menu_item_t *item)
{
    OLED_Clear();
    printf("Main menu entered\r\n");
}

static void main_menu_on_exit(menu_item_t *item)
{
    printf("Main menu exited\r\n");
    OLED_Clear();
}

static void test_menu_on_enter(menu_item_t *item)
{
    printf("Test menu entered\r\n");
}

static void test_menu_on_exit(menu_item_t *item)
{
    printf("Test menu exited\r\n");
    OLED_Clear();
}

// ==================================
// 菜单创建函数
// ==================================

/**
 * @brief 创建主菜单（横向图标菜单）
 * @return 主菜单项指针
 */
menu_item_t* create_main_menu(void)
{
    // 创建主菜单项
    menu_content_t content = {.icon = {gImage_bg, 64, 64}};
    menu_item_t *main_menu = menu_item_create("Main Menu", MENU_TYPE_HORIZONTAL_ICON, content);
    
    if (main_menu == NULL) {
        return NULL;
    }
    
    // 设置主菜单回调
    menu_item_set_callbacks(main_menu, main_menu_on_enter, main_menu_on_exit, NULL, NULL);
    
    // 创建子菜单项
    menu_item_t *stopwatch_item = MENU_ITEM_ICON("Stopwatch", gImage_stopwatch, 32, 32);
    menu_item_t *setting_item = MENU_ITEM_ICON("Setting", gImage_setting, 32, 32);
    menu_item_t *tandh_item = MENU_ITEM_ICON("Temp&Humid", gImage_TandH, 32, 32);
    menu_item_t *flashlight_item = MENU_ITEM_ICON("Flashlight", gImage_flashlight, 32, 32);
    menu_item_t *alarm_item = MENU_ITEM_ICON("Alarm", gImage_bell, 32, 32);
    menu_item_t *step_item = MENU_ITEM_ICON("Step", gImage_step, 32, 32);
    menu_item_t *test_item = MENU_ITEM_ICON("Test", gImage_test, 32, 32);
    
    // 设置子菜单回调
    menu_item_set_callbacks(stopwatch_item, NULL, NULL, stopwatch_on_select, NULL);
    menu_item_set_callbacks(setting_item, NULL, NULL, setting_on_select, NULL);
    menu_item_set_callbacks(tandh_item, NULL, NULL, tandh_on_select, NULL);
    menu_item_set_callbacks(flashlight_item, NULL, NULL, flashlight_on_select, NULL);
    menu_item_set_callbacks(alarm_item, NULL, NULL, alarm_on_select, NULL);
    menu_item_set_callbacks(step_item, NULL, NULL, step_on_select, NULL);
    menu_item_set_callbacks(test_item, NULL, NULL, test_on_select, NULL);
    
    // 添加子菜单项
    menu_add_child(main_menu, stopwatch_item);
    menu_add_child(main_menu, setting_item);
    menu_add_child(main_menu, tandh_item);
    menu_add_child(main_menu, flashlight_item);
    menu_add_child(main_menu, alarm_item);
    menu_add_child(main_menu, step_item);
    menu_add_child(main_menu, test_item);
    
    return main_menu;
}

/**
 * @brief 创建测试菜单（竖向列表菜单）
 * @return 测试菜单项指针
 */
menu_item_t* create_test_menu(void)
{
    // 创建测试菜单项
    menu_content_t content = {.text = {"Test Menu", 20}};
    menu_item_t *test_menu = menu_item_create("Test Menu", MENU_TYPE_VERTICAL_LIST, content);
    
    if (test_menu == NULL) {
        return NULL;
    }
    
    // 设置测试菜单回调
    menu_item_set_callbacks(test_menu, test_menu_on_enter, test_menu_on_exit, NULL, NULL);
    
    // 创建子菜单项
    menu_item_t *spi_test_item = MENU_ITEM_TEXT("SPI_test", "SPI_test", 20);
    menu_item_t *game_2048_item = MENU_ITEM_TEXT("2048_oled", "2048_oled", 20);
    menu_item_t *frid_test_item = MENU_ITEM_TEXT("frid_test", "frid_test", 20);
    menu_item_t *iwdg_test_item = MENU_ITEM_TEXT("iwdg_test", "iwdg_test", 20);
    menu_item_t *air_level_item = MENU_ITEM_TEXT("air_level", "air_level", 20);
    
    // 设置子菜单回调
    menu_item_set_callbacks(spi_test_item, NULL, NULL, spi_test_on_select, NULL);
    menu_item_set_callbacks(game_2048_item, NULL, NULL, game_2048_on_select, NULL);
    menu_item_set_callbacks(frid_test_item, NULL, NULL, frid_test_on_select, NULL);
    menu_item_set_callbacks(iwdg_test_item, NULL, NULL, iwdg_test_on_select, NULL);
    menu_item_set_callbacks(air_level_item, NULL, NULL, air_level_test_on_select, NULL);
    
    // 添加子菜单项
    menu_add_child(test_menu, spi_test_item);
    menu_add_child(test_menu, game_2048_item);
    menu_add_child(test_menu, frid_test_item);
    menu_add_child(test_menu, iwdg_test_item);
    menu_add_child(test_menu, air_level_item);
    
    return test_menu;
}

// ==================================
// 兼容性包装函数
// ==================================

/**
 * @brief 兼容原有的menu函数
 * @param cho 初始选择项
 * @return 选中的菜单项索引
 */
uint8_t unified_menu(uint8_t cho)
{
    // 如果菜单系统未初始化，先初始化
    if (g_menu_sys.root_menu == NULL) {
        menu_system_init();
        
        // 创建主菜单
        menu_item_t *main_menu = create_main_menu();
        if (main_menu == NULL) {
            printf("Failed to create main menu\r\n");
            return cho;
        }
        
        // 设置初始选中项
        if (cho < main_menu->child_count) {
            main_menu->selected_child = cho;
        }
        
        // 进入主菜单
        menu_enter(main_menu);
        g_menu_sys.root_menu = main_menu;
    }
    
    // 设置为横向菜单布局
    g_menu_sys.layout = (menu_layout_config_t)LAYOUT_HORIZONTAL_MAIN();
    
    // 等待用户选择
    uint8_t result = cho;
    uint32_t start_time = xTaskGetTickCount();
    
    while (1) {
        // 处理菜单事件
        menu_event_t event;
        if (xQueueReceive(g_menu_sys.event_queue, &event, pdMS_TO_TICKS(100)) == pdPASS) {
            int8_t handle_result = menu_process_event(&event);
            
            // 如果是进入功能事件，返回选中项索引
            if (event.type == MENU_EVENT_KEY_ENTER && handle_result >= 0) {
                result = handle_result;
                break;
            }
            
            // 如果是选择事件（KEY2），返回选中项索引
            if (event.type == MENU_EVENT_KEY_SELECT) {
                result = g_menu_sys.current_menu->selected_child;
                break;
            }
        }
        
        // 超时检查（兼容原有超时行为）
        if ((xTaskGetTickCount() - start_time) > pdMS_TO_TICKS(30000)) { // 30秒超时
            printf("Menu timeout\r\n");
            result = g_menu_sys.current_menu->selected_child;
            break;
        }
    }
    
    // 退出菜单
    if (g_menu_sys.current_menu && g_menu_sys.current_menu->on_exit) {
        g_menu_sys.current_menu->on_exit(g_menu_sys.current_menu);
    }
    
    OLED_Clear();
    return result;
}

/**
 * @brief 兼容原有的testlist函数
 */
void unified_testlist(void)
{
    // 如果菜单系统未初始化，先初始化
    if (g_menu_sys.root_menu == NULL) {
        menu_system_init();
    }
    
    // 创建测试菜单
    menu_item_t *test_menu = create_test_menu();
    if (test_menu == NULL) {
        printf("Failed to create test menu\r\n");
        return;
    }
    
    // 设置为竖向菜单布局
    g_menu_sys.layout = (menu_layout_config_t)LAYOUT_VERTICAL_TEST();
    
    // 进入测试菜单
    menu_enter(test_menu);
    
    // 等待用户操作
    while (1) {
        // 处理菜单事件
        menu_event_t event;
        if (xQueueReceive(g_menu_sys.event_queue, &event, pdMS_TO_TICKS(50)) == pdPASS) {
            int8_t handle_result = menu_process_event(&event);
            
            // 如果是返回事件，退出菜单
            if (event.type == MENU_EVENT_KEY_SELECT) {
                break;
            }
        }
    }
    
    // 退出测试菜单
    if (test_menu->on_exit) {
        test_menu->on_exit(test_menu);
    }
}

// ==================================
// 初始化和启动函数
// ==================================

/**
 * @brief 初始化统一菜单系统并创建菜单
 * @return 0-成功，其他-失败
 */
int8_t init_unified_menu_system(void)
{
    // 初始化菜单系统
    int8_t result = menu_system_init();
    if (result != 0) {
        return result;
    }
    
    // 创建主菜单
    menu_item_t *main_menu = create_main_menu();
    if (main_menu == NULL) {
        return -1;
    }
    
    g_menu_sys.root_menu = main_menu;
    
    // 创建菜单任务
    BaseType_t task_result = xTaskCreate(menu_task, "MenuTask", 1024, NULL, 3, NULL);
    if (task_result != pdPASS) {
        return -2;
    }
    
    // 创建按键任务
    task_result = xTaskCreate(menu_key_task, "MenuKeyTask", 256, NULL, 4, NULL);
    if (task_result != pdPASS) {
        return -3;
    }
    
    printf("Unified menu system initialized successfully\r\n");
    return 0;
}

/**
 * @brief 触发进入主菜单
 */
void trigger_main_menu(void)
{
    if (g_menu_sys.root_menu) {
        // 设置为横向菜单布局
        g_menu_sys.layout = (menu_layout_config_t)LAYOUT_HORIZONTAL_MAIN();
        
        // 进入主菜单
        menu_enter(g_menu_sys.root_menu);
        
        // 发送刷新事件
        menu_event_t event = {
            .type = MENU_EVENT_REFRESH,
            .timestamp = xTaskGetTickCount(),
            .param = 0
        };
        xQueueSend(g_menu_sys.event_queue, &event, 0);
    }
}

/**
 * @brief 退出当前菜单
 */
void exit_current_menu(void)
{
    if (g_menu_sys.current_menu) {
        menu_back_to_parent();
    }
}