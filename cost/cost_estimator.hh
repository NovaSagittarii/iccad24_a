#ifndef COST_VERILOG_PARSER_HH
#define COST_VERILOG_PARSER_HH

#include <filesystem>

#include "library.hh"
#include "netlist.hh"

class CostFunction {
 public:
  virtual ~CostFunction() {}

  /// @brief Loads the netlist at the path into the cost function.
  /// @param file
  void LoadNetlist(const std::filesystem::__cxx11::path &file);

  /// @brief Loads the library at the path into the cost function.
  ///        This should only be called once.
  /// @param file
  void LoadLibrary(const std::filesystem::__cxx11::path &file);

  /// @brief Evaluates the cost function based on the current netlist and
  /// library.
  double Evaluate();

 private:
  Library library_;
  Netlist netlist_;
};

#endif  // COST_VERILOG_PARSER_HH
