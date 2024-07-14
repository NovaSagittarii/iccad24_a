#ifndef COST_VERILOG_PARSER_HH
#define COST_VERILOG_PARSER_HH

#include <filesystem>

#include "verilog_driver.hpp"  // verilog parser library

class CostFunction : protected verilog::ParserVerilogInterface {
 public:
  virtual ~CostFunction() {}

  /// @brief Loads the netlist at the path into the cost function.
  /// @param file
  void LoadNetlist(const std::filesystem::__cxx11::path &file);

  /// @brief Loads the library at the path into the cost function.
  ///        This should only be called once.
  /// @param file
  void LoadLibrary(const std::filesystem::__cxx11::path &file);

  /// @brief Clears the data
  void Clear();

 protected:
  // Function that will be called when encountering the top module name.
  void add_module(std::string &&name);

  // Function that will be called when encountering a port.
  void add_port(verilog::Port &&port);

  // Function that will be called when encountering a net.
  void add_net(verilog::Net &&net);

  // Function that will be called when encountering a assignment statement.
  void add_assignment(verilog::Assignment &&ast);

  // Function that will be called when encountering a module instance.
  void add_instance(verilog::Instance &&inst);

 private:
  std::string module_name_;
  std::vector<std::string> input_ports_, output_ports, wires_;
};

#endif  // COST_VERILOG_PARSER_HH
