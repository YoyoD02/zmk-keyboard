// 文件: zmk-vfx-keypress/src/keypress_effect.c

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/rgb_underglow.h>

// --- 核心: 定义设备树匹配字符串 ---
#define DT_DRV_COMPAT zmk_vfx_keypress
// ---------------------------------

LOG_MODULE_REGISTER(keypress_effect, LOG_LEVEL_INF);

// ==================== 配置区域 ====================
// TODO: 替换为你的键盘实际的LED总数
#define TOTAL_LEDS 68

// 灯效持续时间（毫秒）
#define EFFECT_DURATION_MS 150

// 高亮颜色 (RGB, 0-255)
#define HIGHLIGHT_R 0xFF
#define HIGHLIGHT_G 0xFF
#define HIGHLIGHT_B 0xFF
// ================================================

// 映射表：按键位置 -> LED索引
// TODO: 根据你的键盘实际布局填充此数组！
static const int position_to_led_map[] = {
    // 这是一个60%键盘的示例，你需要根据自己的键盘修改！
    // 行0 (ESC - Backspace)
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67

};

// 延迟工作项结构体
struct led_restore_timer {
    struct k_work_delayable work;
    uint8_t led_index;
};

// 定时器数组
static struct led_restore_timer restore_timers[TOTAL_LEDS];

// 设置单个LED颜色
static void set_led_color(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b) {
    // 调用ZMK的每键RGB API
    extern int zmk_rgb_underglow_per_key_set(uint32_t led_index, uint8_t r, uint8_t g, uint8_t b);
    zmk_rgb_underglow_per_key_set(led_index, r, g, b);
}

// 定时器回调：恢复LED颜色（熄灭）
static void restore_led_color_fn(struct k_work *work) {
    struct led_restore_timer *timer = CONTAINER_OF(work, struct led_restore_timer, work);
    set_led_color(timer->led_index, 0x00, 0x00, 0x00);
}

// 按键事件回调
static int keypress_effect_callback(const zmk_event_t *eh) {
    // 1. 检查事件类型
    const struct zmk_position_state_changed *pos_ev = as_zmk_position_state_changed(eh);
    if (!pos_ev) {
        return 0;
    }
    
    // 2. 只处理按键按下事件
    if (!pos_ev->state) {
        return 0;
    }
    
    // 3. 获取按键位置并转换为LED索引
    int position = pos_ev->position;
    if (position >= ARRAY_SIZE(position_to_led_map)) {
        LOG_WRN("Position %d out of range", position);
        return 0;
    }
    int led_index = position_to_led_map[position];
    
    // 4. 确保LED索引有效
    if (led_index >= TOTAL_LEDS) {
        LOG_WRN("LED index %d out of range", led_index);
        return 0;
    }
    
    // 5. 点亮LED
    set_led_color(led_index, HIGHLIGHT_R, HIGHLIGHT_G, HIGHLIGHT_B);
    
    // 6. 重置定时器，安排熄灭
    k_work_reschedule(&restore_timers[led_index].work, K_MSEC(EFFECT_DURATION_MS));
    
    LOG_DBG("Keypress at position %d -> LED %d", position, led_index);
    return 0;
}

// 设备驱动初始化函数
static int keypress_effect_init(const struct device *dev) {
    ARG_UNUSED(dev);
    
    // 初始化所有LED的恢复定时器
    for (int i = 0; i < TOTAL_LEDS; i++) {
        k_work_init_delayable(&restore_timers[i].work, restore_led_color_fn);
        restore_timers[i].led_index = i;
    }
    
    LOG_INF("Keypress effect module initialized");
    return 0;
}

// 注册设备驱动
DEVICE_DT_INST_DEFINE(0,                    // 实例编号
                      keypress_effect_init, // 初始化函数
                      NULL,                 // 电源管理
                      NULL,                 // 运行时数据
                      NULL,                 // 配置数据
                      POST_KERNEL,          // 初始化级别
                      CONFIG_APPLICATION_INIT_PRIORITY, // 优先级
                      NULL);                // API

// 注册事件监听器
ZMK_LISTENER(keypress_effect_listener, keypress_effect_callback);
ZMK_SUBSCRIPTION(keypress_effect_listener, zmk_position_state_changed);