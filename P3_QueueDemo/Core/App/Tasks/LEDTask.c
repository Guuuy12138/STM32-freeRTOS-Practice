#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "Types/LEDType.h"

// LED控制任务
// 核心逻辑：阻塞等待LEDQueueHandle队列中的LEDMessage，收到后根据消息中的color字段
// 选择对应的GPIO引脚，根据state字段写高/低电平点亮或熄灭LED，最后释放消息内存。
// 没有消息时任务被永久阻塞（osWaitForever），不占CPU时间。

void StartLEDTask(void *argument) {
    for (;;) {
        LEDMessage *message;
        // 阻塞等待消息队列。第三个参数0表示有消息就取、没消息就挂起任务直到有消息到达
        osMessageQueueGet(LEDQueueHandle, &message, 0, osWaitForever);
        // 根据消息中的颜色字段选择对应的LED GPIO
        switch (message->color) {
            case LEDcolor_Red:
                // state非0→GPIO置高→点亮LED；state为0→GPIO置低→熄灭LED
                HAL_GPIO_WritePin(RedLED_GPIO_Port, RedLED_Pin, message->state ? GPIO_PIN_SET : GPIO_PIN_RESET);
                break;
            case LEDcolor_Blue:
                HAL_GPIO_WritePin(BlueLED_GPIO_Port, BlueLED_Pin, message->state ? GPIO_PIN_SET : GPIO_PIN_RESET);
                break;
            case LEDcolor_Green:
                HAL_GPIO_WritePin(GreenLED_GPIO_Port, GreenLED_Pin, message->state ? GPIO_PIN_SET : GPIO_PIN_RESET);
                break;
        }
        // 消息由KeyTask在堆上申请，这里消费完后归还内存，防止内存泄漏
        vPortFree(message);
    }
}