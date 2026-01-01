# uart_test

串口测试工具，支持自测、发送和接收三种工作模式。

## 简介

uart_test 是一个用于 Linux 系统的串口测试工具，使用 C 语言开发，支持通过命令行参数配置串口参数和工作模式。主要功能包括：

- **自测模式 (loopback)**: 自发自收，用于测试串口硬件是否正常工作
- **发送模式 (send)**: 按指定间隔和次数发送数据，支持 ASCII 和 HEX 格式
- **接收模式 (recv)**: 持续接收串口数据并显示，支持 ASCII 和 HEX 格式显示

## 编译方法

### 基本编译

```bash
./build.sh
```

编译后的可执行文件位于 `./bin/uart_test`

### 交叉编译

支持通过 `ARCH` 环境变量指定目标架构：

```bash
# x86 架构（默认）
ARCH=x86 ./build.sh

# ARM64 架构
ARCH=ARM64 ./build.sh
```

### Debug 模式编译

```bash
./build.sh debug
```

### 清理编译文件

```bash
./build.sh clean
```

## 使用方法

### 基本语法

```bash
uart_test [OPTIONS]
```

### 通用选项（所有模式）

- `-m, --mode <mode>`: 工作模式，必需参数
  - `loopback`: 自测模式
  - `send`: 发送模式
  - `recv`: 接收模式
- `-d, --device <device>`: 串口设备（默认: `/dev/ttyAMA0`）
- `-b, --baud <baudrate>`: 波特率（默认: `115200`）
- `-c, --config <config>`: 串口参数，格式：数据位校验位停止位（默认: `8N1`）
  - 例如：`8N1`, `7E1`, `8O2`
- `-h, --help`: 显示帮助信息

### Loopback 模式选项

- `-s, --send <string>`: 发送字符串（默认: `123456`）
- `-f, --format <format>`: 发送格式 `ascii/hex`（默认: `ascii`）
  - 如果选择 `hex`，字符串会被解析为16进制（例如：`af37126b4A` = 5字节）

### Send 模式选项

- `-s, --send <string>`: 发送字符串（默认: `123456`）
- `-i, --interval <ms>`: 发送间隔，1-10000 毫秒（默认: `1000`）
- `-n, --count <count>`: 发送次数，0 表示无限（默认: `0`）
- `-f, --format <format>`: 发送格式 `ascii/hex`（默认: `ascii`）
  - 如果选择 `hex`，字符串会被解析为16进制（例如：`af37126b4A` = 5字节）

### Receive 模式选项

- `-f, --format <format>`: 输出格式 `ascii/hex`（默认: `ascii`）

## 使用示例

### 自测模式

```bash
# ASCII 格式自测
./bin/uart_test -m loopback -d /dev/ttyUSB0 -s "Hello"

# HEX 格式自测
./bin/uart_test -m loopback -d /dev/ttyUSB0 -s "af37126b4A" -f hex
```

### 发送模式

```bash
# 发送 ASCII 字符串，间隔 500ms，发送 10 次
./bin/uart_test -m send -d /dev/ttyUSB0 -s "Hello" -i 500 -n 10

# 发送 HEX 数据，间隔 1000ms，无限发送
./bin/uart_test -m send -d /dev/ttyUSB0 -s "af37126b4A" -f hex -i 1000

# 自定义波特率和串口参数
./bin/uart_test -m send -d /dev/ttyUSB0 -b 9600 -c 7E1 -s "Test"
```

### 接收模式

```bash
# ASCII 格式接收
./bin/uart_test -m recv -d /dev/ttyUSB0

# HEX 格式接收
./bin/uart_test -m recv -d /dev/ttyUSB0 -f hex
```

## 输出格式

### 发送模式输出

```
Send [1] : "123456" (6 bytes, total: 6 bytes)
Send [2] : "123456" (6 bytes, total: 12 bytes)
```

### 接收模式输出

```
Recv [1] : "123456" (6 bytes, total: 6 bytes)
Recv [2] : "ABCDEF" (6 bytes, total: 12 bytes)
```

## 注意事项

1. 使用串口设备需要相应的权限，可能需要使用 `sudo` 或添加用户到 `dialout` 组
2. 接收模式使用 2 秒超时机制，超时会显示提示信息并继续等待
3. 按 `Ctrl+C` 可以优雅退出程序
4. HEX 格式的字符串必须是偶数长度（每两个字符代表一个字节）
