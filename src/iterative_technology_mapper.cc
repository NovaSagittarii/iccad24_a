#include "iterative_technology_mapper.hh"

void IterativeTechnologyMapper::LoadLibrary(const std::filesystem::path& file) {
  library_.Load(file);
}

void IterativeTechnologyMapper::Initialize() { FindPrimitives(); }

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
      candidates_.push_back(GateMapping(x, y, i * 2, Cell::Type::kNor));

      if (inv_read) {
        ++or_ct;
        candidates_.push_back(GateMapping(x, y, i * 2 + 1, Cell::Type::kOr));
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
          candidates_.push_back(GateMapping(x1, y1, i * 2, Cell::Type::kXnor));
          candidates_.push_back(GateMapping(x2, y2, i * 2, Cell::Type::kXnor));

          if (inv_read) {
            ++xor_ct;
            candidates_.push_back(GateMapping(x1, y1, i * 2, Cell::Type::kXor));
            candidates_.push_back(GateMapping(x2, y2, i * 2, Cell::Type::kXor));
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
      case Cell::Type::kOr: type = "or"; break;
      case Cell::Type::kNor: type = "nor"; break;
      case Cell::Type::kXor: type = "xor"; break;
      case Cell::Type::kXnor: type = "xnor"; break;
    }
    printf("* %i %s %i => %i\n", g.a, type.c_str(), g.b, g.y);
  }
}

void IterativeTechnologyMapper::AddRandomGate() {}

void IterativeTechnologyMapper::UndoGateAdd() {}

int IterativeTechnologyMapper::AddUnaryGate(Cell cell, int y, int a) {
  return -1;
}

void IterativeTechnologyMapper::RemoveUnaryGate(int gate_id) {}

int IterativeTechnologyMapper::AddBinaryGate(Cell cell, int y, int a, int b) {
  return -1;
}

void IterativeTechnologyMapper::RemoveBinaryGate(int gate_id) {}

void IterativeTechnologyMapper::CoverAIG(int variable, int cover_gate_id) {}

void IterativeTechnologyMapper::UncoverAIG(int variable) {}

int32_t main() {
  IterativeTechnologyMapper it;
  it.Load("design0.aig");
  it.LoadLibrary("lib1.json");
  it.Initialize();
}