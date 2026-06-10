//
// Created by G on 2026/6/10.
//
// LED消息类型定义
// LEDMessage 是按键任务和LED任务之间通过队列传递的数据包，包含颜色和开关状态两个字段

#ifndef P3_QUEUEDEMO_LEDTYPE_H
#define P3_QUEUEDEMO_LEDTYPE_H

// LED颜色枚举，目前硬件上接了红、绿、蓝三颗LED
typedef enum {
    LEDcolor_Red = 0,
    LEDcolor_Green = 1,
    LEDcolor_Blue = 2,
}LEDcolor;

// LED开关状态枚举
typedef enum {
    LEDstate_Off = 0,
    LEDstate_On = 1,
}LEDstate;

// 队列消息结构体：告诉LED任务"哪颗灯"该"亮还是灭"
typedef struct {
    LEDcolor color;   // 目标LED颜色
    LEDstate state;   // 目标状态（开/关）
}LEDMessage;

#endif //P3_QUEUEDEMO_LEDTYPE_H