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

  const auto& name() const { return name_; }
  const auto& type() const { return type_; }
  const auto& leakage_power() const { return leakage_power_; }
  const auto& area() const { return area_; }

  /**
   * @brief enumerable for cell types
   * x&1 ~ inverted, x&8 ~ one-input
   */
  enum Type {
    kUnknown = 0,
    kOr = 2,
    kNor = 3,
    kAnd = 4,
    kNand = 5,
    kXor = 6,
    kXnor = 7,
    kBuf = 24,
    kNot = 25,

    kMaskInverted = 1,  // if (type & Type::kMaskInverted) type is inverted
    kMaskUnary = 8,     // if (type & Type::kMaskUnary) type is unary input gate
    kMaskBaseGate = 2 | 4 | 6 | 24,  // removes the isInverted bit
  };

 private:
  std::string name_;
  Type type_ = Type::kUnknown;
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