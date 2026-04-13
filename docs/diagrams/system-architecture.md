# uart_assist 项目架构图

基于当前代码（`src/main.c`、`src/args_parser.c`、`src/uart_assist.c`、`src/json_config.c`、`inc/uartdev.h`）整理。

```mermaid
flowchart TD
    U[用户 / CLI 参数] --> M["main.c<br/>程序入口 + SIGINT 处理"]
    M --> P["args_parser.c<br/>parse_args()"]
    P --> C["uart_config_t<br/>运行配置"]

    C --> D["uartdev.h<br/>uartdev_new() / uartdev_setup()"]
    D --> K["Linux 串口设备<br/>/dev/tty*"]

    M --> S{mode 分发}
    C --> S

    S --> L["loopback<br/>uart_loopback_test()"]
    S --> T["send<br/>uart_send_test()"]
    S --> R["recv<br/>uart_recv_test()"]
    S --> F["file<br/>uart_file_test()"]

    L --> H["parse_hex_string()<br/>ASCII/HEX 编解码"]
    T --> H
    F --> H

    R --> W["uart_recv_with_timeout()<br/>poll 超时接收"]

    F --> J["json_config.c<br/>parse_json_file() / validate_json_config()"]
    J --> CJ["third_party/cJSON"]
    J --> CF["configs/*.json"]

    L --> IO["uartdev_send()/recv()/flush()"]
    T --> IO
    R --> IO
    F --> IO
    W --> IO
    IO --> K

    M --> X["资源回收<br/>uartdev_del() + free_config()"]
```

## 模块职责

- `main.c`：流程编排与生命周期管理（信号、模式选择、资源释放）。
- `args_parser.c`：命令行解析、默认值填充、参数合法性校验。
- `uart_assist.c`：四种业务模式实现与收发格式处理。
- `json_config.c`：`file` 模式 JSON 解析与配置校验。
- `uartdev.h`：串口底层操作封装（open/termios/read/write/poll 相关支持）。
- `third_party/cJSON`：JSON 解析依赖。
