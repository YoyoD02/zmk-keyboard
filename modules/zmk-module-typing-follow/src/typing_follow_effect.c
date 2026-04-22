// modules/zmk-module-typing-follow/src/typing_follow_effect.c
// 打字跟随效果的核心实现

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/rgb_underglow.h>
#include "typing_follow_effect.h"

LOG_MODULE_REGISTER(typing_follow, LOG_LEVEL_DBG);

// 静态变量定义
#define MAX_ACTIVE_EFFECTS 10      // 同时最多活跃的效果数
#define LED_STRIP_LENGTH 68        // LED数量，根据你的键盘修改
#define FADE_STEPS 10              // 淡出步数
#define FADE_STEP_DELAY_MS 30      // 每步延迟30ms

// 活跃效果池
static struct typing_follow_state active_effects[MAX_ACTIVE_EFFECTS];
static struct k_work_delayable fade_work;  // 延迟工作队列

/**
 * 将按键位置映射到LED索引
 * 这里实现简单的区域映射：根据按键位置点亮对应的LED
 * 你可以根据实际键盘布局调整此映射逻辑
 * 
 * @param position 按键位置（矩阵扫描码）
 * @return LED索引 (0-67)
 */
static uint8_t map_position_to_led(uint32_t position) {
    // 简单映射：将按键位置均匀映射到LED范围
    // 68个LED，假设键盘有68个按键（1:1映射）
    // 实际键盘可能有更多按键，这里取模处理
    return position % LED_STRIP_LENGTH;
}

/**
 * 查找或分配一个新的效果槽位
 */
static struct typing_follow_state* find_free_slot(void) {
    for (int i = 0; i < MAX_ACTIVE_EFFECTS; i++) {
        if (!active_effects[i].active) {
            return &active_effects[i];
        }
    }
    return NULL;  // 无可用槽位
}

/**
 * 淡出动画的工作函数
 * 由定时器触发，逐步降低LED亮度
 */
static void fade_work_handler(struct k_work *work) {
    static uint8_t fade_counter = 0;
    bool all_complete = true;
    
    // 遍历所有活跃效果
    for (int i = 0; i < MAX_ACTIVE_EFFECTS; i++) {
        if (!active_effects[i].active) {
            continue;
        }
        
        struct typing_follow_state *state = &active_effects[i];
        
        if (state->step < FADE_STEPS) {
            all_complete = false;
            
            // 计算当前步的亮度：线性递减
            // 亮度 = 最大亮度 * (剩余步数 / 总步数)
            uint8_t remaining_steps = FADE_STEPS - state->step;
            uint8_t brightness = (CONFIG_ZMK_TYPING_FOLLOW_MAX_BRIGHTNESS * 
                                  remaining_steps) / FADE_STEPS;
            
            // 获取当前LED的HSB值并修改亮度
            struct zmk_led_hsb current_color;
            // 注意：ZMK API可能需要获取当前LED颜色
            // 这里简化处理：使用全局HSB设置
            extern struct zmk_led_hsb zmk_rgb_underglow_current_color;
            
            struct zmk_led_hsb new_color = {
                .hue = current_color.hue,
                .saturation = current_color.saturation,
                .brightness = brightness
            };
            
            // 设置指定LED的颜色
            // 注意：需要ZMK提供单LED控制API
            // 如果ZMK不支持单LED控制，需要扩展
            rgb_led_set_color(state->led_index, new_color);
            
            state->step++;
            state->current_bright = brightness;
        }
        
        if (state->step >= FADE_STEPS) {
            // 动画完成，关闭此LED并释放槽位
            rgb_led_set_color(state->led_index, 
                              (struct zmk_led_hsb){0, 0, 0});
            state->active = false;
            state->step = 0;
        }
    }
    
    // 如果还有活跃效果，继续调度下一次淡出
    if (!all_complete) {
        k_work_reschedule(&fade_work, K_MSEC(FADE_STEP_DELAY_MS));
    }
}

/**
 * 按键事件监听回调函数
 * 当键盘有按键按下时触发
 */
static int keycode_state_changed_listener(const struct zmk_event_header *eh) {
    struct zmk_keycode_state_changed *ev = 
        cast_zmk_keycode_state_changed(eh);
    
    // 只在按键按下时触发（state为true表示按下）
    if (ev->state) {
        // 找到空闲效果槽位
        struct typing_follow_state *state = find_free_slot();
        if (state) {
            // 初始化效果状态
            state->led_index = map_position_to_led(ev->position);
            state->step = 0;
            state->active = true;
            state->current_bright = CONFIG_ZMK_TYPING_FOLLOW_MAX_BRIGHTNESS;
            
            // 立即设置LED为最大亮度
            struct zmk_led_hsb max_color = {
                .hue = 0,      // 红色，可改为其他颜色
                .saturation = 100,
                .brightness = CONFIG_ZMK_TYPING_FOLLOW_MAX_BRIGHTNESS
            };
            rgb_led_set_color(state->led_index, max_color);
            
            // 启动/继续淡出动画
            k_work_reschedule(&fade_work, K_MSEC(FADE_STEP_DELAY_MS));
            
            LOG_DBG("Triggered typing follow for position %d, LED %d",
                    ev->position, state->led_index);
        }
    }
    
    return 0;
}

/**
 * 注册事件监听器
 */
ZMK_LISTENER(typing_follow_listener, keycode_state_changed_listener);
ZMK_SUBSCRIPTION(typing_follow_listener, zmk_keycode_state_changed);

/**
 * 初始化函数
 */
int typing_follow_effect_init(void) {
    LOG_INF("Initializing Typing Follow Effect");
    
    // 初始化活跃效果数组
    memset(active_effects, 0, sizeof(active_effects));
    
    // 初始化延迟工作队列
    k_work_init_delayable(&fade_work, fade_work_handler);
    
    return 0;
}