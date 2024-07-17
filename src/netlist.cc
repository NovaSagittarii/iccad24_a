#include "netlist.hh"

#include <sstream>

void Netlist::Load(const std::filesystem::__cxx11::path &file) { read(file); }

void Netlist::add_module(std::string &&name) {
  module_name_ = std::move(name);

  // std::cout << module_name_ << std::endl;
  std::istringstream s(decode(module_name_, 1));

  /// TODO: validate decoded module name
  char dummy;
  s >> clock_period_ >> dummy >> area_constraint_ >> dummy >> power_constraint_;
}

void Netlist::add_port(verilog::Port &&port) {
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

void Netlist::add_net(verilog::Net &&net) {
  // this isn't really needed, you can figure out nets from the instances

  // std::cout << "Net: " << net << '\n';
  // if (net.type == verilog::NetType::WIRE) {
  //   for (std::string &name : net.names) {
  //     wires_.push_back(name);
  //   }
  // }
}

void Netlist::add_assignment(verilog::Assignment &&ast) {
  // std::cout << "Assignment: " << ast << '\n';
}

void Netlist::add_instance(verilog::Instance &&inst) {
  // std::cout << "Instance: " << inst << '\n';

  // std::cout << inst.module_name << "\n";  // cell_name
  ++cell_count_[inst.module_name];

  for (auto &net : inst.net_names) {
    // what nets are connected
    // for (auto &x : net) std::cout << " " << std::get<std::string>(x);
    // std::cout << "\n";
  }
  // pin names is just the pin labels like A, B, Y (not used)
  // for (auto &x : inst.pin_names) std::cout << std::get<std::string>(x) <<
  // "\n";
}

std::string Netlist::decode(const std::string &s, int skip) const {
  std::istringstream in(s);
  int32_t mem[0x20] = {};
  std::string segment;

  int i = 0;
  while (std::getline(in, segment, '_')) {
    if (skip) {
      --skip;
    } else {
      mem[i++] = std::stoi(segment) - 1234567;
    }
  }

  std::string out = (char *)mem;
  return out;
}
