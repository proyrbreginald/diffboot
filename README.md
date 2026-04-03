# DiffBoot

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## 📋 项目简介

DiffBoot 是一个功能丰富的嵌入式引导加载程序，支持固件全量更新、固件差分升级等功能。项目采用模块化设计，便于扩展和维护。

## ✨ 主要特性

- **STM32H743 支持**: 基于 ARM Cortex-M7 内核的高性能微控制器
- **RT-Thread Nano**: 集成轻量级实时操作系统
- **差分升级**: 支持高效的固件差分更新
- **YModem 协议**: 支持通过串口进行固件传输
- **CRC16 校验**: 数据完整性验证
- **多线程支持**: 任务调度和管理
- **硬件抽象层**: 可移植的 BSP 设计

## 📁 项目结构

```txt
/
├── bsp/                  # 板级支持包
│   ├── include/          # BSP 头文件
│   └── source/           # BSP 源文件
├── include/              # 公共头文件
│   ├── algo/             # 算法模块（CRC16 等）
│   ├── load/             # 加载模块
│   └── thread/           # 线程管理（引导、监控）
├── source/               # 源代码
│   ├── algo/             # 算法实现
│   ├── load/             # 加载器实现
│   └── thread/           # 线程实现
├── libs/                 # 第三方库
│   ├── arch/             # 架构相关代码
│   ├── cubemx/           # STM32CubeMX 生成的代码
│   ├── detools/          # 差分工具库
│   ├── libc/             # C 标准库
│   ├── libstm32h743/     # STM32H743 外设库 (子模块)
│   ├── rtthread/         # RT-Thread RTOS
│   └── ymodem/           # YModem 协议实现
├── hardware/             # 硬件资料
│   └── schematic/        # 原理图 (PDF)
├── .eide/                # EIDE 项目配置
├── .vscode/              # VSCode 配置
├── Doxyfile              # Doxygen 文档配置
└── LICENSE               # MIT 许可证
```

## 🛠️ 开发环境

### 必需工具

- **编译器**: ARM GCC 或 MDK-ARM
- **IDE**:
  - VS Code + [EIDE](https://github.com/EmbeddedXplorer/EIDE) (推荐)
  - Keil MDK
- **调试工具**: J-Link / ST-Link / Dap-Link

### 构建步骤

1. 克隆仓库（包含子模块）:

```bash
git clone --recursive <repository-url>
cd diffboot
```

2. 如果已克隆但未初始化的子模块:

```bash
git submodule update --init --recursive
```

3. 使用 EIDE 打开项目并构建

## 📖 使用说明

### 模块说明

#### 算法模块 (`algo`)

- CRC16 校验计算
- 其他数据处理算法

#### 加载模块 (`load`)

- 固件加载器
- 支持多种固件格式

#### 线程模块 (`thread`)

- 引导线程 (`boot.h`)
- 监控线程 (`monitor.h`)

#### RT-Thread 组件

- 线程管理
- 信号量/互斥量
- 消息队列
- 定时器
- 内存管理
- 环形缓冲区

## 📄 文档

项目使用 Doxygen 生成 API 文档：

```bash
doxygen Doxyfile
```

生成的文档将输出到 `html/` 目录。

## 🤝 贡献

欢迎贡献代码！请遵循以下步骤：

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 📝 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 👤 作者

- **proyrbreginald** - [GitHub](https://github.com/proyrbreginald)
- 邮箱: proyrb@yeah.net

## 🔗 相关链接

- [RT-Thread 官方文档](https://www.rt-thread.io/document/)
- [STM32H743 参考手册](https://www.st.com/en/microcontrollers-microprocessors/stm32h743.html)
- [ARM Cortex-M7 技术参考手册](https://developer.arm.com/documentation/ddi0489/latest/)

## 📧 联系方式

如有问题或建议，请通过以下方式联系：
- Email: proyrb@yeah.net
- 提交 Issue

---

*最后更新: 2026 年*
