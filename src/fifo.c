#include "../inc/fifo.h"

#if CONFIG_USE_FREERTOS
#include "semphr.h"
#endif
// 初始化 FIFO 缓冲区
void FIFO_Init(FIFO_Buffer_t *fifo)
{
    fifo->head  = 0;
    fifo->tail  = 0;
    fifo->count = 0;
#if CONFIG_USE_FREERTOS
    fifo->xMutex = xSemaphoreCreateMutex();
    xSemaphoreGive(fifo->xMutex);
#endif
}

// 向 FIFO 中添加事件
bool FIFO_Push(FIFO_Buffer_t *fifo, KEY_Event_t event)
{

    if (fifo->count >= CONFIG_FIFO_BUFFER_SIZE) {
        return false; // 队列已满
    }
    fifo->buffer[fifo->tail] = event;
    fifo->tail               = (fifo->tail + 1) % CONFIG_FIFO_BUFFER_SIZE;
    fifo->count++;
#if CONFIG_USE_FREERTOS
    xSemaphoreGive(fifo->xMutex);
#endif
    return true;
}

// 从 FIFO 中取出事件
bool FIFO_Pop(FIFO_Buffer_t *fifo, KEY_Event_t *event)
{
    if (fifo->count == 0) {
        return false; // 队列为空
    }

#if CONFIG_USE_FREERTOS
    xSemaphoreTake(fifo->xMutex, portMAX_DELAY);
#endif
    *event     = fifo->buffer[fifo->head];
    fifo->head = (fifo->head + 1) % CONFIG_FIFO_BUFFER_SIZE;
    fifo->count--;
#if CONFIG_USE_FREERTOS
    xSemaphoreGive(fifo->xMutex);
#endif
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
    return fifo->count >= CONFIG_FIFO_BUFFER_SIZE;
}
