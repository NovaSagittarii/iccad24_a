#ifndef SRC_GATE_HH_
#define SRC_GATE_HH_

#include <string>
#include <vector>

#include "cell.hh"

class Gate {
 public:
  Gate() {}
  Gate(const std::string& name, const std::string& cell_name,
       const std::string& output)
      : name_(name), cell_name_(cell_name), output_(output) {}

  void AddInput(const std::string& input_name) {
    inputs_.push_back(input_name);
  }

  const auto& name() const { return name_; }
  const auto& cell() const { return *cell_; }
  const auto& cell_name() const { return cell_name_; }
  const auto& output() const { return output_; }
  const auto& inputs() const { return inputs_; }

  void set_cell(Cell& cell) { cell_ = &cell; }

 private:
  std::string name_, cell_name_;
  Cell* cell_ = nullptr;
  std::string output_;
  std::vector<std::string> inputs_;
};

#endif  // SRC_GATE_HH_