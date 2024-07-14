#include "verilog_parser.hh"

#include <iostream>

#include "library.hh"

void CostFunction::LoadNetlist(const std::filesystem::path &file) {
  Clear();
  read(file);
}

void CostFunction::LoadLibrary(const std::filesystem::path &file) {
  Library lib;
  lib.Load(file);
}

void CostFunction::Clear() {
  module_name_.clear();
  input_ports_.clear();
  output_ports.clear();
  wires_.clear();
}

void CostFunction::add_module(std::string &&name) {
  module_name_ = std::move(name);
}

void CostFunction::add_port(verilog::Port &&port) {
  // std::cout << "Port: " << port << '\n';
  switch (port.dir) {
    case verilog::PortDirection::INPUT:
      for (std::string &name : port.names) {
        input_ports_.push_back(name);
      }
      break;
    case verilog::PortDirection::OUTPUT:
      for (std::string &name : port.names) {
        output_ports.push_back(name);
      }
      break;
  }
}

void CostFunction::add_net(verilog::Net &&net) {
  std::cout << "Net: " << net << '\n';
  if (net.type == verilog::NetType::WIRE) {
    for (std::string &name : net.names) {
      wires_.push_back(name);
    }
  }
}

void CostFunction::add_assignment(verilog::Assignment &&ast) {
  // std::cout << "Assignment: " << ast << '\n';
}

void CostFunction::add_instance(verilog::Instance &&inst) {
  // std::cout << "Instance: " << inst << '\n';

  std::cout << inst.module_name << "\n";

  for (auto &net : inst.net_names) {
    for (auto &x : net) std::cout << " " << std::get<std::string>(x);
    std::cout << "\n";
  }
  for (auto &x : inst.pin_names) std::cout << std::get<std::string>(x) << "\n";
}

int main(const int argc, const char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: ./sample_parser verilog_file\n";
    return EXIT_FAILURE;
  }

  if (std::filesystem::exists(argv[1])) {
    CostFunction parser;
    parser.LoadNetlist(argv[1]);
    parser.LoadLibrary("lib1.json");
  }
  return EXIT_SUCCESS;
}
