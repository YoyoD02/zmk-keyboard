# modules/zmk-module-typing-follow/CMakeLists.txt
# CMake构建配置，将源文件加入编译

# 添加编译时宏定义
zephyr_compile_definitions(
    ZMK_BEHAVIOR_TYPING_FOLLOW
)

# 添加头文件搜索路径
zephyr_include_directories(
    src/include
)

# 添加源文件到ZMK核心库
target_sources(app PRIVATE
    src/typing_follow_effect.c
    src/typing_follow_behavior.c
)