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
    KEY_EVENT_NONE,         // 无事件
    KEY_EVENT_PRESSED,      // 按键按下事件
    KEY_EVENT_RELEASED,     // 按键释放事件
    KEY_EVENT_CLICK,        // 按键单击事件
    KEY_EVENT_DOUBLE_CLICK, // 按键双击事件
    KEY_EVENT_MULTI_CLICK
} KEY_Event_t;

typedef enum {
    KEY_EVENT_STATE_IDLE,        // 空闲状态
    KEY_EVENT_STATE_PRESSED,     // 按键按下状态
    KEY_EVENT_STATE_WAIT_DOUBLE, // 等待双击状态
    KEY_EVENT_STATE_WAIT_MULTI,  // 等待连击状态
    KEY_EVENT_STATE_MULTI_CONFIRM,
    KEY_EVENT_STATE_LONG_PRESS // 长按状态
} KEY_EventState_t;

typedef enum {
    KEY_LEVEL_LOW = 0,
    KEY_LEVEL_HIGH
} KEY_LEVEL;

void KEY_Server(void);
void KEY_List_Remove(uint8_t id);
void KEY_List_Add(uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t));
void KEY_List_Init(void);
uint32_t KEY_GetStatusByID(uint8_t id);

#if CONFIG_USE_FREERTOS
void KEY_TimerCall(void);
void key_task(void *args);
#endif

#endif
