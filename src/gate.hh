#ifndef SRC_GATE_HH_
#define SRC_GATE_HH_

#include <string>
#include <vector>

class Gate {
 public:
  Gate() {}
  Gate(const std::string& name, const std::string& cell,
       const std::string& output)
      : name_(name), cell_(cell), output_(output) {}

  void AddInput(const std::string& input_name) {
    inputs_.push_back(input_name);
  }

  const auto& name() const { return name_; }
  const auto& cell() const { return cell_; }
  const auto& output() const { return output_; }
  const auto& inputs() const { return inputs_; }

 private:
  std::string name_, cell_;
  std::string output_;
  std::vector<std::string> inputs_;
};

#endif  // SRC_GATE_HH_