#ifndef _KEY_H
#define _KEY_H
#include "Easykey_config.h"
#include "stdint.h"
#include <stdint.h>
#include <stdbool.h>

#if CONFIG_USE_FREERTOS
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#if CONFIG_DYNAMIC_ALLOCATION
#define malloc pvPortMalloc
#define free   vPortFree
#endif

#endif
// ** ----------按键状态机相关的状态机枚举---------------- **
// 定义按键状态枚举
typedef enum {
    KEY_STATE_RELEASED = 0, // 按键释放状态
    KEY_STATE_DownShake,    // 按键按下抖动状态
    KEY_STATE_PRESSED,      // 按键按下状态
    KEY_STATE_UpShake,      // 按键抬起抖动状态
} KEY_State_t;

typedef enum {
    KEY_EVENT_STATE_IDLE,       // 空闲状态
    KEY_EVENT_STATE_PRESSED,    // 按键按下状态
    KEY_EVENT_STATE_WAIT_MULTI, // 等待连击状态
    KEY_EVENT_STATE_MULTI_CONFIRM,
    KEY_EVENT_STATE_LONG_PRESS // 长按状态
} KEY_EventState_t;
// ** -------------------------------------------------- **

// ** ----------------按键电平枚举----------------------- **
typedef enum {
    KEY_LEVEL_LOW = 0,
    KEY_LEVEL_HIGH
} KEY_LEVEL;
// ** -------------------------------------------------- **

// ** ----------定义按键事件枚举-------------------------- **
typedef enum {
    KEY_EVENT_NONE,              // 无事件
    KEY_EVENT_CLICK,             // 按键单击事件
    KEY_EVENT_DOUBLE_CLICK,      // 按键双击事件
    KEY_EVENT_MULTI_CLICK,       // 连击
    KEY_EVENT_LONG_PRESS,        // 长按
    KEY_EVENT_LONG_PRESS_RELEASE // 长按释放
} KEY_Event_t;
// ** -------------------------------------------------- **

// ** -------------------按键状态结构体------------------ **
typedef struct {
    KEY_State_t state;            // 按键当前状态
    KEY_EventState_t event_state; // 事件状态机状态
    KEY_LEVEL press_level;        // 按键按下的电平
} KEY_Status_t;


// ** ---------------------按键数据结构体------------------ **
typedef struct {

    char *name;            // 按键名
    uint8_t id;            // 按键ID
    uint8_t isLongPressed; // 是否触发长按过
    uint32_t press_time;   // 按键按下的时间
    uint32_t multi_click_count;
    uint32_t wait_double_time;
    KEY_Event_t event;
} KEY_Data_t;


typedef struct {
    KEY_Event_t buffer[CONFIG_FIFO_BUFFER_SIZE]; // 事件缓冲区
    uint8_t head;  // 队列头指针
    uint8_t tail;  // 队列尾指针
    uint8_t count; // 队列中当前事件的数量
#if CONFIG_USE_FREERTOS
    SemaphoreHandle_t xMutex;
#endif
} FIFO_Buffer_t;


typedef struct KEY_t {
    KEY_Status_t status;      // 按键状态
    KEY_Data_t data;          // 按键数据
    FIFO_Buffer_t event_fifo; // FIFO 事件队列
    uint32_t (*get_key_value)(uint32_t key_num);
    struct KEY_t *next; // 指向下一个按键节点的指针
} KEY_t;


// 主要函数
void KEY_Server(void);
#if !CONFIG_USE_FREERTOS
void FSM_Ticks(uint8_t ticks);
#endif
// APIs
#if CONFIG_DYNAMIC_ALLOCATION
void KEY_Member_Register(uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t));
#else
void KEY_Member_Register(KEY_t *newNode, uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t));
#endif
void KEY_Member_Remove(uint8_t id);


bool KEY_GetStatusByID(uint8_t id, KEY_State_t *pointState);
bool KEY_GetEventByID(uint8_t id, KEY_Event_t *pointEven);

#endif
