#include "iterative_technology_mapper.hh"

#include <fstream>
#include <random>

/**
 * randomly pick one from an array
 */
template <class T>
const T choice(const std::vector<T>& arr) {
  return arr.at(std::rand() % arr.size());
}

void IterativeTechnologyMapper::WriteMapping(
    const std::filesystem::path& file) const {
  std::ofstream fout(file);
  std::string module = top_module_name_;
  fout << "module " << module.c_str() << "\n";

  /// TODO: put methods in ostream friend?
  fout << "(";
  for (int i = 0; i < net_names_.size(); ++i) {
    if (i) fout << ", ";
    fout << net_names_[i];
  }
  fout << ");\n";

  fout << "\tinput ";
  for (int i = 0; i < sz_i_; ++i) {
    if (i) fout << ", ";
    fout << net_names_[inputs_[i]];
  }
  fout << ";\n";

  fout << "\toutput ";
  for (int i = 0; i < sz_o_; ++i) {
    if (i) fout << ", ";
    fout << net_names_[outputs_[i]];
  }
  fout << ";\n";

  int gate_id = 0;
  for (int i = 0; i < sz_v_ * 2; ++i) {
    const auto& node = nodes_[i];
    const auto& aig_node = aig_nodes_[i];
    if (aig_node.active) {
      fout << "\t" << aig_node.cell->name() << " g" << (gate_id++) << " ( ";
      if (i & 1) { // NOT gate (from AIG)
        fout << net_names_[i ^ 1] << " , ";
      } else { // AND gate (from AIG)
        fout << net_names_[node.inputs[0]] << " , ";
        fout << net_names_[node.inputs[1]] << " , ";
      }
      fout << net_names_[i] << " ) ;";
      fout << "\n";
    }
  }
  for (int i = 0; i < sz_v_ * 2; ++i) {
    const auto &gate = gates_[i];
    if (gate.active) {
      fout << "\t" << gate.cell->name() << " h" << (gate_id++) << " ( ";
      fout << net_names_[gate.a] << " , ";
      fout << net_names_[gate.b] << " , ";
      fout << net_names_[gate.y] << " ) ;";
      fout << "\n";
    }
  }

  fout << "endmodule\n";
  fout.close();
}

void IterativeTechnologyMapper::LoadLibrary(const std::filesystem::path& file) {
  library_.Load(file);
}

void IterativeTechnologyMapper::Initialize() {
  FindPrimitives();

  aig_nodes_.assign(sz_v_ * 2, AIGAuxiliary());
  aig_gates_.assign(sz_v_ * 2, GateAuxiliary());

  // set default cells
  const auto cell_a = library_.GetCellsByType(Cell::Type::kAnd);
  const auto cell_i = library_.GetCellsByType(Cell::Type::kNot);
  for (int i = sz_i_; i < sz_v_; ++i) aig_nodes_[i * 2].cell = choice(cell_a);
  for (int i = 0; i < sz_v_; ++i) aig_nodes_[i * 2 + 1].cell = choice(cell_i);

  for (int i = 0; i < sz_v_ * 2; ++i) {
    if (nodes_[i].out_degree && aig_nodes_[i].cell != nullptr) {
      UncoverAIG(i);
    }
  }
}

void IterativeTechnologyMapper::FindPrimitives() {
  int and_ct = 0, nand_ct = 0;
  int nor_ct = 0, or_ct = 0;
  int xor_ct = 0, xnor_ct = 0;
  for (int i = sz_i_; i < sz_v_; ++i) {
    const auto& AND = nodes_[i * 2];
    const auto& NAND = nodes_[i * 2 + 1];
    auto [x, y] = AND.inputs;
    bool set_read = AND.out_degree;   // does the AND get read?
    bool inv_read = NAND.out_degree;  // does the NAND get read?
    and_ct += set_read;
    nand_ct += inv_read;
    if (x & y & 1) {
      /**
       * nor takes the for
       *
       * NOR = !x !y
       */
      // if both are inverted inputs
      ++nor_ct;
      candidates_.push_back(
          GateMapping(x, y, i * 2, Cell::Type::kNor, {x, y, i * 2}));

      if (inv_read) {
        ++or_ct;
        candidates_.push_back(GateMapping(x, y, i * 2 + 1, Cell::Type::kOr,
                                          {x, y, i * 2, i * 2 + 1}));
      }

      x ^= (x & 1);
      y ^= (y & 1);
      if (nodes_[x].inputs[0] != -1 && nodes_[y].inputs[0] != -1) {
        // the two inverted inputs to AND are also output of AND
        auto [x1, y1] = nodes_[x].inputs;
        auto [x2, y2] = nodes_[y].inputs;
        if (x1 > y1) std::swap(x1, y1);
        if (x2 > y2) std::swap(x2, y2);
        if (x1 ^ x2 == 1 && y1 ^ y2 == 1 && x1 / 2 == x2 / 2 &&
            y1 / 2 == y2 / 2) {
          x1 ^= (x1 & 1);
          y1 ^= (y1 & 1);
          x2 = x1 | 1;
          y2 = y1 | 1;

          ++xnor_ct;
          std::vector<int> xnor_cover{x1, y1,    x2,    y2,   x,
                                      y,  x | 1, y | 1, i * 2};
          std::vector<int> xor_cover{x1, y1,    x2,    y2,    x,
                                     y,  x | 1, y | 1, i * 2, i * 2 + 1};
          candidates_.push_back(
              GateMapping(x1, y1, i * 2, Cell::Type::kXnor, xnor_cover));
          candidates_.push_back(
              GateMapping(x2, y2, i * 2, Cell::Type::kXnor, xnor_cover));

          if (inv_read) {
            ++xor_ct;
            candidates_.push_back(
                GateMapping(x1, y1, i * 2, Cell::Type::kXor, xor_cover));
            candidates_.push_back(
                GateMapping(x2, y2, i * 2, Cell::Type::kXor, xor_cover));
          }
        }
      }
    }
  }

  // printf("and=%i nand=%i or=%i nor=%i xor=%i xnor=%i\n", and_ct, nand_ct,
  // or_ct, nor_ct, xor_ct, xnor_ct);

  for (auto g : candidates_) {
    std::string type;
    switch (g.type) {
      case Cell::Type::kOr:
        type = "or";
        break;
      case Cell::Type::kNor:
        type = "nor";
        break;
      case Cell::Type::kXor:
        type = "xor";
        break;
      case Cell::Type::kXnor:
        type = "xnor";
        break;
    }
    printf("* %i %s %i => %i\n", g.a, type.c_str(), g.b, g.y);
  }
}

