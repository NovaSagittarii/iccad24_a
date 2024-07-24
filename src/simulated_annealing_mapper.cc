#include "simulated_annealing_mapper.hh"

#include <time.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>

#include "utils.hh"

double SimulatedAnnealingMapper::AcceptProbability(double E, double Ep,
                                                   double T) {
  if (Ep < E) return 1;
  return std::exp(-(Ep - E) / T);
}

void SimulatedAnnealingMapper::Run(TemperatureSchedule temperature_schedule,
                                   CostEstimator cost_estimator,
                                   std::vector<Transition> transitions,
                                   double initial_temperature, int iterations,
                                   int runs) {
  double E = cost_estimator(*this);  // current energy
  double Ep;                         // E' (E prime) -- energy in updated state
  double E_low = E;                  // lowest energy
  double T;                          // temperature

  time_t last_update = time(NULL);

  for (int tn = 0; tn < runs; ++tn) {
    E = cost_estimator(*this);
    for (int it = 0; it < iterations; ++it) {
      // progress display BEGIN_SECTION
      if (it % 100 == 0) {
        os_ << std::fixed << "epoch=" << std::setw(5) << tn
            << " iter=" << std::setw(10) << it << " T=" << std::setw(10)
            << std::setprecision(7) << T << std::scientific
            << " curr = " << std::setw(10) << E << " best = " << std::setw(10)
            << E_low << std::endl;
        time_t current_time = time(NULL);
        if (current_time <= last_update + 10) {
          std::cout << "\u001b[1F\u001b[1K";
        } else {
          last_update = current_time;
        }
      }
      // progress display END_SECTION

      T = temperature_schedule(initial_temperature, it);

      // apply change
      int transition_id = it % transitions.size();
      const auto& transition = transitions[transition_id];
      transition(*this, false);

      Ep = cost_estimator(*this);  // compute new temperature
      if (AcceptProbability(E, Ep, T) * RAND_MAX >= std::rand()) {
        E = Ep;
        if (E < E_low) {  // best seen so far?
          E_low = E;
          WriteMapping(output_path_);
        }
      } else {
        // undo change
        transition(*this, true);
      }
    }
  }

  os_ << std::endl;
}

int32_t main() {
  std::srand(0);

  SimulatedAnnealingMapper mapper("a_out.v", std::cout);
  mapper.Load("design1.aig");
  mapper.LoadLibrary("lib1.json");
  mapper.Initialize();
  mapper.WriteVerilogABC("a_logic_before.v");

  static const SimulatedAnnealingMapper::TemperatureSchedule
      temperature_schedule =
          [](double t1, int i) -> double { return t1 * 0.2 / (i / 10); };

  static const auto cost =
      [](const SimulatedAnnealingMapper& mapper) -> double {
    const double area = mapper.area();
    const double power = mapper.power();
    const double dynamic_power = mapper.dynamic_power();
    double penalty = 0;
    // if (area >= 500 || power > 0.15)
    //   penalty = 1e7 * (area / 500 + power / 0.15);
    return std::pow(penalty + (1 + area) * (1 + power + dynamic_power), 0.5);
  };

  typedef SimulatedAnnealingMapper SAM;
  typedef const SimulatedAnnealingMapper::Transition Transition;
  static Transition add_random_gate = [](SAM& mapper, bool undo) -> void {
    if (!undo) {
      mapper.AddRandomGate();
    } else {
      mapper.UndoGateAdd();
    }
  };

  static Transition remove_random_gate = [](SAM& mapper, bool undo) -> void {
    if (!undo) {
      // mapper.RemoveBinaryGate()
    }
  };

  static Transition change_aig_gate = [](SAM& mapper, bool undo) -> void {
    static int i = -1;
    static const Cell* old_cell = nullptr;
    if (!undo) {
      i = std::rand() % (2 * mapper.sz_v());
      old_cell = mapper.aig_nodes().at(i).cell;
      Cell::Type type = i & 1 ? Cell::Type::kNot : Cell::Type::kAnd;
      auto new_cell = choice(mapper.library().GetCellsByType(type));
      mapper.ChangeAIGNodeGate(i, new_cell);
    } else {
      mapper.ChangeAIGNodeGate(i, old_cell);
    }
  };

  const std::vector<SimulatedAnnealingMapper::Transition> transitions = {
      add_random_gate,
      change_aig_gate,
  };

  // mapper.Run(temperature_schedule, cost, {add_random_gate}, 100, 1000);
  mapper.Run(temperature_schedule, cost, {change_aig_gate}, 1, 1);
  mapper.Run(temperature_schedule, cost, {change_aig_gate}, 1e-2, 1e5);
  mapper.Run(temperature_schedule, cost, transitions, 1e-2, 1e6);
  // mapper.Run(temperature_schedule, cost, transitions, 0, 1e6);
  // mapper.Run(temperature_schedule, cost, {add_random_gate}, 100, 100000);
  mapper.WriteVerilogABC("a_logic_after.v");
}
