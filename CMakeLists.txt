# 规定项目所需的最低 cmake 版本为 3.15
cmake_minimum_required(VERSION 3.15)

# 可以在这里修改项目名称和版本号
project(uartools VERSION 1.0)

# 获得编译的 UTC 时间，存储在 BUILD_TIME 变量
string(TIMESTAMP BUILD_TIME "%Y.%m.%d %H:%M" UTC)
message(STATUS "Build Time: " ${BUILD_TIME})

# 如果存在 git ，就执行获取 git 提交版本
find_package(Git)
if(Git_FOUND)
    message(STATUS "Git found: ${GIT_EXECUTABLE}")
    # 获取最新的 git commit hash，存储在 GIT_HASH 变量
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # 获取最新的 commit 日期，YYYY-MM-DD，存储在 GIT_DATA 变量
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=default
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE  GIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Git Commit ${GIT_HASH}, Date ${GIT_DATE}")
endif()


# 设置源文件目录和头文件目录
set(SOURCES_DIR src)
set(HEADERS_DIR inc)

# 收集源文件
file(GLOB_RECURSE SOURCES "${SOURCES_DIR}/*.c")

# 设置程序名
set(program uart_test)

# 创建可执行文件
add_executable(${program} ${SOURCES})

# 设置配置文件
configure_file(Config.h.in Config.h)

# 添加头文件搜索路径
target_include_directories(${program} PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${HEADERS_DIR}"
)

# 所有编译模式都使用 -Wall 选项
target_compile_options(${program} PRIVATE -Wall)

# 如果是 Debug 版本，就添加 __DEBUG__ 宏定义
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Add Debug")
    target_compile_definitions(${program} PRIVATE "__DEBUG__")
endif()

# 安装可执行文件到 bin 目录
install(TARGETS ${program}
    RUNTIME DESTINATION bin
)