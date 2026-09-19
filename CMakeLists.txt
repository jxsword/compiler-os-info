cmake_minimum_required(VERSION 3.16)

project(CompilerOSInfo VERSION 1.0.0 LANGUAGES C)

# ---------- C 标准 ----------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---------- 默认构建类型 ----------
# 单配置生成器（Makefiles / Ninja）下 CMAKE_BUILD_TYPE 常为空，给个默认值
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ---------- 统一产物目录 ----------
# 所有平台、所有生成器（含 Visual Studio 多配置）的 exe 都落在 <build>/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} "${CMAKE_BINARY_DIR}/bin")
endforeach()

# ---------- 可执行文件 ----------
add_executable(${PROJECT_NAME} src/main.c)

# 编译选项：按编译器分流，作用域限定在本 target
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -pedantic -Wno-missing-field-initializers>
    $<$<C_COMPILER_ID:MSVC>:/W3 /wd4204 /wd4100 /utf-8>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

# ---------- Windows 静态链接 ----------
# MSVC：静态链接 CRT，exe 不再依赖 vc_redist
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# MinGW：静态链接 libgcc / winpthread，exe 可独立运行
if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc)
endif()

# ---------- 安装 ----------
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE  DESTINATION bin
)

# ---------- 测试 ----------
enable_testing()
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME})
set_tests_properties(${PROJECT_NAME}_test PROPERTIES
    PASS_REGULAR_EXPRESSION "."
    TIMEOUT 30
)