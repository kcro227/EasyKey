#include "easykey.h"
#include "easykey_port.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#define MAX_KEY_COUNT     16  // 最大按键数量
#define DEFAULT_DEBOUNCE  20  // 默认消抖时间(ms)
#define DEFAULT_MULTI_CLICK 300 // 默认多次点击时间间隔(ms)

/* 按键消抖状态 */
typedef enum {
    DEBOUNCE_STATE_IDLE = 0, // 空闲状态
    DEBOUNCE_STATE_DOWN,     // 按下消抖
    DEBOUNCE_STATE_UP,       // 释放消抖
} DebounceState;

/* 按键事件状态 */
typedef enum {
    EVENT_STATE_IDLE = 0,     // 空闲状态
    EVENT_STATE_PRESSED,      // 按下状态
    EVENT_STATE_WAIT_MULTI,   // 等待多次点击
    EVENT_STATE_LONG_PRESS,   // 长按状态
} EventState;

/* 按键控制块结构 */
typedef struct {
    EasyKeyConfig config;      // 按键配置
    EasyKeyState stable_state; // 稳定状态
    EasyKeyEvent event;        // 当前事件

    // 消抖状态机
    DebounceState debounce_state; // 消抖状态
    uint32_t debounce_timer;      // 消抖计时器
    bool last_raw_level;          // 上次原始电平

    // 事件状态机
    EventState event_state; // 事件状态
    uint32_t press_timer;   // 按下计时器
    uint32_t release_timer; // 释放计时器
    uint8_t click_count;    // 连击计数
} KeyControlBlock;

static KeyControlBlock key_pool[MAX_KEY_COUNT];               // 按键池
static uint8_t registered_keys           = 0;                 // 已注册按键数
static EasyKeyLogLevel current_log_level = EASYKEY_LOG_ERROR; // 当前日志级别

/* 系统心跳计数器 - 在中断中更新 */
static volatile uint32_t system_tick = 0;

/* 内部函数声明 */
static void debounce_state_machine(KeyControlBlock *key);
static void event_state_machine(KeyControlBlock *key);
static void log_message(EasyKeyLogLevel level, const char *fmt, ...);
static uint32_t get_current_tick(void); // 获取当前系统时间

/* ==== 心跳函数实现 ==== */

/**
 * @brief 更新系统心跳（在中断中调用）
 */
void easykey_tick_update(void)
{
    system_tick++;
}

/**
 * @brief 获取当前系统时间（毫秒）
 */
static uint32_t get_current_tick(void)
{
    return system_tick;
}

/* ==== 核心功能实现 ==== */

/* 初始化按键扫描器 */
void easykey_init(EasyKeyLogLevel log_level)
{
    current_log_level = log_level;
    registered_keys   = 0;
    system_tick       = 0;

    // 初始化所有按键控制块
    for (int i = 0; i < MAX_KEY_COUNT; i++) {
        key_pool[i].config.id    = 0xFF; // 标记为无效
        key_pool[i].stable_state = EASYKEY_STATE_RELEASED;
        key_pool[i].event        = EASYKEY_EVENT_NONE;
    }

    log_message(EASYKEY_LOG_INFO, "EasyKey initialized with log level: %d", log_level);
}

/* 按键扫描任务 */
void easykey_scan_task(void)
{
    for (int i = 0; i < MAX_KEY_COUNT; i++) {
        if (key_pool[i].config.id != 0xFF) {
            // 第一步：执行消抖状态机
            debounce_state_machine(&key_pool[i]);

            // 第二步：执行事件状态机
            event_state_machine(&key_pool[i]);
        }
    }
}

/* 注册按键 */
EasyKeyHandle easykey_register(const EasyKeyConfig *config)
{
    if (registered_keys >= MAX_KEY_COUNT) {
        log_message(EASYKEY_LOG_ERROR, "Register failed: max keys reached (%d)", MAX_KEY_COUNT);
        return NULL;
    }

    for (int i = 0; i < MAX_KEY_COUNT; i++) {
        if (key_pool[i].config.id == 0xFF) {
            key_pool[i].config = *config;

            // 设置默认值
            if (key_pool[i].config.debounce_ms == 0) {
                key_pool[i].config.debounce_ms = DEFAULT_DEBOUNCE;
            }
            if (key_pool[i].config.multi_click_ms == 0) {
                key_pool[i].config.multi_click_ms = DEFAULT_MULTI_CLICK;
            }

            // 初始化状态
            key_pool[i].stable_state   = EASYKEY_STATE_RELEASED;
            key_pool[i].debounce_state = DEBOUNCE_STATE_IDLE;
            key_pool[i].event_state    = EVENT_STATE_IDLE;
            key_pool[i].debounce_timer = 0;
            key_pool[i].press_timer    = 0;
            key_pool[i].release_timer  = 0;
            key_pool[i].click_count    = 0;
            key_pool[i].event          = EASYKEY_EVENT_NONE;

            // 读取初始电平
            bool init_level            = easykey_port_read_key(config->id);
            key_pool[i].last_raw_level = init_level;

            registered_keys++;
            log_message(EASYKEY_LOG_INFO, "Key %d registered (press_level: %s)",
                        config->id, config->press_level ? "HIGH" : "LOW");
            return (EasyKeyHandle)&key_pool[i];
        }
    }

    log_message(EASYKEY_LOG_ERROR, "Register failed: no free slot");
    return NULL;
}

/* 取消注册按键 */
void easykey_unregister(EasyKeyHandle handle)
{
    KeyControlBlock *key = (KeyControlBlock *)handle;
    if (key->config.id != 0xFF) {
        log_message(EASYKEY_LOG_INFO, "Key %d unregistered", key->config.id);
        key->config.id = 0xFF; // 标记为无效
        registered_keys--;
    }
}

