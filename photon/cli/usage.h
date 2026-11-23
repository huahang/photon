/**
 * @file usage.h
 * @brief photon 命令行的用法说明。
 */

#ifndef PHOTON_CLI_USAGE_H_
#define PHOTON_CLI_USAGE_H_

#include <ostream>

namespace photon::cli {

/**
 * @brief 把命令行用法说明写入给定的输出流。
 *
 * 入口在参数缺失或用户请求帮助时调用；scan 在缺少目录参数时也会调用。
 * 输出流由调用方决定，便于测试，也避免把 stderr 写死在实现里。
 *
 * @param out 接收用法说明的输出流。
 */
void PrintUsage(std::ostream& out);

}  // namespace photon::cli

#endif  // PHOTON_CLI_USAGE_H_
