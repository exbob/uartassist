# uart_assist

串口测试工具，支持自测、发送和接收三种工作模式。

## 简介

uart_assist 是一个用于 Linux 系统的串口测试工具，使用 C 语言开发，支持通过命令行参数配置串口参数和工作模式。主要功能包括：

- **自测模式 (loopback)**: 自发自收，用于测试串口硬件是否正常工作
- **发送模式 (send)**: 按指定间隔和次数发送数据，支持 ASCII 和 HEX 格式
- **接收模式 (recv)**: 持续接收串口数据并显示，支持 ASCII 和 HEX 格式显示
- **文件模式 (file)**: 通过 JSON 配置文件批量发送数据，支持循环发送和延时控制

## 编译方法

### 基本编译

```bash
./build.sh
```

编译后的可执行文件位于 `./bin/uart_assist`

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
uart_assist [OPTIONS]
```

### 通用选项（所有模式）

- `-m, --mode <mode>`: 工作模式，必需参数
  - `loopback`: 自测模式
  - `send`: 发送模式
  - `recv`: 接收模式
  - `file`: 文件模式
- `-d, --device <device>`: 串口设备（默认: `/dev/ttyAMA0`）
- `-b, --baud <baudrate>`: 波特率（默认: `115200`）
- `-c, --config <config>`: 串口参数，格式：数据位校验位停止位（默认: `8N1`）
  - 例如：`8N1`, `7E1`, `8O2`
- `-h, --help`: 显示帮助信息

### Loopback 模式选项

用于单个UART端口自发自收测试，UART的Tx和Rx短接。支持的选项：

- `-s, --send <string>`: 发送字符串（默认: `123456`）
- `-f, --format <format>`: 发送格式 `ascii/hex`（默认: `ascii`）
  - 如果选择 `hex`，字符串会被解析为16进制（例如：`af37126b4A` = 5字节）

使用示例：

```bash
# ASCII 格式自测
./bin/uart_assist -m loopback -d /dev/ttyUSB0 -s "Hello"

# HEX 格式自测
./bin/uart_assist -m loopback -d /dev/ttyUSB0 -s "af37126b4A" -f hex
```

### Send 模式选项

根据选项参数定时发送特定数据。支持的选项：

- `-s, --send <string>`: 发送字符串（默认: `123456`）
- `-i, --interval <ms>`: 发送间隔，1-10000 毫秒（默认: `1000`）
- `-n, --count <count>`: 发送次数，0 表示无限（默认: `0`）
- `-f, --format <format>`: 发送格式 `ascii/hex`（默认: `ascii`）
  - 如果选择 `hex`，字符串会被解析为16进制（例如：`af37126b4A` = 5字节）

使用示例：

```bash
# 发送 ASCII 字符串，间隔 500ms，发送 10 次
./bin/uart_assist -m send -d /dev/ttyUSB0 -s "Hello" -i 500 -n 10

# 发送 HEX 数据，间隔 1000ms，无限发送
./bin/uart_assist -m send -d /dev/ttyUSB0 -s "af37126b4A" -f hex -i 1000

# 自定义波特率和串口参数
./bin/uart_assist -m send -d /dev/ttyUSB0 -b 9600 -c 7E1 -s "Test"
```

### Receive 模式选项

循环接收并打印数据。支持的选项：

- `-f, --format <format>`: 输出格式 `ascii/hex`（默认: `ascii`）

使用示例：

```bash
# ASCII 格式接收
./bin/uart_assist -m recv -d /dev/ttyUSB0

# HEX 格式接收
./bin/uart_assist -m recv -d /dev/ttyUSB0 -f hex
```

### File 模式选项

按照 JSON 文件中设定的格式和内容，支持定时、批量发送。支持的选项：

- `-F, --file <file>`: JSON 配置文件路径，必需参数

**JSON 文件格式**：

``` json
{
  "GroupName": "TestConfig",
  "CycleCount": 5,
  "SendList": [
    {
      "Number": 1,
      "HexData": "756172745f6173736973740a",
      "Delay": 50,
      "Enable": 1
    },
    {
      "Number": 2,
      "HexData": "313233343536373839300a",
      "Delay": 1000,
      "Enable": 1
    }
  ]
}
```

**字段说明**：
- `GroupName`: 配置组名称
- `CycleCount`: SendList 发送的循环次数（>= 1）
- `SendList`: 发送列表数组，程序依次发送数组元素中的 HexData
  - `Number`: 该元素的标签号（用于显示）
  - `HexData`: 要发送的16进制数据字符串（必须是偶数长度）
  - `Delay`: 发送该数据后的延时时间，单位毫秒，取值范围 1-1000
  - `Enable`: 是否启用该数据项，1=启用，0=忽略

使用示例：

```bash
# 使用 JSON 配置文件发送数据
./bin/uart_assist -m file -F config.json

# 自定义串口参数
./bin/uart_assist -m file -d /dev/ttyUSB0 -b 9600 -c 7E1 -F config.json
```

## 注意事项

1. 使用串口设备需要相应的权限，可能需要使用 `sudo` 或添加用户到 `dialout` 组
2. 接收模式使用 2 秒超时机制，超时会显示提示信息并继续等待
3. 按 `Ctrl+C` 可以优雅退出程序
4. HEX 格式的字符串必须是偶数长度（每两个字符代表一个字节）
