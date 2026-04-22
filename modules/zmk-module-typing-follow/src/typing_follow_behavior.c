// modules/zmk-module-typing-follow/src/typing_follow_behavior.c
// 自定义行为实现，允许通过键位映射控制效果

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include "typing_follow_effect.h"

LOG_MODULE_REGISTER(typing_follow_behavior, LOG_LEVEL_DBG);

// 行为结构体定义
struct behavior_typing_follow_config {
    uint32_t fade_duration_ms;
    uint8_t max_brightness;
};

// 行为数据（运行时状态）
struct behavior_typing_follow_data {
    bool enabled;
};

/**
 * 行为按键处理函数
 * 当用户按下绑定的按键时调用
 */
static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("Typing follow behavior triggered");
    
    // 可以在这里添加切换效果开关的逻辑
    // 例如：临时提高亮度或触发特殊效果
    
    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    return 0;
}

// 行为API定义
static const struct behavior_driver_api behavior_typing_follow_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

// 行为初始化
static int behavior_typing_follow_init(const struct device *dev) {
    struct behavior_typing_follow_data *data = dev->data;
    data->enabled = true;
    
    // 初始化效果
    typing_follow_effect_init();
    
    return 0;
}

#define TYPING_FOLLOW_INST(n)                                          \
    static struct behavior_typing_follow_data behavior_typing_follow_data_##n = { \
        .enabled = true,                                               \
    };                                                                 \
    static const struct behavior_typing_follow_config behavior_typing_follow_config_##n = { \
        .fade_duration_ms = CONFIG_ZMK_TYPING_FOLLOW_FADE_DURATION_MS, \
        .max_brightness = CONFIG_ZMK_TYPING_FOLLOW_MAX_BRIGHTNESS,     \
    };                                                                 \
    DEVICE_DT_INST_DEFINE(n,                                           \
                          behavior_typing_follow_init,                 \
                          NULL,                                        \
                          &behavior_typing_follow_data_##n,            \
                          &behavior_typing_follow_config_##n,          \
                          APPLICATION,                                 \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,         \
                          &behavior_typing_follow_api);

// 创建行为设备实例
DT_INST_FOREACH_STATUS_OKAY(TYPING_FOLLOW_INST)