#include "verilog_parser.hh"

#include <cmath>
#include <iostream>

void CostFunction::LoadNetlist(const std::filesystem::path &file) {
  netlist_.Load(file);
}

void CostFunction::LoadLibrary(const std::filesystem::path &file) {
  library_.Load(file);
}

double CostFunction::Evaluate() {
  double area = 0, power = 0;
  for (auto [cell_name, cells_used] : netlist_.cell_count()) {
    const auto &cell = library_.cells().at(cell_name);
    area += cell.area() * cells_used;
    power += cell.leakage_power() * cells_used;
  }
  auto [c0, a0, p0] = netlist_.GetConstraints();

  double dynamic_power = 0;
  double cost = area * (power + dynamic_power);
  if (area >= a0 || (dynamic_power + p0 >= 0 && power >= p0)) {
    cost += 2e7;
  }
  return std::pow(cost, 0.5);
}

int main(const int argc, const char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: ./sample_parser verilog_file\n";
    return EXIT_FAILURE;
  }

  if (std::filesystem::exists(argv[1])) {
    CostFunction f;
    f.LoadNetlist(argv[1]);
    f.LoadLibrary("lib1.json");
    std::cout << "cost = " << f.Evaluate() << std::endl;
  }
  return EXIT_SUCCESS;
}
