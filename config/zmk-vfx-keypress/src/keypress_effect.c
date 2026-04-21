// 文件: zmk-vfx-keypress/src/keypress_effect.c

// 在包含任何头文件之前定义 DT_DRV_COMPAT


#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/device.h>  // 设备驱动API

#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/rgb_underglow.h>

#define DT_DRV_COMPAT zmk_vfx_keypress  // 必须与 .dtsi 中的 compatible 完全一致

LOG_MODULE_REGISTER(keypress_effect, LOG_LEVEL_INF);

// 设备驱动初始化函数
static int keypress_effect_init(const struct device *dev) {
    ARG_UNUSED(dev);
    LOG_INF("Keypress effect module initialized");
    return 0;
}

// 定义设备实例
DEVICE_DT_INST_DEFINE(0,                    // 实例编号（如果只有一个设备通常为0）
                      keypress_effect_init, // 初始化函数
                      NULL,                 // 电源管理函数（通常为NULL）
                      NULL,                 // 数据指针
                      NULL,                 // 配置指针
                      POST_KERNEL,          // 初始化级别
                      CONFIG_APPLICATION_INIT_PRIORITY, // 优先级
                      NULL);                // API函数表

// 灯效持续时间（毫秒）
#define EFFECT_DURATION_MS 150

// 高亮颜色（白色）
#define HIGHLIGHT_R 0xFF
#define HIGHLIGHT_G 0xFF
#define HIGHLIGHT_B 0xFF

// 映射表：将按键位置转换为LED索引
static const int position_to_led_map[] = {
    // ⚠️ 必须根据键盘实际布局填充
    // ... 

    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,   // 第一行
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,   // 第二行
    30, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,       // 第三行
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,       // 第四行
    59, 60, 61, 62, 63, 64, 65, 66, 67,                       // 第五行（底部）

};

static void set_led_color(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b) {
    extern int zmk_rgb_underglow_per_key_set(uint32_t led_index, uint8_t r, uint8_t g, uint8_t b);
    zmk_rgb_underglow_per_key_set(led_index, r, g, b);
}

static int keypress_effect_callback(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *pos_ev = 
        as_zmk_position_state_changed(eh);
    if (!pos_ev || !pos_ev->state) {
        return 0;
    }
    
    int position = pos_ev->position;
    if (position >= ARRAY_SIZE(position_to_led_map)) {
        return 0;
    }
    
    int led_index = position_to_led_map[position];
    set_led_color(led_index, HIGHLIGHT_R, HIGHLIGHT_G, HIGHLIGHT_B);
    
    // 安排定时器恢复（简化处理）
    static struct k_work_delayable restore_work;
    k_work_reschedule(&restore_work, K_MSEC(EFFECT_DURATION_MS));
    
    return 0;
}

ZMK_LISTENER(keypress_effect_listener, keypress_effect_callback);
ZMK_SUBSCRIPTION(keypress_effect_listener, zmk_position_state_changed);

// 定义一个结构体来管理每个LED的恢复定时器
struct led_restore_timer {
    struct k_work_delayable work;
    uint8_t led_index;
};

static struct led_restore_timer restore_timers[TOTAL_LEDS];

// 定时器回调函数，用于恢复LED的原始颜色
static void restore_led_color_fn(struct k_work *work) {
    struct led_restore_timer *timer = CONTAINER_OF(work, struct led_restore_timer, work);
    // 这里需要根据你的需求恢复LED的颜色，例如熄灭它
    zmk_rgb_underglow_per_key_set(timer->led_index, 0x00, 0x00, 0x00);
}

static int keypress_effect_callback(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *pos_ev = as_zmk_position_state_changed(eh);
    if (!pos_ev || !pos_ev->state) return 0;

    int position = pos_ev->position;
    if (position >= ARRAY_SIZE(position_to_led_map)) return 0;
    int led_index = position_to_led_map[position];

    // 点亮LED（例如设置成白色）
    zmk_rgb_underglow_per_key_set(led_index, 0xFF, 0xFF, 0xFF);

    // 重置定时器
    k_work_reschedule(&restore_timers[led_index].work, K_MSEC(EFFECT_DURATION_MS));

    return 0;
}

// 在模块初始化函数中初始化所有定时器
static int keypress_effect_init(const struct device *dev) {
    for (int i = 0; i < TOTAL_LEDS; i++) {
        k_work_init_delayable(&restore_timers[i].work, restore_led_color_fn);
        restore_timers[i].led_index = i;
    }
    return 0;
}

