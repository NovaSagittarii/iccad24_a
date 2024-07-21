#ifndef SRC_AIG_HH_
#define SRC_AIG_HH_

#include <array>
#include <cassert>
#include <filesystem>
#include <vector>

/**
 * @brief Represents a sequential And-Inverter Graph. Handles read/write, but
 * mainly holds data for IterativeTechnologyMapper to access.
 */
class AIG {
 public:
  AIG() {}

  /**
   * @brief Loads the file into the AIG, parses and then finalizes.
   *
   * @param file source path
   */
  void Load(const std::filesystem::path& file);

  /**
   * @brief Writes the mapping (in the weird verilog output form) to a file.
   * Note: In weird verilog, the output port is the last parameter instead of
   * the first parameter.
   *
   * @param file destination path
   */
  void Write(const std::filesystem::path& file) const;

  /**
   * @brief Loads the header
   *
   * @param v number of variables
   * @param i number of input variables
   * @param o number of output variables
   * @param a number of AND gates
   */
  void LoadHeader(int v, int i, int o, int a);

  /**
   * @brief Update `inputs_[index] = variable;`
   *
   * @param index index into the `inputs_` array
   * @param variable 0-indexed variable/literal of what the input is
   */
  void SetInputPort(int index, int variable) { inputs_[index] = variable; }

  /**
   * @brief Update `outputs_[index] = variable;`
   *
   * @param index index into the `outputs_` array
   * @param variable 0-indexed variable/literal of what the input is
   */
  void SetOutputPort(int index, int variable) { outputs_[index] = variable; }

  /**
   * @brief Update variable (AND gate)'s dependencies.
   * This orders lhs and rhs such that lhs <= rhs.
   *
   * @param variable which AND gate to update
   * @param lhs
   * @param rhs
   */
  void SetNodeDependencies(int variable, int lhs, int rhs) {
    assert(!(variable & 1) && "NOT gates do not have explicit dependencies.");
    assert(variable >= sz_i_ * 2 && "Inputs do not have dependencies.");
    assert(lhs != rhs && "Dependencies should be distinct, it is AND not BUF.");
    if (lhs > rhs) std::swap(lhs, rhs);
    nodes_[variable].inputs = {lhs, rhs};
  }

  void SetModuleName(const std::string& module_name) {
    top_module_name_ = module_name;
  }

  /**
   * @brief Sets the net name of the ith input
   *
   * @param input_index
   * @param name
   */
  void SetInputNetName(int input_index, const std::string& name) {
    nodes_[inputs_[input_index]].name_index = net_names_.size();
    net_names_.push_back(name);
  }

  /**
   * @brief Sets the net name of the ith output
   *
   * @param output_index
   * @param name
   */
  void SetOutputNetName(int output_index, const std::string& name) {
    nodes_[outputs_[output_index]].name_index = net_names_.size();
    net_names_.push_back(name);
  }

 protected:
  /**
   * @brief Represents an AIG node, it's index in the node list determines
   * whether it is an AND gate or INVERTER.
   */
  struct Node {
    double set_prob;      // set probability (used in dynamic power)
    double q;             // leakage coefficient used in dynamic power q=2p(1-p)
    int covered_by = -1;  // index of the Gate this AIGNode is covered by
    int out_degree = 0;   // how many other nodes depend on this
    int name_index = -1;  // index of net name, -1 if not an I/O port

    /**
     * @brief The inputs (dependencies) of the And gate. When this represents
     * an Inverter gate, the inputs stay as {-1, -1};
     */
    std::array<int, 2> inputs = {-1, -1};
  };

  /**
   * @brief Represents a gate mapping onto the AIG.
   */
  class Gate {
   public:
   private:
    bool active_ = false;
  };

  /**
   * @brief Computes the outdegree for all AIG nodes. Run this after the .aig
   * file is finished being read.
   */
  void ComputeOutDegree();

  int sz_v_;  // number of variables
  int sz_i_;  // number of inputs
  int sz_o_;  // number of outputs
  int sz_a_;  // number of AND gates

  std::vector<int> inputs_;   // variable name of the ith input
  std::vector<int> outputs_;  // variable name of the ith output

  std::vector<Node> nodes_;
  std::vector<Gate> gates_;

 private:
  /**
   * @brief Helper function to computes the set probability for all AIG nodes.
   * Load() invokes this at the end.
   */
  void ComputeSetProbability();

  /**
   * @brief Recursive computation for ComputeSetProbability
   *
   * @param u variable that is being currently computed
   */
  void ComputeSetProbabilityRecursive(int u);

  std::string top_module_name_;  // name of the top module (used in export)
  std::vector<std::string> net_names_;  // node name_index index into here
};

#endif  // SRC_AIG_HH_