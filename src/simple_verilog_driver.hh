#ifndef SRC_SIMPLE_VERILOG_DRIVER_HPP_
#define SRC_SIMPLE_VERILOG_DRIVER_HPP_

#include <filesystem>

#include "verilog_data.hpp"

namespace verilog {
class ParserVerilogInterface {
 public:
  virtual ~ParserVerilogInterface() {}

  virtual void add_module(std::string&&) = 0;
  virtual void add_port(Port&&) = 0;
  virtual void add_net(Net&&) = 0;
  virtual void add_assignment(Assignment&&) = 0;
  virtual void add_instance(Instance&&) = 0;

  /**
   * @brief loads a file and reads it, uses a simpler parsing system
   * compared to the proper lexing.
   */
  void read(const std::filesystem::path&);
};
};  // namespace verilog

#endif  // SRC_SIMPLE_VERILOG_DRIVER_HPP_
