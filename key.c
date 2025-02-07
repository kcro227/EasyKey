#include "key.h"
#include "log.h"
#include "stm32f10x.h"

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

void Key_GpioInit()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure = {
        .GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10,
        .GPIO_Mode  = GPIO_Mode_IPU,
        .GPIO_Speed = GPIO_Speed_50MHz};
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void key_task(void *args)
{
    Key_GpioInit();
    KEY_Member_Register(0, KEY_LEVEL_LOW, "key1", get_gpio_value);
    KEY_Member_Register(1, KEY_LEVEL_LOW, "key2", get_gpio_value);
    KEY_Member_Register(2, KEY_LEVEL_LOW, "key3", get_gpio_value);

    TimerHandle_t key_tick_handle = xTimerCreate("key_tick", 10, pdTRUE, (void *)1, (TimerCallbackFunction_t)KEY_TimerCall);
    xTimerStart(key_tick_handle, 0);

    while (1) {
        KEY_Server();
        KEY_EventData_t keyevent;
        uint8_t ret = KEY_GetEventByID(0,&keyevent);
        if (!ret)
            LOG_DEBUG("event: id-> 0 event -> %d", keyevent.event);
    }
}
