#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "Types/LEDType.h"

#define IS_KEY_PRESSED()(HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin) == GPIO_PIN_RESET)//按键按下函数
#define KEY_CHECK_INTERVAL 10//按键检查间隔
#define KEY_DEBOUNCE_TIME 30//按键消抖时间
#define KEY_DEBOUNCE_COUNT (KEY_DEBOUNCE_TIME / KEY_CHECK_INTERVAL)//按键消抖次数 = 3

//按键检测函数；第一个if：按键按下 | 第二个if：按键抬起并返回一
uint8_t isKey1Clicked() {
    static uint8_t count = 0;
    static uint8_t pressed =0;
    //按键按下 | IS_KEY_PRESSED():检测按键是否按下的函数 && !pressed: 按键标志位为0表示按键被按下
    if (IS_KEY_PRESSED() && !pressed) {
        count++;//按键按下次数加1
        //count>=应该消抖次数3 && 按键还在被按下
        if (count>=KEY_DEBOUNCE_COUNT && IS_KEY_PRESSED()) {
            pressed = 1;//按键标志位设置为1表示被按下
            return 1;
        }
    }
    //!IS_KEY_PRESSED():取反IS_KEY_PRESSED函数 即HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin) == GPIO_PIN_SET
    if (!IS_KEY_PRESSED()) {
        pressed = 0;//按键标志位设置为0表示被抬起
        count = 0;//按键次数归零
    }
    return 0;
}



void StartKeyTask(void *argument) {
    LEDState state = LEDstate_Off;
    for (;;) {
        if (isKey1Clicked()) {
            state = !state;
            LEDMessage* message = pvPortMalloc(sizeof(LEDMessage));
            message->coler = LEDColor_Red;
            message->state = state;
            osMessageQueuePut(LEDQueueHandle, (&message), 0, osWaitForever);
        }
        osDelay(10);
    }
}

