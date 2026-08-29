#!/usr/bin/env bash
#
# 代码风格检查：clang-format、cpplint、clang-tidy 三件套。
# 本地与 CI 跑的是同一个脚本，结果应当一致。
#
# 用法：
#   tools/lint.sh          # 只检查，有问题则以非零码退出
#   tools/lint.sh --fix    # 用 clang-format 就地修复格式，再跑其余检查
#
# 前置条件：python3 -m pip install -r tools/requirements-lint.txt

# 注意：macOS 自带 bash 3.2，脚本需兼容它（不可用 mapfile、空数组等特性）。
set -eo pipefail

cd "$(dirname "$0")/.."

FIX=0
if [[ "${1:-}" == "--fix" ]]; then
  FIX=1
fi

BAZEL="${BAZEL:-bazelisk}"
if ! command -v "$BAZEL" >/dev/null 2>&1; then
  BAZEL=bazel
fi

# 待检查的源文件：仓库自有代码，不含第三方。
SOURCES=()
while IFS= read -r file; do
  SOURCES+=("$file")
done < <(find photon -name '*.cc' -o -name '*.h' | sort)
if [[ ${#SOURCES[@]} -eq 0 ]]; then
  echo "未找到任何源文件" >&2
  exit 1
fi

# 用字符串而非数组记录失败项，规避 bash 3.2 的空数组行为。
FAILED=

# --------------------------------------------------------------------------
# 1. clang-format：格式
# --------------------------------------------------------------------------
echo "==> clang-format (${#SOURCES[@]} 个文件)"
if [[ $FIX -eq 1 ]]; then
  clang-format -i "${SOURCES[@]}"
  echo "    已就地格式化"
elif ! clang-format --dry-run --Werror "${SOURCES[@]}"; then
  FAILED="$FAILED clang-format"
fi

# --------------------------------------------------------------------------
# 2. cpplint：Google 风格约定，配置见根目录 CPPLINT.cfg
# --------------------------------------------------------------------------
echo "==> cpplint"
if ! cpplint --quiet "${SOURCES[@]}"; then
  FAILED="$FAILED cpplint"
fi

# --------------------------------------------------------------------------
# 3. clang-tidy：静态检查，配置见根目录 .clang-tidy
#
# clang-tidy 需要知道编译参数。本项目依赖很少，直接拼出包含路径即可，
# 不必引入 compile_commands.json 生成器。新增第三方依赖时在这里补一行。
# --------------------------------------------------------------------------
echo "==> clang-tidy"
OUTPUT_BASE="$("$BAZEL" info output_base)"
EXTERNAL="$OUTPUT_BASE/external"

# Bazel 9 把外部依赖放在内容寻址缓存里，external/ 下是指向它的符号链接，
# 所以这里不能用 find -type d 过滤。
absl_dir="$(find "$EXTERNAL" -maxdepth 1 -name 'abseil-cpp*' ! -name '*.marker' | head -1)"
gtest_dir="$(find "$EXTERNAL" -maxdepth 1 -name 'googletest*' ! -name '*.marker' | head -1)"
if [[ ! -d "$absl_dir" || ! -d "$gtest_dir" ]]; then
  echo "找不到外部依赖头文件，请先执行：$BAZEL build //..." >&2
  exit 1
fi

# 第三方头文件用 -isystem 引入，避免它们自身的告警混进结果。
# -x c++ 不可省略：clang-tidy 按扩展名判断语言，会把 .h 当成 C 头文件解析。
TIDY_FLAGS=(-x c++ -std=c++20 -I. -isystem "$absl_dir" -isystem "$gtest_dir/googletest/include")

# macOS 上的 LLVM clang-tidy 不会自动找到 SDK，需显式指定。
if [[ "$(uname -s)" == "Darwin" ]]; then
  TIDY_FLAGS+=(-isysroot "$(xcrun --show-sdk-path)")
fi

if ! clang-tidy --quiet "${SOURCES[@]}" -- "${TIDY_FLAGS[@]}"; then
  FAILED="$FAILED clang-tidy"
fi

# --------------------------------------------------------------------------
echo ""
if [[ -n "$FAILED" ]]; then
  echo "检查未通过：$FAILED" >&2
  echo "格式问题可用 tools/lint.sh --fix 自动修复。" >&2
  exit 1
fi
echo "全部检查通过。"
