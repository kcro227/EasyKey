#include "easykey_port.h"
#include <stdio.h>
#include "stm32f4xx.h"
// 用户需要根据实际平台实现以下函数

bool easykey_port_read_key(uint8_t key_id)
{
    // 用户需要根据实际硬件实现按键读取
    // 示例:

    switch (key_id) {
        case 0:
            return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);
        case 1:
            return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);
        case 2:
            return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2);
        default:
            return false;
    }
}

void easykey_port_log(EasyKeyLogLevel level, const char *message)
{
    // 用户可以选择不同的日志输出方式
    // 示例1: 输出到串口
    // printf("[EasyKey] %s\n", message);

    // 示例2: 根据级别输出不同颜色

    switch (level) {
        case EASYKEY_LOG_ERROR:
            printf("\033[1;31m[ERROR] %s\033[0m\n", message); // 红色
            break;
        case EASYKEY_LOG_WARNING:
            printf("\033[1;33m[WARNING] %s\033[0m\n", message); // 黄色
            break;
        case EASYKEY_LOG_INFO:
            printf("\033[1;32m[INFO] %s\033[0m\n", message); // 绿色
            break;
        case EASYKEY_LOG_DEBUG:
            printf("[DEBUG] %s\n", message);
            break;
        default:
            break;
    }
}