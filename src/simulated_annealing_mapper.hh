#ifndef SRC_SIMULATED_ANNEALING_MAPPER_HH_
#define SRC_SIMULATED_ANNEALING_MAPPER_HH_

#include <filesystem>
#include <functional>
#include <iostream>
#include <vector>

#include "iterative_technology_mapper.hh"

/**
 * Technology mapping using simulated annealing.
 *
 * TODO: use time based temperature schedule instead of iteration based (?)
 */
class SimulatedAnnealingMapper : public IterativeTechnologyMapper {
 public:
  typedef std::function<double(double, int)> TemperatureSchedule;
  typedef std::function<double(const SimulatedAnnealingMapper&)> CostEstimator;
  typedef std::function<void(SimulatedAnnealingMapper&, bool)> Transition;
  SimulatedAnnealingMapper(const std::filesystem::path& output_path,
                           std::ostream& os)
      : output_path_(output_path), os_(os) {}

  /**
   * @brief Runs SA using starting temperature `t1` for `iter` iterations and
   * `runs` runs in total.
   *
   * @param temperature_schedule takes `(double)initial_temperature`,
   * `(int)iteration` and outputs `(double)temperature`
   * @param cost_estimator used to compute energy. takes an instance of
   * `SimulatedAnnealingMapper` and returns `(double)cost`
   * @param transitions a vector of invertible transitions. each transition
   * takes `SimulatedAnnealingMapper` and `(bool)undo`.
   * @param initial_temperature
   * @param iterations
   * @param runs
   */
  void Run(TemperatureSchedule temperature_schedule,
           CostEstimator cost_estimator, std::vector<Transition> transitions,
           double initial_temperature, int iterations, int runs = 1);

 protected:
  /**
   * @brief Acceptance probability function in SA.
   *
   * @param E previous energy
   * @param Ep current energy
   * @param T temperature
   * @return double probability to accept state in [0.0, 1.0]
   */
  double AcceptProbability(double E, double Ep, double T);

 private:
  std::filesystem::path output_path_;  // where the output netlist goes
  std::ostream& os_;                   // where to write debug info to
};

#endif  // SRC_SIMULATED_ANNEALING_MAPPER_HH_