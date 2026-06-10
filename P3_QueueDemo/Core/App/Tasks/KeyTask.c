#include <sys/types.h>
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "Types/LEDType.h"

// 按键检测任务
// 核心逻辑：每10ms轮询一次Key1引脚，软件消抖后若确认按下，则分配一个LEDMessage
// 通过osMessageQueuePut发给LED任务。每次按下只触发一次（按下不放不会重复发送）。

// 判断按键是否被按下：读到低电平就是按下（按键一端接GPIO、一端接地，按下导通拉低）
#define IS_KEY_PRESSED()   (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_RESET)

// 按键参数
#define KEY_CHECK_INTERVAL 10   // 轮询间隔：每10ms检查一次按键电平
#define KEY_DEBOUNCE_TIME  30   // 消抖窗口：30ms。机械按键触点抖动通常在几十ms内结束
// 30ms÷10ms=3次。连续3次都读到低电平才认定为有效按下，不用硬件定时器，纯软件计数即可
#define KEY_DEBOUNCE_COUNT (KEY_DEBOUNCE_TIME / KEY_CHECK_INTERVAL)  // =3

// 检测Key1是否被单击（带软件消抖）
// 原理：用两个static变量跟踪状态——count记录连续检测到低电平的次数，pressed标记本次按下是否已被确认。
//       消抖通过"连续N次读到低电平"实现，而不是两次读取之间等30ms，这样不会阻塞任务循环。
// 返回值：1表示检测到一次有效单击，0表示无按键或按键尚未确认
uint8_t isKey1Clicked(void) {
    static uint8_t count = 0;   // 连续低电平计数（每次+1，松开清零）
    static uint8_t pressed = 0; // 按下已确认标志（防止按住时反复触发）

    // 按键正被按着(低电平) 且 本轮尚未确认按下
    if (IS_KEY_PRESSED() && !pressed) {
        count++;
        // 消抖判断：次数够了且当前仍按着 → 认定有效按下
        if (count >= KEY_DEBOUNCE_COUNT && IS_KEY_PRESSED()) {
            pressed = 1;   // 标记已确认，防止下一轮再进这个分支
            return 1;      // 返回"有按键"
        }
    }

    // 按键松开了(高电平)，重置所有状态，为下一次按下做准备
    if (!IS_KEY_PRESSED()) {
        pressed = 0;
        count = 0;
        return 0;
    }
    return 0;
}

// 按键任务入口（FreeRTOS自动创建）
// 循环流程：调用isKey1Clicked检测按键 → 检测到了就创建LEDMessage放入队列 → osDelay 10ms
// 消息在堆上分配(pvPortMalloc)，由LED任务消费后释放(vPortFree)，不在这里释放
void StartKeyTask(void *argument) {
    LEDstate state = LEDstate_Off;//初始状态设为关闭
    for (;;) {
        if (isKey1Clicked()) {
            state = !state;//反转状态
            // 在堆上申请消息内存，填充红色+关灯状态，发送到LED队列（阻塞等待直到队列有空位）
            // 声明指针message，在堆上申请一块sizeof(LEDMessage)大小的内存，首地址赋给message
            LEDMessage* message = pvPortMalloc(sizeof(LEDMessage));
            message->color = LEDcolor_Red;
            message->state = state;
            //osMessageQueuePut：FreeRTOS消息队列发送函数 LEDQueueHandle：LED队列句柄
            //message：要发送的消息指针 0：消息优先级 osWaitForever：阻塞等待（永久等待）
            osMessageQueuePut(LEDQueueHandle, (&message), 0, osWaitForever);
        }
        osDelay(KEY_CHECK_INTERVAL);  // 延时10ms，同时也是按键轮询的周期
    }
}