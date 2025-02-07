#ifndef _FIFO_H
#define _FIFO_H

#include "fsm_core.h"

#if 1
// FIFO
void FIFO_Init(FIFO_Buffer_t *fifo);
bool FIFO_Push(FIFO_Buffer_t *fifo, KEY_Event_t event);
bool FIFO_Pop(FIFO_Buffer_t *fifo, KEY_Event_t *event);
bool FIFO_IsEmpty(FIFO_Buffer_t *fifo);
bool FIFO_IsFull(FIFO_Buffer_t *fifo);

#endif

#endif
