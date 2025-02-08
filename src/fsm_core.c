#include "fsm_core.h"
#include "fifo.h"
uint8_t g_sanTicks;
uint32_t g_ticks;
extern KEY_t *g_keyListHead;

/**
 * @brief 放在定时任务中，为状态机提供心跳
 * @param ticks 每次执行经过的时间 单位:ms
 *  */
void FSM_Ticks(uint8_t ticks)
{
    g_sanTicks += ticks;
    g_ticks += ticks; // 更新全局时间戳
}

#if CONFIG_USE_FREERTOS
/***
 * @brief FreeRTOS下的软件定时器回调函数
 */
void KEY_TimerCall(void)
{
    FSM_Ticks(10);
}
#endif

/***
 * @brief 按键的状态扫描
 */
void FSM_KeyStateScan(void)
{
    // 以链表结构设置按键，从链表第一个按键开始扫描
    KEY_t *point = g_keyListHead;

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
    KEY_t *point = g_keyListHead;

    while (point != NULL) {
        switch (point->status.event_state) {
            case KEY_EVENT_STATE_IDLE: {
                // 空闲状态，等待按键按下
                point->data.event.event = KEY_EVENT_NONE;
                if (point->status.state == KEY_STATE_PRESSED) {
                    // 按键按下，进入按下状态
                    point->status.event_state     = KEY_EVENT_STATE_PRESSED;
                    point->data.press_time        = 0;     // 重置按压时间
                    point->data.isLongPressed     = false; // 重置长按标志
                    point->data.multi_click_count = 0;     // 重置多击计数
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
                        point->status.event_state = KEY_EVENT_STATE_LONG_PRESS;
                    }
                } else {
                    // 按键释放，进入等待双击状态
                    point->status.event_state     = KEY_EVENT_STATE_WAIT_MULTI;
                    point->data.wait_double_time  = 0; // 重置等待双击时间
                    point->data.multi_click_count = 1; // 设置多击计数为1
                    // LOG_DEBUG("Key %d: Released, goto wait double state", point->data.id);
                }
                break;
            }
            case KEY_EVENT_STATE_WAIT_MULTI: {
                // 等待多击状态
                point->data.wait_double_time += CONFIG_TICKS_INTERVAL;

                if (point->status.state == KEY_STATE_PRESSED) {
                    // 按键按下，进入多击确认状态
                    point->status.event_state = KEY_EVENT_STATE_MULTI_CONFIRM;
                } else if (point->data.wait_double_time >= CONFIG_DOUBLE_CLICK_TIME) {
// 超过多击时间窗口，触发多击事件
#if CONFIG_KEY_DEBUG
                    LOG_DEBUG("Key %d: Multi-click event triggered (count: %d)", point->data.id, point->data.multi_click_count);
#endif
                    if (point->data.multi_click_count == 1) {
                        point->data.event.event       = KEY_EVENT_CLICK;
                        point->data.event.click_times = 1;
                        FIFO_Push(&(point->event_fifo), KEY_EVENT_CLICK);
                    } else if (point->data.multi_click_count == 2) {
                        point->data.event.event       = KEY_EVENT_DOUBLE_CLICK;
                        point->data.event.click_times = 2;
                        FIFO_Push(&point->event_fifo, KEY_EVENT_DOUBLE_CLICK);
                    } else {
                        point->data.event.event       = KEY_EVENT_MULTI_CLICK;
                        point->data.event.click_times = point->data.multi_click_count;
                        FIFO_Push(&point->event_fifo, KEY_EVENT_MULTI_CLICK);
                    }
                    point->status.event_state = KEY_EVENT_STATE_IDLE; // 返回空闲状态
                }
                break;
            }
            case KEY_EVENT_STATE_MULTI_CONFIRM: {
                // 多击确认状态
                if (point->status.state == KEY_STATE_RELEASED) {
                    // 按键释放，增加多击计数
                    point->data.multi_click_count++;
                    point->status.event_state    = KEY_EVENT_STATE_WAIT_MULTI; // 返回等待多击状态
                    point->data.wait_double_time = 0;                          // 重置等待双击时间
                }
                break;
            }
            case KEY_EVENT_STATE_LONG_PRESS: {
                // 长按状态
                if (!point->data.isLongPressed) {
// 触发长按事件
#if CONFIG_KEY_DEBUG
                    LOG_DEBUG("Key %d: Long press event triggered", point->data.id);
#endif
                    point->data.event.event       = KEY_EVENT_LONG_PRESS;
                    point->data.event.click_times = 0;
                    point->data.isLongPressed     = true; // 设置长按标志

                    FIFO_Push(&point->event_fifo, KEY_EVENT_LONG_PRESS);
                }

                if (point->status.state == KEY_STATE_RELEASED) {
// 按键释放，触发长按释放事件
#if CONFIG_KEY_DEBUG
                    LOG_DEBUG("Key %d: Long press release event triggered", point->data.id);
#endif
                    point->data.event.event       = KEY_EVENT_LONG_PRESS_RELEASE;
                    point->data.event.click_times = 0;
                    point->status.event_state     = KEY_EVENT_STATE_IDLE; // 返回空闲状态
                    point->data.isLongPressed     = false;                // 重置长按标志

                    FIFO_Push(&point->event_fifo, KEY_EVENT_LONG_PRESS_RELEASE);
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

#if CONFIG_USE_FREERTOS
    FSM_KeyStateScan();
    FSM_KeyEventScan();
    vTaskDelay(pdMS_TO_TICKS(CONFIG_TICKS_INTERVAL));
#else
    if (g_sanTicks >= CONFIG_TICKS_INTERVAL) {
        // 达到消抖间隔，开始扫描按键状态
        g_sanTicks = 0;
        FSM_KeyStateScan();
        FSM_KeyEventScan();
    }
#endif
}
