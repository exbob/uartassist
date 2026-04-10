cmake_minimum_required(VERSION 3.15)
project(uartassist C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

string(TIMESTAMP BUILD_TIME "%Y.%m.%d %H:%M" UTC)
message(STATUS "Build Time: ${BUILD_TIME}")

execute_process(
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/git-version.sh"
	OUTPUT_VARIABLE GIT_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_QUIET
)
if(NOT GIT_VERSION)
	set(GIT_VERSION "unknown")
endif()
message(STATUS "Git Version: ${GIT_VERSION}")

configure_file(Config.h.in Config.h)

set(PROGRAM_NAME uart_assist)
set(PROGRAM_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/args_parser.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/uart_assist.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/json_config.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson/cJSON.c"
)

add_executable(${PROGRAM_NAME} ${PROGRAM_SOURCES})

target_include_directories(${PROGRAM_NAME}
	PRIVATE
	"${PROJECT_BINARY_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/inc"
	"${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson"
)

target_compile_options(${PROGRAM_NAME}
	PRIVATE
	-Wall
	-Wextra
	$<$<CONFIG:Debug>:-g;-O0>
	$<$<CONFIG:Release>:-O2>
)

target_compile_definitions(${PROGRAM_NAME}
	PRIVATE
	_GNU_SOURCE
	_DEFAULT_SOURCE
	$<$<CONFIG:Debug>:APP_DEBUG>
)

install(TARGETS ${PROGRAM_NAME}
	RUNTIME DESTINATION "."
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/configs")
	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/configs/"
		DESTINATION "."
	)
endif()
