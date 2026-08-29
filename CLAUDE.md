# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 协作语言

与用户沟通、代码注释、项目文档、commit message 一律使用**中文**；标识符、类型名、
Bazel target 名等代码元素仍用英文。注释采用 **Doxygen 风格**。

## 常用命令

Bazel 版本由 `.bazelversion` 固定（9.2.0）。机器上没有 `bazel`，用 `bazelisk` 调用，
它会自动下载并使用固定版本：

```bash
bazelisk build //...                      # 构建全部目标
bazelisk test //...                       # 运行全部测试
bazelisk test //photon/scan:scanner_test  # 运行单个测试目标
```

运行单个测试用例（注意用例名是中文）：

```bash
bazelisk test //photon/scan:scanner_test --test_filter='ScannerTest.非递归模式只看根目录一层'
```

（Bazel 的测试摘要会把中文用例名显示成 `?`，这只是它的输出编码问题，过滤与执行本身正常。）

运行命令行程序（`bazelisk run` 会切换工作目录到 runfiles，**传路径时必须用绝对路径**）：

```bash
bazelisk run //photon/cli:photon -- scan /Users/hans/Pictures/2026
```

其他：

```bash
bazelisk test --config=asan //...     # 地址消毒器；另有 --config=ubsan / debug / release
xcrun clang-format -i photon/scan/scanner.cc   # 格式化（clang-format 随 Xcode 提供，不在 PATH 中）
bazelisk clean                        # 清理输出
```

## 架构

自底向上四层，**依赖只能向下，禁止反向依赖**（详见 `docs/architecture.md`）：

- `photon/media/` — 媒体类型 `MediaType` 的定义与识别。全项目共享的"词汇表"，
  不依赖任何其他 photon 模块。新增相机格式时改这里的扩展名表。
- `photon/scan/` — 目录遍历，产出 `MediaFile` 列表。只回答"有哪些文件"，不解析文件内容。
  遍历中的单点错误（权限不足、坏链接）跳过而不中断，只有根目录不可用才返回错误。
- `photon/metadata/`（规划中）— 解析 EXIF / QuickTime，回答"何时、用什么设备拍的"。
  重型第三方解码库（LibRaw、Exiv2）的依赖应收敛在这一层。
- `photon/organize/`（规划中）— 把事实转换成"该放到哪里"的决策，产出纯数据的执行计划，本身不碰磁盘。
- `photon/cli/` — 唯一执行副作用（打印、落盘）的层，负责参数解析、输出格式与退出码。

### 领域约束：默认不破坏

处理的是用户不可再生的原始素材。任何会改动磁盘的功能，都必须先生成可预览的执行计划
（dry-run），由用户确认后才执行；优先复制而非移动，优先移动而非删除。
RAW+JPEG 双格式文件与 `.xmp` / `.aae` 等附属文件属于同一次拍摄，整理时不可被拆散。

### 错误处理

统一用 `absl::Status` / `absl::StatusOr<T>`，**不使用 C++ 异常**。
可预期的失败返回带中文说明和具体路径的 Status；违反前置条件的编程错误用 `CHECK` 让它尽早崩溃。

## 代码约定

- C++20；风格基于 Google C++ Style，由 `.clang-format` 约束（行宽 100 列）。
- 命名空间为 `photon::<模块名>`，实现细节放在匿名 namespace 中。
- include 路径从仓库根算起：`#include "photon/media/media_type.h"`。
- 头文件卫哨形如 `PHOTON_SCAN_SCANNER_H_`。
- 每个 `.h` / `.cc` 顶部写 `@file` + `@brief`；公开函数写 `@brief` / `@param` / `@return`；
  枚举成员与结构体字段用行尾 `///<` 注释。
- 测试套件名用英文、用例名用中文，如 `TEST_F(ScannerTest, 默认跳过隐藏文件与隐藏目录)`。
  测试写临时文件时使用 Bazel 提供的 `TEST_TMPDIR` 环境变量。

## Bazel 约定

- 每个源码目录一个 `BUILD.bazel`；Bazel 9 已移除全局的原生 C++ 规则，
  **必须显式 load**：`load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")`。
- 库的可见性默认写 `package(default_visibility = ["//photon:__subpackages__"])`，
  只有 `photon/cli` 对外公开。
- 依赖 label 形如 `@abseil-cpp//absl/status:statusor`、`@googletest//:gtest_main`；
  测试专用依赖在 `MODULE.bazel` 中标 `dev_dependency = True`。
- 新增第三方依赖优先用 Bazel Central Registry 的 `bazel_dep`；BCR 没有时再用
  `http_archive` 并把自写的 BUILD 文件放到 `third_party/<库名>/`。避免依赖系统预装库。
- `MODULE.bazel.lock` 纳入版本管理，改动依赖后需连同它一起提交。
- 个人本地配置写到 `user.bazelrc`（已被 `.bazelrc` 的 `try-import` 引入且不纳入版本管理）。

## 提交约定

遵循 Conventional Commits，描述用中文，scope 用模块目录名（`media` / `scan` / `cli` /
`metadata` / `organize` / `build` / `docs`）：

```
feat(scan): 支持按文件头校验媒体类型
fix(media): 修正 .heif 扩展名未被识别的问题
```

常用 type：`feat` / `fix` / `docs` / `refactor` / `test` / `perf` / `build` / `chore`。
