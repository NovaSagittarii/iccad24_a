#include <filesystem>
#include <iostream>

#include "verilog_driver.hpp" // The only include you need

// Define your own parser by inheriting the ParserVerilogInterface
struct SampleParser : public verilog::ParserVerilogInterface {
  virtual ~SampleParser() {}

  // Function that will be called when encountering the top module name.
  void add_module(std::string &&name) {
    std::cout << "Module: " << name << '\n';
  }

  // Function that will be called when encountering a port.
  void add_port(verilog::Port &&port) { std::cout << "Port: " << port << '\n'; }

  // Function that will be called when encountering a net.
  void add_net(verilog::Net &&net) { std::cout << "Net: " << net << '\n'; }

  // Function that will be called when encountering a assignment statement.
  void add_assignment(verilog::Assignment &&ast) {
    std::cout << "Assignment: " << ast << '\n';
  }

  // Function that will be called when encountering a module instance.
  void add_instance(verilog::Instance &&inst) {
    std::cout << "Instance: " << inst << '\n';
  }
};

int main(const int argc, const char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: ./sample_parser verilog_file\n";
    return EXIT_FAILURE;
  }

  if (std::filesystem::exists(argv[1])) {
    SampleParser parser;
    parser.read(argv[1]);
  }
  return EXIT_SUCCESS;
}
