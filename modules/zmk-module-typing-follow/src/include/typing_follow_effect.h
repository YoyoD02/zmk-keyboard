// modules/zmk-module-typing-follow/src/include/typing_follow_effect.h
// 头文件：声明打字跟随效果的公共API

#pragma once

#include <zmk/rgb_underglow.h>
#include <zephyr/kernel.h>

/**
 * 打字跟随效果的状态结构体
 * 跟踪每个按键触发的LED动画
 */
struct typing_follow_state {
    uint8_t led_index;      // 要控制的LED索引
    uint8_t current_bright; // 当前亮度值
    uint8_t step;           // 当前步数
    bool active;            // 动画是否激活中
};

/**
 * 初始化打字跟随效果
 * 注册按键事件监听器
 */
int typing_follow_effect_init(void);

/**
 * 触发打字跟随效果
 * 当按键按下时调用此函数
 * 
 * @param position 按键在矩阵中的位置（用于确定点亮哪个区域的LED）
 */
void typing_follow_effect_trigger(uint32_t position);