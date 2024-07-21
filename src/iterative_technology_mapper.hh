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

 private:
  struct GateMapping {
    GateMapping(int a, int b, int y, Cell::Type type)
        : a(a), b(b), y(y), type(type) {}
    int a = -1;       // input A
    int b = -1;       // input B (not used in unary gates)
    int y = -1;       // output Y
    Cell::Type type;  // gate type
  };

  /**
   * @brief Analyzes the AIG for primitive gate locations.
   */
  void FindPrimitives();

  /**
   * @brief
   */
  void AddRandomGate();

  /**
   * @brief Undoes the most recently added gate. Used for rollbacks in SA.
   */
  void UndoGateAdd();

  int AddUnaryGate(Cell cell, int y, int a);

  void RemoveUnaryGate(int gate_id);

  int AddBinaryGate(Cell cell, int y, int a, int b);

  void RemoveBinaryGate(int gate_id);

  void CoverAIG(int variable, int cover_gate_id);

  void UncoverAIG(int variable);

  double area_;           // area of the current mapping
  double power_;          // power of current mapping
  double dynamic_power_;  // dynamic power of current mapping
  Library library_;

  std::vector<int> added_gates_;  // history of added gates (stack)
  std::vector<GateMapping> candidates_;
};

#endif  // SRC_ITERATIVE_TECHNOLOGY_MAPPER_