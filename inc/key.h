#ifndef _KEY_H
#define _KEY_H
#include "key_config.h"
#include "stdint.h"
#include <stdint.h>
#include <stdbool.h>

#if CONFIG_USE_FREERTOS
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"

#define malloc pvPortMalloc
#define free   vPortFree
#endif

// 定义按键状态枚举
typedef enum {
    KEY_STATE_RELEASED = 0, // 按键释放状态
    KEY_STATE_DownShake,    // 按键按下抖动状态
    KEY_STATE_PRESSED,      // 按键按下状态
    KEY_STATE_UpShake,      // 按键抬起抖动状态
} KEY_State_t;

// 定义按键事件枚举
typedef enum {
    KEY_EVENT_NONE,              // 无事件
    KEY_EVENT_CLICK,             // 按键单击事件
    KEY_EVENT_DOUBLE_CLICK,      // 按键双击事件
    KEY_EVENT_MULTI_CLICK,       // 连击
    KEY_EVENT_LONG_PRESS,        // 长按
    KEY_EVENT_LONG_PRESS_RELEASE // 长按释放
} KEY_Event_t;

typedef enum {
    KEY_LEVEL_LOW = 0,
    KEY_LEVEL_HIGH
} KEY_LEVEL;

// 按键事件结构体
typedef struct {
    KEY_Event_t event;            // 按键事件
    uint8_t volatile click_times; // 连按次数
} KEY_EventData_t;

// 主要函数
void KEY_Server(void);
void FSM_Ticks(uint8_t ticks);

// APIs
void KEY_Member_Remove(uint8_t id);
void KEY_Member_Register(uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t));



bool KEY_GetStatusByID(uint8_t id, KEY_State_t *pointState);
bool KEY_GetEventByID(uint8_t id, KEY_EventData_t *pointEven);

#if CONFIG_USE_FREERTOS
void KEY_TimerCall(void);
void key_task(void *args);
#endif

#endif
