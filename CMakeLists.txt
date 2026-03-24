cmake_minimum_required(VERSION 3.10)
project(CompilerOSInfo VERSION 1.0.0 LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 根据编译器设置不同的标志
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # GCC和Clang的编译选项
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
    
    # 添加特定的警告抑制
    if(CMAKE_C_COMPILER_ID MATCHES "GNU")
        # 针对GCC的特定选项
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers")
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        # 针对Clang的特定选项
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers")
    endif()
elseif(MSVC)
    # MSVC的编译选项
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /wd4204 /wd4100 /utf-8")
endif()

# 添加可执行文件
add_executable(${PROJECT_NAME} src/main.c)

# 安装目标
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)

# 测试
enable_testing()
add_test(NAME ${PROJECT_NAME}_test
    COMMAND ${PROJECT_NAME}
)