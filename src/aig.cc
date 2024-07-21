#include "aig.hh"

#define FMT_HEADER_ONLY // make sure FMT only gets compiled once
#include "aig_reader.hh"

void AIG::Load(const std::filesystem::path& file) {
  AIGReader reader((AIG&)*this);
  auto result = lorina::read_aiger(file, reader);
  ComputeSetProbability();
  ComputeOutDegree();
}

void AIG::Write(const std::filesystem::path& file) const {}

void AIG::LoadHeader(int v, int i, int o, int a) {
  sz_v_ = v;
  sz_i_ = i;
  sz_o_ = o;
  sz_a_ = a;
  nodes_.assign(v * 2, Node());
  gates_.assign(v * 2, Gate());
  inputs_.assign(sz_i_, -1);
  outputs_.assign(sz_o_, -1);
  net_names_.clear();
}

void AIG::ComputeSetProbability() {
  for (int u : outputs_) {
    ComputeSetProbabilityRecursive(u);
  }
}

void AIG::ComputeSetProbabilityRecursive(int u) {
  if (u & 1 == 1) {  // is an Inverter
    ComputeSetProbabilityRecursive(u ^ 1);
    nodes_[u].set_prob = 1 - nodes_[u ^ 1].set_prob;
  } else if (u < sz_i_ * 2) {  // is an Input port
    nodes_[u].set_prob = 0.5;
  } else {  // is an And gate
    auto [x, y] = nodes_[u].inputs;
    assert(x != -1 && "Dependency x for And gate should exist.");
    assert(y != -1 && "Dependency y for And gate should exist.");
    ComputeSetProbabilityRecursive(x);
    ComputeSetProbabilityRecursive(y);
    nodes_[u].set_prob = nodes_[x].set_prob * nodes_[y].set_prob;
  }
}

void AIG::ComputeOutDegree() {
  for (int i = 0; i < sz_v_; ++i) {
    nodes_[i * 2].out_degree = nodes_[i * 2 + 1].out_degree = 0;
  }

  for (auto x : outputs_) {
    ++nodes_[x].out_degree;
  }

  for (int i = sz_i_; i < sz_v_; ++i) {
    auto& u = nodes_[i * 2];
    auto [a, b] = u.inputs;
    ++nodes_[a].out_degree;
    ++nodes_[b].out_degree;
  }

  // for (int i = 0; i < 2*sz_v_; ++i) {
  //   std::cout << nodes_[i].out_degree << " ";
  //   if (i % 10 == 9) std::cout << "\n";
  // }
  // std::cout << std::endl;
}
