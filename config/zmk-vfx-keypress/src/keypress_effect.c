// 文件: config/zmk-vfx-keypress/src/keypress_effect.c

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/activity.h>
#include <zmk/events/ble_active_profile_changed.h>

#include <dt-bindings/zmk/rgb.h>
#include <zmk/rgb_underglow.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// 定义灯效持续的时间（毫秒），之后LED会恢复到原来的状态
#define EFFECT_DURATION_MS 150

// 这里需要你根据自己的键盘硬件，定义一个从按键位置（position）到LED编号（led_index）的映射表。
// 以下是一个假设的60%键盘示例，你需要根据你的实际键盘来填充这个数组。
static const int position_to_led_map[] = {
    // 这是一个简化的例子，实际键盘的映射可能非常复杂
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,   // 第一行
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,   // 第二行
    30, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,       // 第三行
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,       // 第四行
    59, 60, 61, 62, 63, 64, 65, 66, 67, 68,                       // 第五行（底部）
    
    // ... 你需要在这里完成所有按键的映射
};

static void set_led_color(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
    // 这是一个用于设置单个LED颜色的辅助函数。
    // 注意：你需要调用ZMK的底层API，但官方的每键RGB API可能尚未完全稳定。
    // 这需要根据你的ZMK版本来实现，可能涉及到直接操作LED strip或使用某个中间层。
    // 目前ZMK中并没有一个简单的 'set_per_key_led' 函数，
    // 实现它可能需要深入理解RGB灯效系统的工作方式。
    // 一种方法是修改或自定义一个RGB效果，在你的效果函数内部为特定位置的LED分配颜色。
}

static int keypress_effect_callback(const zmk_event_t *eh) {
    // 1. 检查事件类型：是否为按键状态改变事件？
    if (as_zmk_position_state_changed(eh) == NULL) {
        return 0; // 不是，继续传递给其他监听器
    }

    // 2. 获取事件详情：哪个键被按下或释放了？
    struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
    if (ev == NULL || !ev->state) {
        return 0; // 只关心按键按下事件（state为true）
    }

    // 3. 获取按键位置（position），并转换为LED编号
    int position = ev->position;
    // 需要确保 position 在映射表的有效范围内
    if (position >= ARRAY_SIZE(position_to_led_map)) {
        return 0;
    }
    int led_index = position_to_led_map[position];

    // 4. 点亮对应的LED（例如，设置为白色）
    // TODO: 这里需要调用实际的API来设置LED颜色
    // set_led_color(led_index, 0xff, 0xff, 0xff);

    // 5. 启动一个定时器，在延迟后熄灭该LED
    // TODO: 实现定时器逻辑，以恢复LED的原始颜色
    // 这通常涉及在延迟后再次调用 set_led_color 或标记该LED为熄灭状态。

    LOG_INF("Keypress detected at position %d, LED index %d", position, led_index);
    return 0;
}

// 监听按键事件
ZMK_LISTENER(keypress_effect_listener, keypress_effect_callback);
ZMK_SUBSCRIPTION(keypress_effect_listener, zmk_position_state_changed);