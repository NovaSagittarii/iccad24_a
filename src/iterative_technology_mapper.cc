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
  for (int i = 0; i < sz_i_; ++i) {
    if (i) fout << ", ";
    fout << net_names_[inputs_[i]];
  }
  for (auto i : outputs_) fout << ", " << net_names_[i];
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
      if (i & 1) {  // NOT gate (from AIG)
        fout << net_names_[i ^ 1] << " , ";
      } else {  // AND gate (from AIG)
        fout << net_names_[node.inputs[0]] << " , ";
        fout << net_names_[node.inputs[1]] << " , ";
      }
      fout << net_names_[i] << " ) ;";
      fout << "\n";
    }
  }
  for (int i = 0; i < sz_v_ * 2; ++i) {
    const auto& gate = gates_[i];
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

void IterativeTechnologyMapper::WriteVerilogABC(
    const std::filesystem::path& file) const {
  std::ofstream fout(file);
  std::string module = top_module_name_;
  fout << "module " << module.c_str() << "\n";

  std::vector<int> used(sz_v_ * 2);

  /// TODO: put methods in ostream friend?
  fout << "(";
  for (int i = 0; i < sz_i_; ++i) {
    if (i) fout << ", ";
    fout << net_names_[inputs_[i]];
  }
  for (auto i : outputs_) fout << ", " << net_names_[i];
  fout << ");\n";

  fout << "\tinput ";
  for (int i = 0; i < sz_i_; ++i) {
    if (i) fout << ", ";
    fout << net_names_[inputs_[i]];
    used[inputs_[i]] = 1;
  }
  fout << ";\n";

  fout << "\toutput ";
  for (int i = 0; i < sz_o_; ++i) {
    if (i) fout << ", ";
    fout << net_names_[outputs_[i]];
    used[outputs_[i]] = 1;
  }
  fout << ";\n";

  fout << "\twire ";
  bool first = false;
  for (int i = 0; i < sz_v_ * 2; ++i) {
    if (used[i]) continue;
    if (first) fout << ", ";
    first = true;
    fout << net_names_[i];
  }
  fout << ";\n";

  int gate_id = 0;
  for (int i = 0; i < sz_v_ * 2; ++i) {
    const auto& node = nodes_[i];
    const auto& aig_node = aig_nodes_[i];
    if (aig_node.active) {
      std::string name = aig_node.cell->name();
      while (name.back() != '_') name.pop_back();
      name.pop_back();
      fout << "\t" << name << " ( ";
      fout << net_names_[i] << " , ";
      if (i & 1) {  // NOT gate (from AIG)
        fout << net_names_[i ^ 1];
      } else {  // AND gate (from AIG)
        fout << net_names_[node.inputs[0]] << " , ";
        fout << net_names_[node.inputs[1]];
      }
      fout << " ) ;";
      fout << "\n";
    }
  }

  fout << "\n";

  for (int i = 0; i < sz_v_ * 2; ++i) {
    const auto& gate = gates_[i];
    if (gate.active) {
      std::string name = gate.cell->name();
      while (name.back() != '_') name.pop_back();
      name.pop_back();
      fout << "\t" << name << " ( ";
      fout << net_names_[gate.y] << " , ";
      fout << net_names_[gate.a] << " , ";
      fout << net_names_[gate.b] << " );";
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

  // handle I/O dependencies
  for (int i : inputs_) AddDependency(i);
  for (int i : outputs_) AddDependency(i);

  // handle INV dependencies
  for (int i = 0; i < sz_v_; ++i) {
    // (i*2) ---NOT---> (i*2+1)
    AddDependency(i * 2);
  }

  // handle AND gate dependencies
  for (int i = sz_i_; i < sz_v_; ++i) {
    // (a) + (b) ---AND---> (i*2)
    for (auto inp : nodes_[i * 2].inputs) AddDependency(inp);
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
    const int z = i * 2;
    const int znot = i * 2 + 1;

    bool set_read = AND.out_degree;   // does the AND get read?
    bool inv_read = NAND.out_degree;  // does the NAND get read?
    and_ct += set_read;
    nand_ct += inv_read;
    if (inv_read) {
      candidates_.push_back(GateMapping(x, y, znot, Cell::Type::kNand));
    }

    if (x & y & 1) {
      x ^= 1;
      y ^= 1;

      /**
       * nor takes the for
       *
       * NOR = !x !y
       */
      // if both are inverted inputs
      ++nor_ct;
      candidates_.push_back(GateMapping(x, y, z, Cell::Type::kNor));

      if (inv_read) {
        ++or_ct;
        candidates_.push_back(GateMapping(x, y, znot, Cell::Type::kOr));
      }

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
          continue;
          candidates_.push_back(GateMapping(x1, y1, z, Cell::Type::kXnor));
          candidates_.push_back(GateMapping(x2, y2, z, Cell::Type::kXnor));

          if (inv_read) {
            ++xor_ct;
            candidates_.push_back(GateMapping(x1, y1, znot, Cell::Type::kXor));
            candidates_.push_back(GateMapping(x2, y2, znot, Cell::Type::kXor));
          }
        }
      }
    }
  }

  // printf("and=%i nand=%i or=%i nor=%i xor=%i xnor=%i\n", and_ct, nand_ct,
  // or_ct, nor_ct, xor_ct, xnor_ct);

  /*
  // print all candidates
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
  */
}

void IterativeTechnologyMapper::AddRandomGate() {
  const auto& g = choice(candidates_);
  int gate_id = AddBinaryGate(g);
  if (gate_id != -1) {
    added_gates_.push_back(gate_id);
  }
}

void IterativeTechnologyMapper::UndoGateAdd() {
  if (!added_gates_.empty()) {
    RemoveBinaryGate(added_gates_.back());
    added_gates_.pop_back();
  }
}

int IterativeTechnologyMapper::AddUnaryGate(const GateMapping& gate) {
  return -1;
}

void IterativeTechnologyMapper::RemoveUnaryGate(int gate_id) {}

int IterativeTechnologyMapper::AddBinaryGate(const GateMapping& mapping) {
  // ensure there is no output overlap first
  // remember, **one** driver per net!
  // its a waste for multiple drivers anyways
  if (aig_nodes_[mapping.y].covered_by != -1) return -1;

  // this checks for intermediate overlapping
  // for (auto i : mapping.covers) {
  //   if (nodes_[i].is_io) return -1;
  //   if (aig_nodes_[i].covered_by != -1) return -1;
  // }

  // nevermind, i assume SA will figure overlap out

  // find allocation spot
  static int gate_id = 0;
  while (gates_[gate_id].active) {
    ++gate_id;
    gate_id %= gates_.size();
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

  // update dependencies
  auto& aig_node = aig_nodes_[gate.y];
  aig_node.covered_by = gate_id;

  // you need to add new dependencies of the added gate
  AddDependency(gate.a);
  AddDependency(gate.b);

  // you don't need the output node anymore
  // since the new gate fills the role of net driver
  CoverAIG(gate.y);

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
  gate.active = true;

  // update dependencies
  // uncover the AIG node since the gate is removed
  // something NEEDS to fill the role of net driver
  aig_nodes_[gate.y].covered_by = -1;
  UncoverAIG(gate.y);

  // and since you removed it... the inputs are no longer there
  RemoveDependency(gate.a);
  RemoveDependency(gate.b);

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
  // printf("cover (-) %i\n", variable);

  auto& aig_node = aig_nodes_[variable];
  if (!aig_node.active) return;
  aig_node.active = false;

  double leak = aig_node.cell->leakage_power();
  area_ += aig_node.cell->area();
  power_ += leak;
  dynamic_power_ += leak * nodes_[variable].q;

  // since you are covering the AIG node -- presumbly because either its one
  // dependency has been removed, or that the output gate is being
  // reimplemented by another gate -- in either case, it's old dependencies
  // are no longer needed
  if (variable & 1) {  // INV node
    RemoveDependency(variable ^ 1);
  } else {  // AND node
    auto [x, y] = nodes_[variable].inputs;
    RemoveDependency(x);
    RemoveDependency(y);
  }
}

void IterativeTechnologyMapper::UncoverAIG(int variable) {
  // printf("uncover (+) %i\n", variable);

  auto& aig_node = aig_nodes_[variable];
  if (aig_node.covered_by != -1) return;  // its output is already covered
  if (aig_node.active) return;
  aig_node.active = true;

  double leak = aig_node.cell->leakage_power();
  area_ -= aig_node.cell->area();
  power_ -= leak;
  dynamic_power_ -= leak * nodes_[variable].q;

  // since you're uncovering the AIG node to use the default gate,
  // you have additional dependencies now
  if (variable & 1) {  // INV node
    AddDependency(variable ^ 1);
  } else {  // AND node
    auto [x, y] = nodes_[variable].inputs;
    AddDependency(x);
    AddDependency(y);
  }
}

void IterativeTechnologyMapper::AddDependency(int variable) {
  // printf("++dep %i\n", variable);
  auto& aig_node = aig_nodes_[variable];

  // if adding the first dependency, you'll need to uncover it
  bool first_dependency = ++aig_node.deps == 1;

  // make sure its not an input port
  bool input_port = (variable & 1) == 0 && variable / 2 < sz_i_;

  if (first_dependency && !input_port) UncoverAIG(variable);
}

void IterativeTechnologyMapper::RemoveDependency(int variable) {
  // printf("--dep %i\n", variable);
  auto& aig_node = aig_nodes_[variable];

  // if removing the last dependency, you don't need it anymore
  bool last_dependency = --aig_node.deps == 0;
  if (last_dependency) CoverAIG(variable);
}

int32_t main() {
  IterativeTechnologyMapper it;
  it.Load("design6.aig");
  it.LoadLibrary("lib1.json");
  it.Initialize();

  it.WriteMapping("a1.txt");
  it.WriteVerilogABC("a1.v");
  std::srand(0);
  for (int i = 0; i < 100; ++i) it.AddRandomGate();
  it.WriteMapping("a2.txt");
  it.WriteVerilogABC("a2.v");
}