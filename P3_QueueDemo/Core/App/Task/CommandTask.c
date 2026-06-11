#include <sys/types.h>
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "usart.h"
#include "Types/LEDType.h"


/* ── 串口协议定义 ────────────────────────────────────────
 * 帧格式: [0xAA] [长度] [数据...] [校验和]
 *   0xAA    — 帧头标识
 *   长度     — 整帧总字节数 (含帧头和校验和), 范围 4~30
 *   数据     — 每 2 字节一对: [颜色] [状态], 颜色取值 1/2/3
 *   校验和   — 前面所有字节累加和的低 8 位
 * ───────────────────────────────────────────────────── */

// 串口命令处理任务 — 从队列取字节 → 按协议拼帧 → 校验 → 解析成 LED 指令发给 LED 队列
void StartCommandTask(void *argument) { // argument: FreeRTOS 任务参数 (本任务未使用)
    UART2_Receive_Start();             // 启动 USART2 中断接收，之后每收到 1 字节都会进队列

    uint8_t receive;                   // 从队列取出的单个字节数据
    uint8_t command[30];               // 帧缓冲区，暂存一帧的所有字节直到校验
    uint8_t commandIndex = 0;          // 当前帧已写入的字节数 / 下一个写入位置
    uint8_t commandLength = 0;         // 协议中声明的整帧总长度 (含帧头、长度、数据、校验和)

    for (;;) {                         // FreeRTOS 任务主循环，永不退出
        // 阻塞等待串口队列，直到有数据可读才返回
        osMessageQueueGet(CommandQueueHandle, &receive, 0, osWaitForever);

        if (commandIndex == 0) {       // ── 阶段 1: 当前还没有收到帧头 ──
            if (receive == 0xAA) {     // 0xAA 是帧头标识
                command[commandIndex++] = receive; // 把 0xAA 存入 buffer[0]，commandIndex 变为 1
            }
            // 不是 0xAA 就丢弃，继续等
        }else if (commandIndex == 1) { // ── 阶段 2: 帧头已收到，等待长度字节 ──
            if (receive < 4 || receive > sizeof(command)) { // 长度非法 (<4 帧太短 或 >30 超出缓冲区)
                commandIndex = 0;      // 放弃当前帧，回到阶段 1 重新等帧头
                continue;              // 跳过本次循环，去取下一个字节
            }
            commandLength = receive;    // 记录协议声明的整帧长度
            command[commandIndex++] = receive; // 把长度存入 buffer[1]，commandIndex 变为 2
        }else {                        // ── 阶段 3: 帧头和长度已收到，接收剩余的数据字节 ──
            command[commandIndex++] = receive; // 存入当前字节，commandIndex 自增

            if (commandIndex == commandLength) { // 已收满一帧，开始校验
                uint8_t checksum = 0;  // 累加校验和，初始为 0
                for (uint8_t i = 0; i < commandLength - 1; i++) { // 遍历除最后一个字节外的所有字节
                    checksum += command[i]; // 累加，溢出自动取低 8 位
                }
                if (checksum == command[commandLength - 1]) { // 校验和 == 帧末尾的校验字节
                    // 校验通过，解析有效载荷: 从 command[2] 开始，跳过头和长度
                    for (uint8_t i = 2; i < commandLength - 2; i += 2) { // 每 2 字节一组
                        LEDMessage* message = pvPortMalloc(sizeof(LEDMessage)); // 在堆上分配 LEDMessage
                        message->coler = command[i] - 1;     // 第 1 字节: 颜色 (协议传 1/2/3 → 枚举 0/1/2)
                        message->state = command[i + 1];     // 第 2 字节: 亮灭状态 (0=关, 1=开)
                        osMessageQueuePut(LEDQueueHandle, &message, 0, osWaitForever); // 发送到 LED 队列
                    }
                }
                // 无论校验成功与否，帧处理完毕，复位准备收下一帧
                commandIndex = 0;      // 缓冲区写指针归零
                commandLength = 0;     // 帧长度清零
            }
        }
    }
}