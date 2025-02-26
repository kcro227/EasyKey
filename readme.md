# Easy Key

**Easy Key** 是一个基于链表的、适用于 RTOS 的有限状态机按键检测库。它提供了一种简单且高效的方式来检测和处理按键事件，特别适合在资源受限的嵌入式系统中使用。

## 🚀 快速开始

### 1. 状态机核心

1. **添加源代码**：将 Easy Key 的源代码添加到您的项目中，并包含 `Easykey.h` 头文件。
2. **主循环调用**：在主循环中循环调用 `KEY_Server()` 函数，以处理按键事件。
3. **定时器调用**：在定时器中调用 `FSM_Ticks()` 函数，参数为定时器的周期（例如 10ms）。

### 2. 按键读取回调

首先，实现一个按键读取的回调函数，用于读取按键的状态。回调函数的格式如下：

```c
uint32_t get_gpio_value(uint32_t id)
{
    uint32_t value;
    switch (id) {
        case 0:
            value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
            break;
        case 1:
            value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
            break;
        case 2:
            value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10);
            break;
        default:
            break;
    }
    return value;
}
```

### 3. 按键注册

按键注册有两种方式：动态内存申请和静态内存申请。使用哪种方式可以在 `Easykey_config.h` 中的 `CONFIG_DYNAMIC_ALLOCATION` 宏确定，置 1 为使用动态内存申请。

#### 动态内存申请

使用 `KEY_Member_Register` 函数注册按键。函数原型如下：

```c
void KEY_Member_Register(uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t));
```

示例：

```c
KEY_Member_Register(0, KEY_LEVEL_LOW, "key1", get_gpio_value);
```

#### 静态内存申请

使用 `KEY_Member_Register` 函数注册按键。函数原型如下：

```c
void KEY_Member_Register(KEY_t *newNode, uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t));
```

示例：

```c
KEY_t key1_struct; // 声明一个按键结构体
KEY_Member_Register(&key1_struct, 0, KEY_LEVEL_LOW, "key1", get_gpio_value);
```

### 4. 按键事件获取

使用 `KEY_GetEventByID` 函数获取按键事件。函数原型如下：

```c
bool KEY_GetEventByID(uint8_t id, KEY_EventData_t *pointEven);
```

示例：

```c
KEY_EventData_t keyevent;
uint8_t ret = KEY_GetEventByID(0, &keyevent);
if (!ret) {
    LOG_DEBUG("event: id-> 0 event -> %d", keyevent.event);
}
```

## 📝 注意事项

- **定时器周期**：确保定时器的周期与 `FSM_Ticks()` 函数的调用频率一致，以保证按键检测的准确性。
- **硬件配置**：按键读取回调函数应根据实际硬件配置进行实现。

## 🧑‍💻 示例代码

以下是一个完整的示例代码，展示了如何使用 Easy Key 库：

```c
#include "Easykey.h"

uint32_t get_gpio_value(uint32_t id)
{
    uint32_t value;
    switch (id) {
        case 0:
            value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
            break;
        case 1:
            value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
            break;
        case 2:
            value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10);
            break;
        default:
            break;
    }
    return value;
}

void main(void)
{
    // 注册按键
    KEY_t key1_struct; // 声明一个按键结构体
    KEY_Member_Register(&key1_struct, 0, KEY_LEVEL_LOW, "key1", get_gpio_value);

    while (1) {
        // 处理按键事件
        KEY_Server();

        // 获取按键事件
        KEY_Event_t keyevent;
        uint8_t ret = KEY_GetEventByID(0, &keyevent);
        if (!ret) {
            LOG_DEBUG("event: id-> 0 event -> %d", keyevent.event);
        }

        // 其他任务
        // ...
    }
}

void Timer_IRQHandler(void)
{
    // 定时器中断处理
    FSM_Ticks(10);  // 假设定时器周期为 10ms
}
```

## 📊 配置宏说明

以下是 `Easykey_config.h` 中定义的宏及其解释：

| 宏名称                     | 默认值 | 说明                                                                 |
|----------------------------|--------|----------------------------------------------------------------------|
| `CONFIG_DOUBLE_CLICK_TIME`  | 200    | 双击的阈值，单位为毫秒。在这个时间内连续按下按键则会触发双击事件。       |
| `CONFIG_DYNAMIC_ALLOCATION` | 0      | 是否使用动态内存申请。置 1 为使用动态内存申请，否则使用静态内存申请。    |
| `CONFIG_FIFO_BUFFER_SIZE`   | 10     | FIFO 队列的长度，用于存储按键事件。                                     |
| `CONFIG_FREE_RTOS_QUEUE`    | 1      | 是否使用 FreeRTOS 的队列实现按键事件传输。置 1 为使用，否则不使用。      |
| `CONFIG_KEY_DEBUG`          | 0      | 是否启用按键的调试信息。置 1 为启用调试信息，否则不启用。                |
| `CONFIG_LONG_PRESS_TIME`    | 400    | 长按按键的时间阈值，单位为毫秒。超过该时间则触发长按事件。               |
| `CONFIG_TICKS_INTERVAL`     | 10     | 按键扫描的时间间隔，单位为毫秒。                                        |
| `CONFIG_USE_FREERTOS`       | 1      | 是否使用 FreeRTOS 实现按键扫描。置 1 为使用，否则不使用。                |


## 🤝 贡献

欢迎贡献代码和提出建议！请通过 GitHub 提交 Pull Request 或 Issue。
