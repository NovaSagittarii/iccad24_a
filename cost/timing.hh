#ifndef COST_TIMING_HH_
#define COST_TIMING_HH_

#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

std::vector<std::chrono::_V2::system_clock::time_point> times;

/**
 * @brief Starts the clock, pushes the current time onto a the stack.
 */
void StartClock() {
  times.push_back(std::chrono::high_resolution_clock::now());
}

/**
 * @brief Compares the time with the top of the time stack. Pops off the top.
 *
 * @return double time elasped
 */
double EndClock() {
  auto t0_ = times.back();
  times.pop_back();
  using std::chrono::duration;
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;
  auto t1 = std::chrono::high_resolution_clock::now();
  // auto ms_int = duration_cast<milliseconds>(t1 - t0_);
  // return ms_int.count();
  duration<double, std::milli> ms_double = t1 - t0_;
  return ms_double.count();
}

/**
 * @brief Calls `EndClock()` and prints the output
 *
 * @param label text printed before the elapsed time.
 */
void EndClockPrint(const std::string& label) {
  std::cout << std::fixed << std::setprecision(6);
  std::cout << std::setw(10) << EndClock() << "ms" << " " << label << std::endl;
}

#endif  // COST_TIMING_HH_