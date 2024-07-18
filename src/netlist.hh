#ifndef SRC_NETLIST_HH_
#define SRC_NETLIST_HH_

#include <array>
#include <filesystem>
#include <map>

// #include "verilog_driver.hpp"  // verilog parser library
#include "gate.hh"
#include "library.hh"
#include "simple_verilog_driver.hh"  // verilog parser library (reduced functionality)

/**
 * @brief Represents a netlist, supports various netlist queries.
 */
class Netlist : protected verilog::ParserVerilogInterface {
 public:
  virtual ~Netlist() {}

  /// @brief Loads files into netlist instance. This handles parsing.
  /// @param file
  void Load(const std::filesystem::__cxx11::path &file);

  /**
   * @brief Sets the cell attribute data for every gate based on the library.
   *
   * @param lib library to load
   */
  void LoadLibrary(Library &lib);

  /**
   * @brief Computes dynamic power of the design.
   *
   * @param lib library to use
   * @return double dynamic power
   */
  double ComputeDynamicPower(const Library &lib) const;

  const auto cell_count() const { return cell_count_; }
  const auto clock_period() const { return clock_period_; }
  const auto area_constraint() const { return area_constraint_; }
  const auto power_constraint() const { return power_constraint_; }

  /**
   * @brief Returns constraints as a 3-tuple
   *
   * @return const std::array<double, 3>
   * <`clock_period`, `area_constraint`, `power_constraint`>
   */
  const std::array<double, 3> GetConstraints() const {
    return {clock_period_, area_constraint_, power_constraint_};
  }

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

  /**
   * Applies the -1234567 transformation to a string of
   * underscore-delimited (_), skipping the first `skip` items.
   * Then interprets the transformed `uint32_t*` as `char*`.
   *
   * @param s input string
   * @param skip number of entries to skip, typically 1
   *        for the module name
   * @return decoded string -- should be in the format %d_%d_%d
   */
  std::string decode(const std::string &s, int skip) const;

  std::string module_name_;
  std::vector<std::string> input_ports_, output_ports, wires_;
  std::vector<Gate> gates_;
  double clock_period_, area_constraint_, power_constraint_;

  std::map<std::string, int> cell_count_;
};

#endif  // SRC_NETLIST_HH_
