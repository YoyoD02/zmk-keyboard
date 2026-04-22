// modules/zmk-module-typing-follow/src/typing_follow_behavior.c
// 自定义行为实现，允许通过键位映射控制效果

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include "typing_follow_effect.h"

LOG_MODULE_REGISTER(typing_follow_behavior, LOG_LEVEL_DBG);


#include "typing_follow_effect.h"

LOG_MODULE_REGISTER(typing_follow_behavior, LOG_LEVEL_DBG);

// ======================== 效果开关状态管理 ========================

// 静态变量：记录打字跟随效果是否启用（默认开启）
static bool typing_follow_enabled = true;

// 供效果模块查询当前开关状态
bool typing_follow_effect_is_enabled(void) {
    return typing_follow_enabled;
}

// 切换效果开关状态
void typing_follow_effect_toggle(void) {
    typing_follow_enabled = !typing_follow_enabled;
    LOG_INF("Typing follow effect %s", 
            typing_follow_enabled ? "ENABLED" : "DISABLED");

    // 可选：提供视觉反馈（LED 闪烁一次）
    // 注意：如果 ZMK 的 RGB API 可用，可以取消下面的注释块
    /*
    #ifdef CONFIG_ZMK_RGB_UNDERGLOW
    extern struct zmk_led_hsb zmk_rgb_underglow_current_color;
    struct zmk_led_hsb original = zmk_rgb_underglow_current_color;
    
    if (typing_follow_enabled) {
        // 开启效果：绿色快速闪烁
        struct zmk_led_hsb flash = { .hue = 80, .saturation = 100, .brightness = 100 };
        zmk_rgb_underglow_set_hsb(flash);
        k_sleep(K_MSEC(100));
    } else {
        // 关闭效果：红色快速闪烁
        struct zmk_led_hsb flash = { .hue = 0, .saturation = 100, .brightness = 100 };
        zmk_rgb_underglow_set_hsb(flash);
        k_sleep(K_MSEC(100));
    }
    zmk_rgb_underglow_set_hsb(original);
    #endif
    */
}




// 行为结构体定义
struct behavior_typing_follow_config {
    uint32_t fade_duration_ms;
    uint8_t max_brightness;
};

// 行为数据（运行时状态）
struct behavior_typing_follow_data {
    bool initialized;
};

/**
 * 行为按键处理函数
 * 当用户按下绑定的按键时调用
 */
static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("Typing follow behavior triggered");
    typing_follow_effect_toggle();
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