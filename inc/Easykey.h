#ifndef EASYKEY_H
#define EASYKEY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 按键状态类型 */
typedef enum {
    EASYKEY_STATE_RELEASED = 0,  // 释放状态
    EASYKEY_STATE_PRESSED,       // 按下状态
} EasyKeyState;

/* 按键事件类型 */
typedef enum {
    EASYKEY_EVENT_NONE = 0,      // 无事件
    EASYKEY_EVENT_PRESS,         // 按下事件
    EASYKEY_EVENT_RELEASE,       // 释放事件
    EASYKEY_EVENT_CLICK,         // 点击事件（包含单击、双击和多击）
    EASYKEY_EVENT_LONG_PRESS,    // 长按事件
    EASYKEY_EVENT_CONTINUOUS,    // 连续触发事件
} EasyKeyEvent;

/* 按键配置结构 */
typedef struct {
    uint8_t id;             // 按键ID
    bool press_level;       // 按下电平 (true:高电平按下, false:低电平按下)
    uint16_t debounce_ms;   // 消抖时间(ms)
    uint16_t long_press_ms; // 长按时间阈值(ms)
    uint16_t multi_click_ms; // 多次点击时间间隔(ms)
    uint16_t conti_interval;// 连续触发间隔(ms)
} EasyKeyConfig;

/* 按键句柄 */
typedef void* EasyKeyHandle;

/* 日志级别 */
typedef enum {
    EASYKEY_LOG_NONE = 0,
    EASYKEY_LOG_ERROR,
    EASYKEY_LOG_WARNING,
    EASYKEY_LOG_INFO,
    EASYKEY_LOG_DEBUG
} EasyKeyLogLevel;

/* 移植接口函数 */
typedef bool (*EasyKeyReadFunc)(uint8_t key_id);
typedef void (*EasyKeyLogFunc)(EasyKeyLogLevel level, const char* fmt, ...);

/* ===== 核心功能API ===== */

/**
 * @brief 初始化按键扫描器
 * 
 * @param log_level 日志级别
 */
void easykey_init(EasyKeyLogLevel log_level);

/**
 * @brief 更新系统心跳（在中断中调用）
 */
void easykey_tick_update(void);

/**
 * @brief 按键扫描任务(需周期调用)
 */
void easykey_scan_task(void);

/**
 * @brief 注册按键
 * 
 * @param config 按键配置
 * @return EasyKeyHandle 按键句柄，注册失败返回NULL
 */
EasyKeyHandle easykey_register(const EasyKeyConfig* config);

/**
 * @brief 取消注册按键
 * 
 * @param handle 按键句柄
 */
void easykey_unregister(EasyKeyHandle handle);

/**
 * @brief 获取按键当前状态
 * 
 * @param handle 按键句柄
 * @return EasyKeyState 按键状态
 */
EasyKeyState easykey_get_state(EasyKeyHandle handle);

/**
 * @brief 获取按键事件
 * 
 * @param handle 按键句柄
 * @return EasyKeyEvent 按键事件
 */
EasyKeyEvent easykey_get_event(EasyKeyHandle handle);

/**
 * @brief 获取按键点击次数
 * 
 * @param handle 按键句柄
 * @return uint8_t 点击次数
 */
uint8_t easykey_get_click_count(EasyKeyHandle handle);

#ifdef __cplusplus
}
#endif

#endif /* EASYKEY_H */