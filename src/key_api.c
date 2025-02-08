#include "fsm_core.h"
#include "string.h"
#include "fifo.h"

KEY_t *g_keyListHead = NULL;

/**
 * @brief 添加节点到链表
 *  */
void KEY_Member_Register(uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t))
{
    // 创建新节点
    KEY_t *newNode = (KEY_t *)malloc(sizeof(KEY_t));
    if (newNode == NULL) {
        // 内存分配失败
        LOG_ERROR("malloc faild!");
        return;
    }
    
    // 初始化新节点的数据
    memset(newNode, 0, sizeof(KEY_t));
    
    FIFO_Init(&newNode->event_fifo);
    newNode->status.press_level = key_press_level;
    newNode->status.state       = KEY_STATE_RELEASED;
    newNode->status.cnt         = 0;
    newNode->status.isPressed   = false;

    newNode->data.name     = name;
    newNode->data.id       = id;
    newNode->get_key_value = get_value_func;
    newNode->next          = NULL;

    // 如果链表为空，新节点作为头节点
    if (g_keyListHead == NULL) {
        g_keyListHead = newNode;
    } else {
        // 否则，将新节点添加到链表末尾
        KEY_t *current = g_keyListHead;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

// 从链表中删除节点
void KEY_Member_Remove(uint8_t id)
{
    KEY_t *current  = g_keyListHead;
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
            g_keyListHead = current->next;
        } else {
            // 否则，将前一个节点的next指向当前节点的next
            previous->next = current->next;
        }

        // 释放当前节点的内存
        free(current);
    }
}

bool KEY_GetStatusByID(uint8_t id, KEY_State_t *pointState)
{
    KEY_t *current = g_keyListHead;

    // 遍历链表，查找id所在的节点
    while (current != NULL && current->data.id != id) {
        current = current->next;
    }
    // 如果找到节点
    if (current != NULL) {
        *pointState = current->status.state;
        return 0;
    } else
        return 1;
}

bool KEY_GetEventByID(uint8_t id, KEY_EventData_t *pointEven)
{
    KEY_t *current = g_keyListHead;

    // 遍历链表，查找id所在的节点
    while (current != NULL && current->data.id != id) {
        current = current->next;
    }
    // 如果找到节点
    if (current != NULL) {
        if (FIFO_Pop(&current->event_fifo, &pointEven->event))
            return 0;
        else {
            pointEven->event = KEY_EVENT_NONE;
            return 2;
        }
    } else
        return 1;
}
