#include "main.h"
#include "cmsis_os2.h"
#include "Types/LEDType.h"
#include "FreeRTOS.h"


void StartLEDTask(void *argument) {
    for (;;) {
        LEDMessage* message;  // 接收消息的指针
        osMessageQueueGet(LEDQueueHandle, &message, NULL, osWaitForever);  // 阻塞等待消息
        // switch 用法: 根据 message->coler 的值匹配不同的 case
        // 匹配到哪个 case 就从哪里执行，break 防止"穿透"到下一个 case
        switch (message->coler) {
            // 例: 如果 coler == LEDColor_Red (0)，执行下面的语句
            case LEDColor_Red:
                // 三元运算符: state ? 高电平 : 低电平  → state=1开灯，state=0关灯
                HAL_GPIO_WritePin(RedLED_GPIO_Port, RedLED_Pin, message->state ? GPIO_PIN_SET : GPIO_PIN_RESET);
                break;
            case LEDColor_Green:
                HAL_GPIO_WritePin(GreenLED_GPIO_Port, GreenLED_Pin, message->state ? GPIO_PIN_SET : GPIO_PIN_RESET);
                break;
            case LEDColor_Blue:
                HAL_GPIO_WritePin(BlueLED_GPIO_Port, BlueLED_Pin, message->state ? GPIO_PIN_SET : GPIO_PIN_RESET);
                break;
        }
        vPortFree(message);  // 释放消息占用的内存
    }
}
