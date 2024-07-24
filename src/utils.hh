/**
 * @file utils.hh
 * @author
 * @brief Utility functions (random for now)
 * @version 0.1
 * @date 2024-07-24
 */

#ifndef SRC_UTILS_HH_
#define SRC_UTILS_HH_

#include <random>

/**
 * randomly pick one from an array
 */
template <class T>
const T choice(const std::vector<T>& arr) {
  return arr.at(std::rand() % arr.size());
}

template <class T>
const T* choice_ptr(const std::vector<T>& arr) {
  return &arr.at(std::rand() % arr.size());
}

#endif  // SRC_UTILS_HH_
