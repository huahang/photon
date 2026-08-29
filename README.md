# photon

用于整理相机拍摄成果的命令行工具集：把 RAW、JPEG、HEIF 照片与 MP4、MOV 视频
从存储卡与硬盘的杂乱目录里，整理成结构清晰、可长期维护的媒体库。

- 开发语言：C++20
- 构建与依赖管理：[Bazel](https://bazel.build)（bzlmod）
- 平台：macOS（当前主力开发平台），代码尽量保持 POSIX 可移植

## 快速开始

本仓库通过 `.bazelversion` 固定 Bazel 版本，推荐用 `bazelisk` 调用，它会自动下载对应版本：

```bash
# 构建全部目标
bazelisk build //...

# 运行全部测试
bazelisk test //...

# 扫描一个目录，按媒体类型汇总
bazelisk run //photon/cli:photon -- scan ~/Pictures/2026
```

## 目录结构

| 路径 | 职责 |
| --- | --- |
| `photon/media/` | 媒体类型的定义与识别（扩展名、格式家族） |
| `photon/scan/` | 目录遍历，发现媒体文件 |
| `photon/cli/` | `photon` 命令行程序入口与子命令 |
| `docs/` | 架构说明与开发规划 |

更多设计背景见 [docs/architecture.md](docs/architecture.md)，后续计划见 [docs/roadmap.md](docs/roadmap.md)。

## 开发约定

- 注释与文档使用中文，注释采用 Doxygen 风格。
- 代码风格基于 Google C++ Style，由 clang-format、cpplint、clang-tidy 三重把关：

  ```bash
  python3 -m pip install -r tools/requirements-lint.txt
  tools/lint.sh --fix
  ```

- 提交信息遵循 [Conventional Commits](https://www.conventionalcommits.org/zh-hans/)。
- `main` 分支受保护，改动一律走 Pull Request，CI 全绿后合并。

面向协作 Agent 的详细约定见 [CLAUDE.md](CLAUDE.md)。
