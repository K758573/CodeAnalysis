//
// Created by shiro on 2023/3/23.
//

#ifndef ASIOZ_LOG_H
#define ASIOZ_LOG_H

#include <format>
#include <fstream>

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif

extern "C++" std::ofstream log_file_out;

#define LOG(...) \
  log_file_out << std::format("{}:{} :",__FILENAME__,__LINE__) << std::format(__VA_ARGS__) << '\n' << std::flush
//  log_file_out << std::format(__VA_ARGS__) << '\n' << std::flush
//  log_file_out << std::format("{} info [{}:{}]:",std::chrono::system_clock::now().time_since_epoch().count(),__FILENAME__,__LINE__) << std::format(__VA_ARGS__.c_str()) << '\n'

#endif //ASIOZ_LOG_H
