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
  void WriteMapping(const std::filesystem::path& file) const;

  /**
   * @brief Picks a random primitive from the list and adds it.
   */
  void AddRandomGate();

  /**
   * @brief Undoes the most recently added gate. Used for rollbacks in SA.
   */
  void UndoGateAdd();

 private:
  struct GateMapping {
    GateMapping(int a, int b, int y, Cell::Type type,
                const std::vector<int> &covers)
        : a(a), b(b), y(y), type(type), covers(covers) {}
    int a = -1;               // input A
    int b = -1;               // input B (not used in unary gates)
    int y = -1;               // output Y
    Cell::Type type;          // gate type
    std::vector<int> covers;  // what literals this replaces
  };

  struct AIGAuxiliary {          // holds extra info about AIG nodes
    const Cell *cell = nullptr;  // default cell
    bool active = false;         // whether its currently summed into cost
    int covered_by = -1;         // index of the Gate this AIGNode is covered by
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
   * @brief Query at add the binary gate at gates_[gate_id] from cost.
   *
   * @param mapping
   * @return int gate_id of the newly added gate, -1 if failed to add
   */
  int AddBinaryGate(const GateMapping &mapping);

  /**
   * @brief Query to remove the binary gate at gates_[gate_id] from cost.
   *
   * @param gate_id
   */
  void RemoveBinaryGate(int gate_id);

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

  double area_;           // area of the current mapping
  double power_;          // power of current mapping
  double dynamic_power_;  // dynamic power of current mapping
  Library library_;

  std::vector<int> added_gates_;          // history of added gates (stack)
  std::vector<GateMapping> candidates_;   // candidate gates for tech map
  std::vector<AIGAuxiliary> aig_nodes_;   // extra data for AIG nodes
  std::vector<GateAuxiliary> aig_gates_;  // extra data for AIG gates
};

#endif  // SRC_ITERATIVE_TECHNOLOGY_MAPPER_