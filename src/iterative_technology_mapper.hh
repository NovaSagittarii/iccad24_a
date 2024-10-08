#ifndef SRC_ITERATIVE_TECHNOLOGY_MAPPER_
#define SRC_ITERATIVE_TECHNOLOGY_MAPPER_

#include "aig.hh"
#include "cell.hh"
#include "library.hh"

/**
 * @brief Iterative technology mapper for an And-Inverter graph.
 * Each update (query) recomputes area, power, dynamic power efficiently.
 */
class IterativeTechnologyMapper : public AIG {
 public:
  IterativeTechnologyMapper() {}

  struct GateMapping {
    GateMapping(int a, int b, int y, Cell::Type type)
        : a(a), b(b), y(y), type(type) {}
    int a = -1;       // input A
    int b = -1;       // input B (not used in unary gates)
    int y = -1;       // output Y
    Cell::Type type;  // gate type
    // std::vector<int> covers;  // what literals this replaces
  };

  /**
   * @brief Loads the library at the path into the cost function.
   * This should only be called once.
   * @param file
   */
  void LoadLibrary(const std::filesystem::__cxx11::path &file);

  const double area() const { return area_; }
  const double power() const { return power_; }
  const double dynamic_power() const { return dynamic_power_; }

  /**
   * @brief Setup the mapper. This assigns random gates, and then sets up AIG
   */
  void Initialize();

  /**
   * @brief Writes the mapping (in the weird verilog output form) to a file.
   * Note: In weird verilog, the output port is the last parameter instead of
   * the first parameter.
   *
   * @param file destination path
   */
  void WriteMapping(const std::filesystem::path &file) const;

  /**
   * @brief Writes the mapping in verilog for ABC to read -- gate names omitted.
   *
   * @param file destination path
   */
  void WriteVerilogABC(const std::filesystem::path &file) const;

  /**
   * @brief Picks a random primitive from the list and adds it.
   */
  void AddRandomGate();

  /**
   * @brief Undoes the most recently added gate. Used for rollbacks in SA.
   */
  void UndoGateAdd();

  /**
   * @brief Updates an AIG node's gate. If it is already active, stats will
   * update as if removing then adding in a new gate.
   *
   * @param aig_variable
   * @param new_cell
   */
  void ChangeAIGNodeGate(int aig_variable, const Cell *new_cell);

  /**
   * @brief Add the associated gate mapping. You can access it via
   * `gates_[gate_id].mapping`. This also acts as an update-query for various
   * statistics (power, area, etc.).
   *
   * @param mapping
   * @return int gate_id of the newly added gate, -1 if failed to add
   */
  int AddBinaryGate(const GateMapping *mapping);

  /**
   * @brief Query to remove the binary gate at gates_[gate_id] from cost.
   *
   * @param gate_id
   */
  void RemoveBinaryGate(int gate_id);

  const auto &aig_nodes() const { return aig_nodes_; }  // see aig_nodes_
  const auto &aig_gates() const { return aig_gates_; }  // see aig_gates_
  const auto &library() const { return library_; }

 private:
  struct AIGAuxiliary {          // holds extra info about AIG nodes
    const Cell *cell = nullptr;  // default cell -- nullptr for input nodes
    bool active = false;         // whether its currently summed into cost
    int covered_by = -1;         // index of the Gate this AIGNode is covered by
    int deps = 0;  // how many nodes depend on this (outdegree) after mapping
  };

  struct GateAuxiliary {                   // holds extra info about AIG gates
    const GateMapping *mapping = nullptr;  // the associated GateMapping
  };

  /**
   * @brief Analyzes the AIG for primitive gate locations.
   */
  void FindPrimitives();

  int AddUnaryGate(const GateMapping &mapping);

  void RemoveUnaryGate(int gate_id);

  /**
   * @brief Query to remove the base gate of the AIG node from the cost.
   *
   * @param variable
   */
  void CoverAIG(int variable);

  /**
   * @brief Query to add the base gate of the AIG node to the cost.
   *
   * @param variable which AIG node to uncover
   */
  void UncoverAIG(int variable);

  /**
   * Removes outdegree from the AIG node.
   * Something no longer depends on `variable`.
   *
   * This is called when you map
   * a gate onto the AIG, while the output gets replaced entirely, the inputs
   * may still be read by other nets, so decrement the input net by one. When
   * the outdegree reaches zero, then the node can be pruned.
   */
  void RemoveDependency(int variable);

  /**
   * @brief Add outdegree to AIG node. Something new now depends on `variable`.
   *
   * @param variable
   */
  void AddDependency(int variable);

  double area_ = 0;           // area of the current mapping
  double power_ = 0;          // power of current mapping
  double dynamic_power_ = 0;  // dynamic power of current mapping
  Library library_;

  std::vector<int> added_gates_;          // history of added gates (stack)
  std::vector<GateMapping> candidates_;   // candidate gates for tech map
  std::vector<AIGAuxiliary> aig_nodes_;   // extra data for AIG nodes
  std::vector<GateAuxiliary> aig_gates_;  // extra data for AIG gates
};

#endif  // SRC_ITERATIVE_TECHNOLOGY_MAPPER_