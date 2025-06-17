#ifndef EASYKEY_PORT_H
#define EASYKEY_PORT_H

#include "easykey.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 读取按键电平状态
 * 
 * @param key_id 按键ID
 * @return true 高电平
 * @return false 低电平
 */
bool easykey_port_read_key(uint8_t key_id);

/**
 * @brief 日志输出函数
 * 
 * @param level 日志级别
 * @param message 日志消息
 */
void easykey_port_log(EasyKeyLogLevel level, const char* message);

#ifdef __cplusplus
}
#endif

#endif /* EASYKEY_PORT_H */