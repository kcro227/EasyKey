#ifndef __FSM_CORE_H
#define __FSM_CORE_H

#include "key.h"

// 按键状态结构体
typedef struct {
    KEY_State_t state;     // 按键当前状态
    KEY_LEVEL press_level; // 按键按下的电平
    uint32_t volatile cnt; // 定时器，用于消抖或长按检测
    uint32_t volatile recnt;
    bool isPressed; // 按键是否按下
} KEY_Status_t;

// 按键事件结构体
typedef struct {
    KEY_Event_t event;            // 按键事件
    uint8_t volatile click_times; // 连按次数

} KEY_EventData_t;

// 按键数据结构体
typedef struct {
    uint8_t id;            // 按键ID
    char *name;            // 按键名
    uint8_t isLongPressed; // 是否触发长按过
    uint32_t press_time;   // 按键按下的时间
    uint32_t multi_click_count;
    uint32_t wait_double_time;
    uint32_t press_interval; // 两次按压的时间间隔
} KEY_Data_t;

#define FIFO_BUFFER_SIZE 10 // 定义 FIFO 缓冲区的大小

typedef struct {
    KEY_Event_t buffer[FIFO_BUFFER_SIZE]; // 事件缓冲区
    uint8_t head;                         // 队列头指针
    uint8_t tail;                         // 队列尾指针
    uint8_t count;                        // 队列中当前事件的数量
} FIFO_Buffer_t;

typedef struct KEY_t {
    KEY_Status_t status;          // 按键状态
    KEY_Data_t data;              // 按键数据
    FIFO_Buffer_t event_fifo;     // FIFO 事件队列
    KEY_EventState_t event_state; // 事件状态机状态
    uint32_t (*get_key_value)(uint32_t key_num);
    struct KEY_t *next; // 指向下一个按键节点的指针

} KEY_t;
#endif
