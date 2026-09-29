# QuickStart

一个基于 Qt 的应用程序启动器，用分组的方式把常用程序和命令集中起来，双击即可运行。

## 功能特性

- 简约的图标网格界面，支持浅色 / 深色两套主题
- 分组管理应用，可无限层级嵌套
- 为每个分组绑定一组命令，一键执行
- 未设置图标的条目自动生成带首字母的图标
- 中英双语界面切换
- 跨平台构建（Windows / Linux / macOS）

## 环境依赖

| 依赖 | 版本要求 | 说明 |
| --- | --- | --- |
| CMake | 3.16+ | 构建系统 |
| Qt | 5.15 或 6.x | 仅需 Widgets 模块 |
| C++ 编译器 | 支持 C++17 | GCC / Clang / MSVC / MinGW |
| yaml-cpp | 0.7+ | 配置读写，**必需** |

> yaml-cpp 是硬依赖：CMake 使用 `find_package(yaml-cpp CONFIG REQUIRED)`，
> 缺少它会在**配置阶段**直接失败。

## 构建

### 通用方式

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

若 CMake 找不到 Qt 或 yaml-cpp，用 `CMAKE_PREFIX_PATH` 显式指定：

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/path/to/Qt/6.x;/path/to/yaml-cpp"
```

产物位于 `build/bin/QtAppLauncher`（Windows 下为 `QtAppLauncher.exe`）。

### 各平台安装依赖

<details>
<summary><b>Linux (Debian / Ubuntu)</b></summary>

```bash
sudo apt-get install build-essential cmake ninja-build \
  qt6-base-dev qt6-tools-dev qt6-tools-dev-tools libqt6widgets6 \
  libyaml-cpp-dev
```

Qt 5 替换为 `qtbase5-dev`。
</details>

<details>
<summary><b>macOS (Homebrew)</b></summary>

```bash
brew install cmake ninja qt yaml-cpp
```

配置时指定前缀：

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DQt6_DIR="$(brew --prefix qt)/lib/cmake/Qt6" \
  -Dyaml-cpp_DIR="$(brew --prefix yaml-cpp)/lib/cmake/yaml-cpp"
```

> 注意：Homebrew 中承载 Qt6 的 formula 是 `qt`，`qt@6` 仅为别名。
> 命令行变量名不能含连字符，所以 shell 侧用 `YAML_CPP_DIR`，
> 传给 CMake 时再写回 `yaml-cpp_DIR`。
</details>

<details>
<summary><b>Windows (vcpkg)</b></summary>

```powershell
vcpkg install yaml-cpp:x64-mingw-dynamic
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic
```

vcpkg 通常已预装在 `C:\vcpkg`，但**不会**自动导出 `VCPKG_ROOT`，
需要自行设置后再调用。
</details>

## 配置

配置保存在**当前工作目录**下的 `config.yaml`（找不到时回退读取旧版 `config.json`）。
因此从不同目录启动会读到不同配置。

```yaml
name: "Home"
iconPath: ""
language: "zh_CN"        # 界面语言：en / zh_CN
theme: "light"           # 主题：light / dark
subApps:
  - name: "Dev Tools"
    iconPath: ""
    subApps:
      - name: "Qt Creator"
        iconPath: ""
        subApps: []
        funcs: []
    funcs: []
funcs:
  - name: "Notepad"
    iconPath: ""
    cmds:
      - "notepad.exe"
```

在程序内修改主题、语言或增删条目后会自动写回该文件。
菜单「开始 → Open config」可直接用编辑器打开。

## 界面设计

界面遵循「简约」原则，细节见 [`resources/style.qss`](resources/style.qss)：

- **单一强调色**：全局只用一种低饱和蓝，仅出现在主按钮、焦点态和选中态
- **层次靠留白**：卡片为 1px 细描边 + 纯色填充，不使用投影
- **图标低饱和**：程序自带图标为柔和中性色，避免网格中相互争抢
- **深浅同源**：深色主题与浅色主题色相一致，仅明度反转

尺寸常量集中在 [`src/iconlistdelegate.cpp`](src/iconlistdelegate.cpp) 顶部，
网格尺寸与卡片几何共用同一组数值，修改时不会失配。

## 持续集成

`.github/workflows/ci.yml` 在 push 到 `main` / `master` 及推送 `v*` 标签时触发，
覆盖 Linux (x86_64 / ARM64)、Windows (MinGW)、macOS (Clang) 四个平台。

依赖安装策略：

- 平台镜像已预装的工具（MinGW、vcpkg、CMake、Ninja）不重复安装
- 较重的依赖走缓存：Windows 缓存 Qt 与 vcpkg 构建产物，Linux/macOS 缓存 ccache
- **缓存键包含依赖版本号**，升级版本时缓存自动失效并重新构建

打 `v` 前缀标签时，全部平台构建成功后会自动创建 Release。

## 开发

### 代码风格

```bash
clang-format -i include/*.h src/*.cpp   # 格式化
clang-tidy include/*.h src/*.cpp -- -Iinclude   # 静态分析
```

### 内存所有权约定

配置树由 `AppItem` 持有，所有子项通过 `addSubApp()` / `addFunc()` 挂到父节点下。
修改代码时请注意：

- **对话框返回的条目不带 QObject 父对象**，由调用方转交给配置树。
  若转交失败必须显式释放，否则泄漏。
- **导航父子关系由 `MainWindow::m_parentWindow` 维护**，而非 `QWidget::parent()`。
  子窗口带 `WA_DeleteOnClose`，若同时作为 QWidget 子对象会被重复删除。
- **配置树只由主窗口释放**（`ownsRootItem` 为真时）。
  子窗口共享 `rootItem` 指针但不得删除，否则二次释放。
- 模态对话框会运行嵌套事件循环，期间 `currentItem` / `contextMenuItem`
  可能失效，**使用前需重新校验**。

## 版本号

版本号只有一处来源：仓库根目录的 [`VERSION`](VERSION)。

CMake 读取该文件并生成 `build/config.h`，程序通过 `AppConfig::APP_VERSION`
读取（显示在「开始 → About」中），CPack 打包名与 Release 也使用同一版本。

```bash
# 升级版本
echo "2.5.0" > VERSION
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release   # 需重新配置
cmake --build build
```

打 `v` 前缀标签（如 `v2.5.0`）时，CI 会在四个平台构建成功后自动发布。

## 许可证

本项目基于 [LICENSE](LICENSE) 文件中的许可证条款发布。
