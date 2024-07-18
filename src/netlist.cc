#include "netlist.hh"

#include <queue>
#include <sstream>

void Netlist::Load(const std::filesystem::__cxx11::path &file) { read(file); }

void Netlist::LoadLibrary(Library &lib) {
  std::map<std::string, Cell> &cells = lib.cells();
  for (Gate &gate : gates_) {
    const std::string &cell_name = gate.cell_name();
    if (cells.count(cell_name)) {
      gate.set_cell(cells.at(cell_name));
    } else {
      printf("Missing cell type %s for gate %s", cell_name, gate.name());
      throw std::logic_error("Missing cell type in library");
    }
  }
}

double Netlist::ComputeDynamicPower(const Library &lib) const {
  // map<net, vector<gate>>
  std::map<std::string, std::vector<std::string>> adj;
  // map<gate, vector<net>>
  std::map<std::string, std::vector<std::string>> radj;
  std::map<std::string, std::string> driver;  // map<gate, net>
  std::map<std::string, int> deps;            // map<gate, remaining deps>
  std::map<std::string, Cell::Type> types;    // map<gate, gate_type>
  std::map<std::string, double> set_prob;     // map<net, set_prob>
  std::queue<std::string> queue;              // things ready for processing
  std::map<std::string, Gate> gates;

  for (auto gate : gates_) {
    const Cell::Type cell_type = gate.cell().type();
    const std::string curr = gate.name();
    deps[curr] = (cell_type & Cell::Type::kMaskUnary) ? 1 : 2;
    for (std::string prev : gate.inputs()) {
      adj[prev].push_back(curr);
      radj[curr].push_back(prev);
    }
    driver[curr] = gate.output();
    gates[curr] = gate;
    types[curr] = cell_type;
  }

  for (std::string net : input_ports_) {
    set_prob[net] = 0.5;
    for (std::string gate : adj[net]) {
      if (--deps[gate] == 0) {
        queue.push(gate);
      }
    }
  }

  double dynamic_power = 0.0;
  while (!queue.empty()) {
    const std::string gate = queue.front();
    queue.pop();
    std::vector<double> P;
    for (std::string inet : radj[gate]) P.push_back(set_prob[inet]);

    // std::cout << gate << " | ";
    // for (auto x : P) std::cout << x << " ";
    // std::cout << "\n";

    double p = 0;
    // clang-format off
    switch (types[gate] & Cell::Type::kMaskBaseGate) {
      case Cell::Type::kBuf: p = P[0]; break;
      case Cell::Type::kOr:  p = 1 - (1 - P[0]) * (1 - P[1]); break;
      case Cell::Type::kAnd: p = P[0] * P[1]; break;
      case Cell::Type::kXor: p = P[0] + P[1] - (2 * P[0] * P[1]); break;
    }
    // clang-format on
    if (types[gate] & Cell::Type::kMaskInverted) p = 1.0 - p;
    set_prob[driver[gate]] = p;

    double q = 2 * p * (1 - p);
    dynamic_power += q * gates[gate].cell().leakage_power();

    for (std::string v : adj[driver[gate]]) {
      if (--deps[v] == 0) {
        queue.push(v);
      }
    }
  }
  return dynamic_power;
}

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

  std::vector<std::string> pins;
  for (auto &net : inst.net_names) {
    // what nets are connected
    // for (auto &x : net) std::cout << " " << std::get<std::string>(x);
    // std::cout << "\n";

    pins.push_back(std::get<std::string>(net.front()));
  }

  Gate gate(inst.inst_name, inst.module_name, pins.back());
  pins.pop_back();
  for (std::string pin : pins) gate.AddInput(pin);
  gates_.push_back(std::move(gate));

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
