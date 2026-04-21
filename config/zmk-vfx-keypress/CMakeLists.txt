
include(${ZEPHYR_BASE}/cmake/modules/extension.cmake)
zephyr_library()
zephyr_library_sources(src/keypress_effect.c)
zephyr_library_include_directories(include)
# 添加设备树目录
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dts)