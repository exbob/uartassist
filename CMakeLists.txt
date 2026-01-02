# 规定项目所需的最低 cmake 版本为 3.15
cmake_minimum_required(VERSION 3.15)

# 可以在这里修改项目名称和版本号
project(uartools VERSION 1.0)

# 获得编译的 UTC 时间，存储在 BUILD_TIME 变量
string(TIMESTAMP BUILD_TIME "%Y.%m.%d %H:%M" UTC)
message(STATUS "Build Time: " ${BUILD_TIME})

# 初始化完整版本号为基础版本号
set(FULL_VERSION "${PROJECT_VERSION}")

# 如果存在 git ，就执行获取 git 提交版本
find_package(Git)
if(Git_FOUND)
    message(STATUS "Git found: ${GIT_EXECUTABLE}")
    # 获取最新的 git commit hash（短格式，7位），存储在 GIT_HASH 变量
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # 获取最新的 commit 日期，YYYY-MM-DD，存储在 GIT_DATA 变量
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=default
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE  GIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # 如果成功获取到 git hash，则更新版本号为 VERSION-g[commit_id] 格式
    if(NOT "${GIT_HASH}" STREQUAL "")
        set(FULL_VERSION "${PROJECT_VERSION}-g${GIT_HASH}")
        message(STATUS "Git Commit ${GIT_HASH}, Date ${GIT_DATE}")
        message(STATUS "Version: ${FULL_VERSION}")
    else()
        message(STATUS "Git Commit not found, using base version")
    endif()
else()
    message(STATUS "Git not found, using base version")
endif()


# 设置源文件目录和头文件目录
set(SOURCES_DIR src)
set(HEADERS_DIR inc)

# uart_assist 源文件
set(UART_ASSIST_SOURCES
    ${SOURCES_DIR}/main.c
    ${SOURCES_DIR}/args_parser.c
    ${SOURCES_DIR}/uart_assist.c
    ${SOURCES_DIR}/json_config.c
    third_party/cjson/cJSON.c
)

# 设置程序名
set(program uart_assist)

# 创建可执行文件
add_executable(${program} ${UART_ASSIST_SOURCES})

# 设置配置文件
configure_file(Config.h.in Config.h)

# 添加头文件搜索路径
target_include_directories(${program} PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${HEADERS_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson"
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
