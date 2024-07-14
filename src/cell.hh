#ifndef ICCAD_SRC_CELL_H_
#define ICCAD_SRC_CELL_H_

#include <string>
#include <vector>

class Cell {
 public:
  Cell() {}

  /**
   * @brief Updates the cell with the given key, value pair information.
   * Decodes based on the cost_function_1 ~ cost_function_8 attribute
   * mapping and then stores the attributes into an int and double array.
   *
   * @param key
   * @param value
   */
  void LoadProperty(const std::string& key, const std::string& value);

 private:
  std::string name_, type_;
  std::vector<double> f_properties_;
  std::vector<int> i_properties_;

  /// @brief probably propagation delay
  double a_;

  /// @brief probably transition delay
  double b_;

  /// @brief power domain
  int pd_;

  double leakage_power_;

  /// @brief capacitance
  double c_;

  double area_;

  /// @brief max capacitance
  double max_c_;
};

#endif  // ICCAD_SRC_CELL_H_