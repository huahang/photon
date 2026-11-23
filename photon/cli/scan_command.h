/**
 * @file scan_command.h
 * @brief scan 子命令：扫描目录并按媒体类型汇总。
 */

#ifndef PHOTON_CLI_SCAN_COMMAND_H_
#define PHOTON_CLI_SCAN_COMMAND_H_

#include <ostream>
#include <string_view>
#include <vector>

namespace photon::cli {

/**
 * @brief 执行 scan 子命令。
 *
 * 成功时把汇总写到 `out`，参数错误与扫描失败写到 `err`。
 * 返回值即进程退出码：0 成功，1 扫描失败，2 参数错误。
 *
 * @param args scan 之后的全部参数。
 * @param out 汇总结果的输出流。
 * @param err 错误信息与用法说明的输出流。
 * @return 进程退出码。
 */
int RunScan(const std::vector<std::string_view>& args, std::ostream& out, std::ostream& err);

}  // namespace photon::cli

#endif  // PHOTON_CLI_SCAN_COMMAND_H_
