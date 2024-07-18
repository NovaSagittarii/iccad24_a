#include "cost_estimator.hh"

#include <cmath>
#include <iostream>

#include "timing.hh"

void CostFunction::LoadNetlist(const std::filesystem::path &file) {
  StartClock();
  netlist_.Load(file);
  EndClockPrint("<load:netlist>");
}

void CostFunction::LoadLibrary(const std::filesystem::path &file) {
  StartClock();
  library_.Load(file);
  EndClockPrint("<load:library>");
}

double CostFunction::Evaluate() {
  StartClock();
  double area = 0, power = 0;
  for (auto [cell_name, cells_used] : netlist_.cell_count()) {
    const auto &cell = library_.cells().at(cell_name);
    area += cell.area() * cells_used;
    power += cell.leakage_power() * cells_used;
  }
  auto [c0, a0, p0] = netlist_.GetConstraints();

  double dynamic_power = netlist_.ComputeDynamicPower(library_);
  std::cout << std::fixed << std::setprecision(26);
  std::cout << "area      = " << area << "\n"
            << "power     = " << power << "\n"
            << "dyn_power = " << dynamic_power << std::endl;

  double cost = area * (power + dynamic_power);
  if (area >= a0 || (dynamic_power + p0 >= 0 && power >= p0)) {
    cost += 2e7;
  }
  EndClockPrint("<eval:costfunc>");
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
    double cost = f.Evaluate();
    std::cout << "cost = " << cost << std::endl;
  }
  return EXIT_SUCCESS;
}
