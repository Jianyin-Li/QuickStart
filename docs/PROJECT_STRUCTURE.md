# 项目结构说明

## 目录结构

```
QuickStart/
├── .github/                      # GitHub Actions
│   └── workflows/
│       ├── ci.yml                # CI/CD 流水线（4 平台构建）
│       └── README.md             # 工作流说明
├── docs/                         # 项目文档
│   └── PROJECT_STRUCTURE.md
├── include/                      # 公共头文件
│   ├── appconfigdialog.h         # 应用配置对话框
│   ├── appitem.h                 # 配置树节点（应用/分组）
│   ├── config.h.in               # 版本信息模板（生成 config.h）
│   ├── funcconfigdialog.h        # 函数配置对话框
│   ├── funcitem.h                # 函数条目（命令列表）
│   ├── icongenerator.h           # 程序化图标生成
│   ├── iconlistdelegate.h        # 网格卡片绘制
│   └── mainwindow.h              # 主窗口
├── resources/                    # 资源文件
│   ├── app_icon.ico              # 应用图标
│   ├── app_icon.rc               # Windows 资源脚本
│   ├── favicon.ico               # 站点图标
│   ├── style.qss                 # 界面样式表（浅色主题）
│   ├── QuickStart_en.qm          # 英文翻译（编译产物）
│   └── QuickStart_zh_CN.qm       # 中文翻译（编译产物）
├── src/                          # 源代码
│   ├── appconfigdialog.cpp
│   ├── appitem.cpp
│   ├── funcconfigdialog.cpp
│   ├── funcitem.cpp
│   ├── icongenerator.cpp
│   ├── iconlistdelegate.cpp
│   ├── main.cpp                  # 程序入口
│   └── mainwindow.cpp
├── tools/                        # 独立工具
│   └── test-tools/
│       ├── README.md
│       └── test_icon_generator.cpp
├── ui/                           # Qt Designer 界面文件
│   ├── appconfigdialog.ui
│   ├── funcconfigdialog.ui
│   └── mainwindow.ui
├── build/                        # 构建输出（不提交）
│   └── config.h                  # 由 config.h.in 生成（含版本号）
├── .clang-format                 # 代码格式化规则
├── .clang-tidy                   # 静态分析规则
├── .gitattributes                # Git 属性配置
├── .gitignore                    # Git 忽略规则
├── CMakeLists.txt                # CMake 构建配置
├── CMakePresets.json             # CMake 预设
├── config.json                   # 旧版配置（兼容回退）
├── config.yaml                   # 当前配置（运行时读写）
├── LICENSE
├── QuickStart_en.ts              # 英文翻译源
├── QuickStart_zh_CN.ts           # 中文翻译源
├── README.md
├── resources.qrc                 # Qt 资源清单
└── VERSION                       # 版本文件
```

## 关键文件说明

### 构建

| 文件 | 作用 |
| --- | --- |
| `CMakeLists.txt` | 主构建脚本。`find_package(yaml-cpp CONFIG REQUIRED)` 为硬依赖 |
| `CMakePresets.json` | 各平台预设，CI 供参考 |
| `resources.qrc` | 注册图标、样式表与 `.qm` 翻译到资源系统 |

### 界面与样式

| 文件 | 作用 |
| --- | --- |
| `resources/style.qss` | 浅色主题样式表 |
| `src/iconlistdelegate.cpp` | 网格卡片绘制；**尺寸常量集中于此** |
| `src/icongenerator.cpp` | 生成默认图标、返回箭头等矢量图形 |
| `src/mainwindow.cpp` | 深色主题以 `DARK_OVERLAYS` 常量叠加，header 样式由 `headerQss()` 统一生成 |

### 翻译

- `.ts` 为源文件，`.qm` 为编译产物（已随仓库提供）
- 运行时从 `:/i18n/` 资源路径加载
- 修改 `.ts` 后需重新编译为 `.qm` 才会生效

### 配置

- `config.yaml` — 当前主用格式
- `config.json` — 旧版格式，仅在找不到 yaml 时回退读取
- 两者都从**当前工作目录**读取，而非程序所在目录

## 架构要点

### 配置树

`AppItem` 通过 `addSubApp()` / `addFunc()` 组织成树，所有子项的 QObject 父节点
指向自己的父条目。序列化由 `toYaml()` / `fromYaml()` 完成。

### 窗口导航

点击条目会打开一个**新的顶层窗口**（无 QWidget 父对象），并记录
`m_parentWindow` 指向上一级；返回按钮据此回到上一层并关闭当前窗口。

> 之所以不用 QWidget 父子关系：子窗口带 `WA_DeleteOnClose`，
> 若同时挂在父窗口下会被删除两次。

### 内存所有权

配置树**只由主窗口释放**（`ownsRootItem == true`）；子窗口共享指针但不负责释放。
涉及 `currentItem` / `contextMenuItem` 的操作需考虑模态对话框带来的嵌套事件循环，
使用前重新校验指针。

## 构建说明

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/bin/QtAppLauncher
```

依赖安装方式与平台差异见 [README](../README.md#环境依赖)。

## 注意事项

1. `build/` 目录不提交（已在 `.gitignore` 中通过 `/build*/` 规则忽略）
2. `.qm` 为编译产物但需入库，缺失会导致界面语言回退为英文
3. **版本号只需改 `VERSION` 一处**：CMake 会读取它生成 `build/config.h`，
   C++ 侧通过 `APP_VERSION_STRING` 获取（见 `include/config.h.in`）。
   修改后需重新运行 CMake 配置。
