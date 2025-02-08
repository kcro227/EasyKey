# Easy Key

**Easy Key** 是一个基于链表的、适用于 RTOS 的有限状态机按键检测库。它提供了一种简单且高效的方式来检测和处理按键事件，特别适合在资源受限的嵌入式系统中使用。

## 🚀 快速开始

### 1. 状态机核心

1. **添加源代码**：将 Easy Key 的源代码添加到您的项目中，并包含 `key.h` 头文件。
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

使用 `KEY_Member_Register` 函数注册按键。函数原型如下：

```c
void KEY_Member_Register(uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t));
```

示例：

```c
KEY_Member_Register(0, KEY_LEVEL_LOW, "key1", get_gpio_value);
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
#include "key.h"

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
    KEY_Member_Register(0, KEY_LEVEL_LOW, "key1", get_gpio_value);

    while (1) {
        // 处理按键事件
        KEY_Server();

        // 获取按键事件
        KEY_EventData_t keyevent;
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

## 🤝 贡献

欢迎贡献代码和提出建议！请通过 GitHub 提交 Pull Request 或 Issue。

