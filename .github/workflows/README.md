# CI/CD Pipeline

本文档描述 [`.github/workflows/ci.yml`](ci.yml) 的实际行为。

## 概述

为 QuickStart 提供跨平台自动化构建。共 5 个构建作业并行执行，
外加一个发布作业和一个状态汇总作业。

## 触发条件

| 事件 | 条件 |
| --- | --- |
| `push` | 推送到 `main` 或 `master` 分支 |
| `push` | 推送 `v*` 标签（如 `v2.4.3`） |
| `workflow_dispatch` | 手动触发，附带 `enable_nsis` 开关 |

> **不监听 pull_request**，PR 不会触发构建。

## 作业结构

```
code-quality-checks
        │
        ├── build-windows-mingw ──┐
        ├── build-linux-gcc ──────┤
        ├── build-linux-arm64 ────┼── create-release ── summary
        └── build-macos-clang ────┘
```

所有构建作业都 `needs: [code-quality-checks]`，因此代码质量检查失败会阻断后续构建。

## 支持的平台

| 作业 | Runner | 构建器 | 说明 |
| --- | --- | --- | --- |
| `build-windows-mingw` | `windows-latest` | MinGW Makefiles | Qt 6.5.3（`install-qt-action`） |
| `build-linux-gcc` | `ubuntu-latest` | Ninja | 系统包 Qt6 + `libyaml-cpp-dev` |
| `build-linux-arm64` | `ubuntu-24.04-arm` | Ninja | 原生 ARM64 runner |
| `build-macos-clang` | `macos-latest` | Ninja | Homebrew Qt + `yaml-cpp` |

> 只支持 MinGW，**不含 MSVC 构建**。

## 依赖安装策略

### yaml-cpp（必需依赖）

`CMakeLists.txt` 中 `find_package(yaml-cpp CONFIG REQUIRED)` 为硬依赖，
缺少时会在 CMake 配置阶段直接失败。各平台安装方式：

| 平台 | 方式 |
| --- | --- |
| Linux / ARM64 | `apt-get install libyaml-cpp-dev` |
| macOS | `brew install yaml-cpp`，并传 `-Dyaml-cpp_DIR` |
| Windows | `vcpkg install yaml-cpp:x64-mingw-dynamic` |

Windows 上 vcpkg 通常已预装于 `C:\vcpkg`，但 runner **不会**导出
`VCPKG_ROOT`，脚本会主动探测该路径；探测失败会明确报错而非静默继续。

### macOS 的 Qt 路径

Homebrew 中承载 Qt6 的是 `qt`，`qt@6` 仅为别名。脚本会先尝试
`$(brew --prefix qt)/lib/cmake/Qt6`，失败再回退 `qt@6`，都没有则直接退出。

> 变量命名注意：shell 变量不能含连字符，因此使用 `YAML_CPP_DIR`，
> 传给 CMake 时才写回官方的 `yaml-cpp_DIR`。

## 缓存策略

重依赖走 `actions/cache` 恢复，**缓存键包含版本号**，升级依赖时自动失效重建。

| 缓存内容 | 缓存键 | 失效条件 |
| --- | --- | --- |
| Qt 安装目录 | `qt-<os>-6.5.3-win64_mingw-<hash>` | 改 `version:` 或 `CMakeLists.txt` 变更 |
| vcpkg 的 yaml-cpp 产物 | `vcpkg-yaml-cpp-x64-mingw-dynamic-0.9.0-<os>` | 改 yaml-cpp 版本号 |
| ccache | `ccache-<os>-<arch>-qt6-yamlcpp` | 依赖列表变化 |
| Homebrew 下载 | `brew-<os>-qt6-yamlcpp` | 依赖列表变化 |

缓存命中时会跳过对应的安装步骤（`if: steps.cache-*.outputs.cache-hit != 'true'`）。

> 首次运行必然 miss，第二次起才享受加速。

## 代码质量检查

`code-quality-checks` 在 `ubuntu-latest` 上安装 `clang-tidy`、`cppcheck`、`valgrind`，
但**实际只执行 cppcheck**，且末尾带 `|| echo`，因此不会阻断构建。

```bash
cppcheck --enable=all --suppress=missingIncludeSystem \
  --suppress=unmatchedSuppression --error-exitcode=1 --inline-suppr \
  --std=c++17 --platform=unix64 -I include \
  -I /usr/include/x86_64-linux-gnu/qt6 src/
```

> `clang-tidy` 与 `valgrind` 仅被安装，未接入任何步骤。

## 发布

仅在推送 `v*` 标签、且四个平台构建**全部成功**时创建 Release：

- 产物包含各平台的 ZIP / tar.gz / exe
- Release 说明由 `generate_release_notes` 自动生成

任一平台失败则不发布（`create-release` 作业被跳过）。

## 构建总结

`summary` 作业用 `if: always()` 执行，无论成败都会打印各作业结果；
存在失败作业时以退出码 1 结束，让总体状态如实反映。

## 环境变量

| 变量 | 值 | 说明 |
| --- | --- | --- |
| `BUILD_TYPE` | `Release` | 构建类型 |
| `PROJECT_NAME` | `QtAppLauncher` | 项目名（对应 CMake target） |
| `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24` | `true` | 强制 JS action 使用 Node 24 |

> Qt 版本**没有**通过环境变量配置，而是直接写在 `install-qt-action`
> 的 `version:` 字段与缓存键中。修改时两处需同步。

## 产物

| 平台 | 产物 |
| --- | --- |
| Windows | `*.zip`（CPack）、`bin/*.exe` |
| Linux | `*.tar.gz`、`bin/*` |
| macOS | `*.zip`（CPack）、`bin/*` |

代码质量报告上传为 `code-quality-reports` artifact，保留 7 天。

## 故障排除

### CMake 找不到 yaml-cpp

确认依赖已安装，且 `find_package` 能定位到配置文件：

```bash
cmake -B build -S . -Dyaml-cpp_DIR=/path/to/lib/cmake/yaml-cpp
```

### CMake 找不到 Qt

```bash
cmake -B build -S . -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x
```

### Windows 报 toolchain file 找不到

`VCPKG_ROOT` 未设置。vcpkg 一般在 `C:\vcpkg`，请显式设置：

```powershell
$env:VCPKG_ROOT = "C:\vcpkg"
```

## 相关文件

- [`ci.yml`](ci.yml) — 流水线定义
- [`CMakeLists.txt`](../../CMakeLists.txt) — 构建配置
- [`CMakePresets.json`](../../CMakePresets.json) — 本地预设
