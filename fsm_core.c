#include "./inc/fsm_core.h"


uint8_t FSM_Scan_Count;
uint32_t global_tick;
KEY_t *head;

/**
 * @brief 放在定时任务中，为状态机提供心跳
 * @param ticks 每次执行经过的时间 单位:ms
 *  */
void FSM_Ticks(uint8_t ticks)
{
    FSM_Scan_Count += ticks;
    global_tick += ticks; // 更新全局时间戳
}

/***
 * @brief FreeRTOS下的软件定时器回调函数
 */
void KEY_TimerCall(void)
{
    FSM_Ticks(10);
}

// 初始化链表
void KEY_List_Init()
{
    head = NULL; // 初始化头节点为空
}

/**
 * @brief 添加节点到链表
 *  */
void KEY_List_Add(uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t))
{
    // 创建新节点
    KEY_t *newNode = (KEY_t *)malloc(sizeof(KEY_t));
    if (newNode == NULL) {
        // 内存分配失败
        LOG_ERROR("malloc faild!");
        return;
    }

    // 初始化新节点的数据

    newNode->status.press_level = key_press_level;
    newNode->status.state       = KEY_STATE_RELEASED;
    newNode->status.cnt         = 0;
    newNode->status.isPressed   = false;

    newNode->data.name     = name;
    newNode->data.id       = id;
    newNode->get_key_value = get_value_func;
    newNode->next          = NULL;

    // 如果链表为空，新节点作为头节点
    if (head == NULL) {
        head = newNode;
    } else {
        // 否则，将新节点添加到链表末尾
        KEY_t *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

// 从链表中删除节点
void KEY_List_Remove(uint8_t id)
{
    KEY_t *current  = head;
    KEY_t *previous = NULL;

    // 遍历链表，查找要删除的节点
    while (current != NULL && current->data.id != id) {
        previous = current;
        current  = current->next;
    }

    // 如果找到要删除的节点
    if (current != NULL) {
        // 如果要删除的是头节点
        if (previous == NULL) {
            head = current->next;
        } else {
            // 否则，将前一个节点的next指向当前节点的next
            previous->next = current->next;
        }

        // 释放当前节点的内存
        free(current);
    }
}

uint32_t KEY_GetStatusByID(uint8_t id)
{
    KEY_t *current = head;

    // 遍历链表，查找id所在的节点
    while (current != NULL && current->data.id != id) {
        current = current->next;
    }
    // 如果找到节点
    if (current != NULL) {
        return current->status.state;
    } else
        return 255;
}

// **********************************
// 初始化 FIFO 缓冲区
void FIFO_Init(FIFO_Buffer_t *fifo)
{
    fifo->head  = 0;
    fifo->tail  = 0;
    fifo->count = 0;
}

// 向 FIFO 中添加事件
bool FIFO_Push(FIFO_Buffer_t *fifo, KEY_Event_t event)
{
    if (fifo->count >= FIFO_BUFFER_SIZE) {
        return false; // 队列已满
    }
    fifo->buffer[fifo->tail] = event;
    fifo->tail               = (fifo->tail + 1) % FIFO_BUFFER_SIZE;
    fifo->count++;
    return true;
}

// 从 FIFO 中取出事件
bool FIFO_Pop(FIFO_Buffer_t *fifo, KEY_Event_t *event)
{
    if (fifo->count == 0) {
        return false; // 队列为空
    }
    *event     = fifo->buffer[fifo->head];
    fifo->head = (fifo->head + 1) % FIFO_BUFFER_SIZE;
    fifo->count--;
    return true;
}

// 检查 FIFO 是否为空
bool FIFO_IsEmpty(FIFO_Buffer_t *fifo)
{
    return fifo->count == 0;
}

// 检查 FIFO 是否已满
bool FIFO_IsFull(FIFO_Buffer_t *fifo)
{
    return fifo->count >= FIFO_BUFFER_SIZE;
}

// **********************************

/***
 * @brief 按键的状态扫描
 */
void FSM_KeyStateScan(void)
{
    // 以链表结构设置按键，从链表第一个按键开始扫描
    KEY_t *point = head;

    while (point != NULL) {
        switch (point->status.state) {
            case KEY_STATE_RELEASED: {
                if ((point->get_key_value(point->data.id)) == (point->status.press_level)) {
                    point->status.state = KEY_STATE_DownShake;
                } else {
                    point->status.state = KEY_STATE_RELEASED;
                }
                break;
            }
            case KEY_STATE_DownShake: {
                if ((point->get_key_value(point->data.id)) == (point->status.press_level)) {
                    point->status.state = KEY_STATE_PRESSED;
                } else {
                    point->status.state = KEY_STATE_RELEASED;
                }
                break;
            }
            case KEY_STATE_PRESSED: {
                if ((point->get_key_value(point->data.id)) == (point->status.press_level)) {
                    point->status.state = KEY_STATE_PRESSED;
                } else {
                    point->status.state = KEY_STATE_UpShake;
                }
                break;
            }
            case KEY_STATE_UpShake: {
                if ((point->get_key_value(point->data.id)) == (point->status.press_level)) {
                    point->status.state = KEY_STATE_PRESSED;
                } else {
                    point->status.state = KEY_STATE_RELEASED;
                }
                break;
            }
        }
        point = point->next;
    }
}

/***
 * @brief 按键事件的判断
 *
 */

void FSM_KeyEventScan(void)
{
    // 以链表结构设置按键，从链表第一个按键开始扫描
    KEY_t *point = head;

    while (point != NULL) {
        switch (point->event_state) {
            case KEY_EVENT_STATE_IDLE: {
                // 空闲状态，等待按键按下
                if (point->status.state == KEY_STATE_PRESSED) {
                    // 按键按下，进入按下状态
                    point->event_state = KEY_EVENT_STATE_PRESSED;
                    point->data.press_time = 0; // 重置按压时间
                    point->data.isLongPressed = false; // 重置长按标志
                    point->data.multi_click_count = 0; // 重置多击计数
                    LOG_DEBUG("Key %d: Pressed, goto press state", point->data.id);
                }
                break;
            }
            case KEY_EVENT_STATE_PRESSED: {
                // 按键按下状态
                if (point->status.state == KEY_STATE_PRESSED) {
                    // 持续按压，增加按压时间
                    point->data.press_time += CONFIG_TICKS_INTERVAL;

                    // 判断是否达到长按时间
                    if (point->data.press_time >= CONFIG_LONG_PRESS_TIME) {
                        // 进入长按状态
                        point->event_state = KEY_EVENT_STATE_LONG_PRESS;
                        LOG_DEBUG("Key %d: Long press detected, goto long press state", point->data.id);
                    }
                } else {
                    // 按键释放，进入等待双击状态
                    point->event_state = KEY_EVENT_STATE_WAIT_DOUBLE;
                    point->data.wait_double_time = 0; // 重置等待双击时间
                    LOG_DEBUG("Key %d: Released, goto wait double state", point->data.id);
                }
                break;
            }
            case KEY_EVENT_STATE_WAIT_DOUBLE: {
                // 等待双击状态
                point->data.wait_double_time += CONFIG_TICKS_INTERVAL;

                if (point->status.state == KEY_STATE_PRESSED) {
                    // 再次按下，进入等待多击状态
                    point->event_state = KEY_EVENT_STATE_WAIT_MULTI;
                    point->data.multi_click_count = 1; // 设置多击计数为1
                    point->data.wait_double_time = 0; // 重置等待双击时间
                    LOG_DEBUG("Key %d: Double click detected, goto wait multi state", point->data.id);
                } else if (point->data.wait_double_time >= CONFIG_DOUBLE_CLICK_TIME) {
                    // 超过双击时间窗口，触发单击事件
                    LOG_DEBUG("Key %d: Single click detected", point->data.id);
                    point->event_state = KEY_EVENT_STATE_IDLE; // 返回空闲状态
                }
                break;
            }
            case KEY_EVENT_STATE_WAIT_MULTI: {
                // 等待多击状态
                point->data.wait_double_time += CONFIG_TICKS_INTERVAL;

                if (point->status.state == KEY_STATE_PRESSED) {
                    // 按键按下，进入多击确认状态
                    point->event_state = KEY_EVENT_STATE_MULTI_CONFIRM;
                    LOG_DEBUG("Key %d: Multi-click detected, goto multi confirm state", point->data.id);
                } else if (point->data.wait_double_time >= CONFIG_DOUBLE_CLICK_TIME) {
                    // 超过多击时间窗口，触发多击事件
                    LOG_DEBUG("Key %d: Multi-click event triggered (count: %d)", point->data.id, point->data.multi_click_count);
                    point->event_state = KEY_EVENT_STATE_IDLE; // 返回空闲状态
                }
                break;
            }
            case KEY_EVENT_STATE_MULTI_CONFIRM: {
                // 多击确认状态
                if (point->status.state == KEY_STATE_RELEASED) {
                    // 按键释放，增加多击计数
                    point->data.multi_click_count++;
                    point->event_state = KEY_EVENT_STATE_WAIT_MULTI; // 返回等待多击状态
                    point->data.wait_double_time = 0; // 重置等待双击时间
                    LOG_DEBUG("Key %d: Multi-click confirmed (count: %d)", point->data.id, point->data.multi_click_count);
                }
                break;
            }
            case KEY_EVENT_STATE_LONG_PRESS: {
                // 长按状态
                if (!point->data.isLongPressed) {
                    // 触发长按事件
                    LOG_DEBUG("Key %d: Long press event triggered", point->data.id);
                    point->data.isLongPressed = true; // 设置长按标志

                    // 根据多击计数触发相应的事件
                    if (point->data.multi_click_count == 0) {
                        // 触发单击事件
                        LOG_DEBUG("Key %d: Single click event triggered", point->data.id);
                    } else if (point->data.multi_click_count == 1) {
                        // 触发双击事件
                        LOG_DEBUG("Key %d: Double click event triggered", point->data.id);
                    } else {
                        // 触发多击事件
                        LOG_DEBUG("Key %d: Multi-click event triggered (count: %d)", point->data.id, point->data.multi_click_count);
                    }
                }

                if (point->status.state == KEY_STATE_RELEASED) {
                    // 按键释放，触发长按释放事件
                    LOG_DEBUG("Key %d: Long press release event triggered", point->data.id);
                    point->event_state = KEY_EVENT_STATE_IDLE; // 返回空闲状态
                    point->data.isLongPressed = false; // 重置长按标志
                }
                break;
            }
            default:
                break;
        }

        point = point->next;
    }
}

/***
 * @brief 按键扫描服务函数
 */
void KEY_Server(void)
{
    if (FSM_Scan_Count >= CONFIG_TICKS_INTERVAL) {
        // 达到消抖间隔，开始扫描按键状态
        FSM_Scan_Count = 0;
        FSM_KeyStateScan();
        FSM_KeyEventScan();
    }
}
