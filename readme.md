# Easy Key
这是一个基于链表的，适用于RTOS的有限状态机按键检测库

# 使用
## 状态机核心
首先将源代码放进项目中，然后包含`key.h`
然后在主循环中循环执行`KEY_Server()`函数
在定时器中执行`FSM_Ticks();`函数，参数为定时器的周期。
## 函数回调
首先实现按键读取的回调，格式如下：
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
## 按键注册
函数原型：
```c
void KEY_Member_Register(uint8_t id, KEY_LEVEL key_press_level, char *name, uint32_t (*get_value_func)(uint32_t))
```
如下：
```c
KEY_Member_Register(0, KEY_LEVEL_LOW, "key1", get_gpio_value);

```
## 按键事件获取
函数：
```c
bool KEY_GetEventByID(uint8_t id, KEY_EventData_t *pointEven)
```
示例：
```c
KEY_EventData_t keyevent;
uint8_t ret = KEY_GetEventByID(0,&keyevent);
        if (!ret)
            LOG_DEBUG("event: id-> 0 event -> %d", keyevent.event);
```