/* 获取按键状态 */
EasyKeyState easykey_get_state(EasyKeyHandle handle)
{
    KeyControlBlock *key = (KeyControlBlock *)handle;
    return key->stable_state;
}

/* 获取按键事件 */
EasyKeyEvent easykey_get_event(EasyKeyHandle handle)
{
    KeyControlBlock *key = (KeyControlBlock *)handle;
    EasyKeyEvent event   = key->event;
    key->event           = EASYKEY_EVENT_NONE; // 清除事件
    return event;
}

/* 获取按键点击次数 */
uint8_t easykey_get_click_count(EasyKeyHandle handle)
{
    KeyControlBlock *key = (KeyControlBlock *)handle;
    return key->click_count;
}

/* 消抖状态机 */
static void debounce_state_machine(KeyControlBlock *key)
{
    uint32_t current_tick = get_current_tick();
    bool current_level    = easykey_port_read_key(key->config.id);
    bool press_active     = (current_level == key->config.press_level);

    switch (key->debounce_state) {
        case DEBOUNCE_STATE_IDLE:
            if (press_active != (key->stable_state == EASYKEY_STATE_PRESSED)) {
                key->debounce_state = press_active ? DEBOUNCE_STATE_DOWN : DEBOUNCE_STATE_UP;
                key->debounce_timer = current_tick;
                log_message(EASYKEY_LOG_DEBUG, "Key %d: debounce start", key->config.id);
            }
            break;

        case DEBOUNCE_STATE_DOWN:
            if (current_tick - key->debounce_timer >= key->config.debounce_ms) {
                if (current_level == key->last_raw_level) {
                    key->stable_state = EASYKEY_STATE_PRESSED;
                    key->event        = EASYKEY_EVENT_PRESS;
                    log_message(EASYKEY_LOG_DEBUG, "Key %d: pressed (stable)", key->config.id);
                }
                key->debounce_state = DEBOUNCE_STATE_IDLE;
            }
            break;

        case DEBOUNCE_STATE_UP:
            if (current_tick - key->debounce_timer >= key->config.debounce_ms) {
                if (current_level == key->last_raw_level) {
                    key->stable_state = EASYKEY_STATE_RELEASED;
                    key->event        = EASYKEY_EVENT_RELEASE;
                    log_message(EASYKEY_LOG_DEBUG, "Key %d: released (stable)", key->config.id);
                }
                key->debounce_state = DEBOUNCE_STATE_IDLE;
            }
            break;
    }

    key->last_raw_level = current_level;
}

/* 事件状态机 */
static void event_state_machine(KeyControlBlock *key)
{
    uint32_t current_tick = get_current_tick();

    switch (key->event_state) {
        case EVENT_STATE_IDLE:
            if (key->stable_state == EASYKEY_STATE_PRESSED) {
                key->event_state = EVENT_STATE_PRESSED;
                key->press_timer = current_tick;
                log_message(EASYKEY_LOG_DEBUG, "Key %d: event pressed", key->config.id);
            }
            else{
                key->click_count = 0;
            }
            break;

        case EVENT_STATE_PRESSED:
            // 检测长按
            if (key->stable_state == EASYKEY_STATE_PRESSED &&
                current_tick - key->press_timer >= key->config.long_press_ms) {
                key->event       = EASYKEY_EVENT_LONG_PRESS;
                key->event_state = EVENT_STATE_LONG_PRESS;
                key->press_timer = current_tick; // 重置连续触发计时
                log_message(EASYKEY_LOG_DEBUG, "Key %d: long press", key->config.id);
            }
            // 检测释放
            else if (key->stable_state == EASYKEY_STATE_RELEASED) {
                key->click_count++;
                key->event = EASYKEY_EVENT_CLICK; // 立即触发点击事件
                key->event_state   = EVENT_STATE_WAIT_MULTI;
                key->release_timer = current_tick;
                log_message(EASYKEY_LOG_DEBUG, "Key %d: click event (count: %d)",
                            key->config.id, key->click_count);
            }
            break;

        case EVENT_STATE_WAIT_MULTI:
            // 超时处理：回到空闲状态
            if (current_tick - key->release_timer >= key->config.multi_click_ms) {
                key->event_state = EVENT_STATE_IDLE;
                log_message(EASYKEY_LOG_DEBUG, "Key %d: multi click timeout (count: %d)",
                            key->config.id, key->click_count);
            }
            // 检测再次按下
            else if (key->stable_state == EASYKEY_STATE_PRESSED) {
                key->event_state = EVENT_STATE_PRESSED;
                key->press_timer = current_tick; // 重置按下计时器
                log_message(EASYKEY_LOG_DEBUG, "Key %d: next press (count: %d)",
                            key->config.id, key->click_count);
            }
            break;

        case EVENT_STATE_LONG_PRESS:
            // 检测连续触发
            if (key->stable_state == EASYKEY_STATE_PRESSED &&
                current_tick - key->press_timer >= key->config.conti_interval) {
                key->event       = EASYKEY_EVENT_CONTINUOUS;
                key->press_timer = current_tick; // 重置连续触发计时
                log_message(EASYKEY_LOG_DEBUG, "Key %d: continuous", key->config.id);
            }
            // 检测释放
            else if (key->stable_state == EASYKEY_STATE_RELEASED) {
                key->event_state = EVENT_STATE_IDLE;
                log_message(EASYKEY_LOG_DEBUG, "Key %d: long press release", key->config.id);
            }
            break;
    }
}

/* 日志输出函数 */
static void log_message(EasyKeyLogLevel level, const char *fmt, ...)
{
    if (level > current_log_level) return;

    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    easykey_port_log(level, buffer);
}