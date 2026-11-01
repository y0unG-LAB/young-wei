cmake_minimum_required(VERSION 3.20)
project(linear_list_lab C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    add_compile_options(/W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -g)
endif()

set(LAB_CORE
    src/task.c
    src/seqlist.c
    src/linkedlist.c
)

# 同一份测试源码编两次，靠 -D 切到不同的实现：
#   test-seq    -> -DUSE_SEQLIST
#   test-linked -> -DUSE_LINKEDLIST
add_executable(test-seq ${LAB_CORE} tests/test_list.c)
target_compile_definitions(test-seq PRIVATE USE_SEQLIST)
target_include_directories(test-seq PRIVATE include)

add_executable(test-linked ${LAB_CORE} tests/test_list.c)
target_compile_definitions(test-linked PRIVATE USE_LINKEDLIST)
target_include_directories(test-linked PRIVATE include)

add_executable(test-cross ${LAB_CORE} tests/test_cross.c)
target_include_directories(test-cross PRIVATE include)

add_executable(bench ${LAB_CORE} bench/bench.c)
target_include_directories(bench PRIVATE include bench)

# 你的作业本：只链接 my-impl/linkedlist.c，不动 src/linkedlist.c
add_executable(test-my src/task.c my-impl/linkedlist.c tests/test_list.c)
target_compile_definitions(test-my PRIVATE USE_LINKEDLIST)
target_include_directories(test-my PRIVATE include)

# 学习阶梯：每一步一个独立小程序，不依赖项目里的头文件
add_executable(step1 ladder/step1.c)
add_executable(step1_wz ladder/step1_wangzhuo.c)
add_executable(step1_full ladder/step1_full.c)
add_executable(step2 ladder/step2.c)

# 让 CLion 的 Run 配置和 ctest 都能直接跑
enable_testing()
add_test(NAME 1-seqlist COMMAND test-seq)
add_test(NAME 2-linkedlist COMMAND test-linked)
add_test(NAME 3-cross COMMAND test-cross)
add_test(NAME 4-myimpl COMMAND test-my)