void IterativeTechnologyMapper::AddRandomGate() {
  const auto& g = choice(candidates_);
  AddBinaryGate(g);
}

void IterativeTechnologyMapper::UndoGateAdd() {}

int IterativeTechnologyMapper::AddUnaryGate(const GateMapping& gate) {
  return -1;
}

void IterativeTechnologyMapper::RemoveUnaryGate(int gate_id) {}

int IterativeTechnologyMapper::AddBinaryGate(const GateMapping& mapping) {
  // ensure there is no overlap first
  for (auto i : mapping.covers) {
    if (aig_nodes_[i].covered_by != -1) return -1;
  }

  // find allocation spot
  static int gate_id = 0;
  while (gates_[gate_id].active) {
    ++gate_id;
    gate_id %= gates_.size();
  }

  // remove overlapping aig node
  for (auto i : mapping.covers) {
    aig_nodes_[i].covered_by = gate_id;
    CoverAIG(i);
  }

  // pick a random cell
  const auto cell = choice(library_.GetCellsByType(mapping.type));

  // allocate
  auto& gate = gates_[gate_id];
  gate.active = true;
  gate.cell = cell;
  gate.a = mapping.a;
  gate.b = mapping.b;
  gate.y = mapping.y;
  aig_gates_[gate_id].mapping = &mapping;

  // update stats
  double leak = cell->leakage_power();
  area_ += cell->area();
  power_ += leak;
  dynamic_power_ += leak * nodes_[mapping.y].q;

  return gate_id;
}

void IterativeTechnologyMapper::RemoveBinaryGate(int gate_id) {
  auto& gate = gates_[gate_id];
  auto& aig_gate = aig_gates_[gate_id];
  if (!gate.active) return;

  // uncover the AIG nodes since the gate is removed
  for (const auto i : aig_gate.mapping->covers) {
    aig_nodes_[i].covered_by = -1;
    UncoverAIG(i);
  }

  // update stats
  const auto cell = gate.cell;
  double leak = cell->leakage_power();
  area_ -= cell->area();
  power_ -= leak;
  dynamic_power_ -= leak * nodes_[aig_gate.mapping->y].q;

  // deallocate
  gate.active = false;
  aig_gate.mapping = nullptr;
}

void IterativeTechnologyMapper::CoverAIG(int variable) {
  auto& aig_node = aig_nodes_[variable];
  if (!aig_node.active) return;
  aig_node.active = false;

  double leak = aig_node.cell->leakage_power();
  area_ += aig_node.cell->area();
  power_ += leak;
  dynamic_power_ += leak * nodes_[variable].q;
}

void IterativeTechnologyMapper::UncoverAIG(int variable) {
  auto& aig_node = aig_nodes_[variable];
  if (aig_node.active) return;
  aig_node.active = true;

  double leak = aig_node.cell->leakage_power();
  area_ -= aig_node.cell->area();
  power_ -= leak;
  dynamic_power_ -= leak * nodes_[variable].q;
}

int32_t main() {
  IterativeTechnologyMapper it;
  it.Load("design1.aig");
  it.LoadLibrary("lib1.json");
  it.Initialize();
  it.WriteMapping("a1.txt");
  it.AddRandomGate();
  it.WriteMapping("a2.txt");
}