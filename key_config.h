#pragma once
#ifndef __KEY_CONFIG_H__
#define __KEY_CONFIG_H__

// Auto-generated by menuconfig from bsp/bsp_key/Kconfig

// 双击的阈值，在这个时间内连续按下按键则会触发双击
#define CONFIG_DOUBLE_CLICK_TIME 200

// FIFO的队列长度
#define CONFIG_FIFO_BUFFER_SIZE 10

// 使用FreeRTOS的队列实现按键事件传输
#define CONFIG_FREE_RTOS_QUEUE 0

// 按键的调试信息
#define CONFIG_KEY_DEBUG 1

// key long press time (tick)
#define CONFIG_LONG_PRESS_TIME 400

// 按键扫描的时间间隔，单位：ms
#define CONFIG_TICKS_INTERVAL 10

// 使用FreeRTOS实现按键扫描
#define CONFIG_USE_FREERTOS 1


#endif // __KEY_CONFIG_H__
