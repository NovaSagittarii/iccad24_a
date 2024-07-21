#include "aig_reader.hh"

void AIGReader::on_header(u64 m, u64 i, u64 l, u64 o, u64 a) const {
  // printf("[h] tot=%i i=%i l=%i o=%i a=%i\n", m, i, l, o, a);
  aig_.LoadHeader(m, i, o, a);
}

void AIGReader::on_input(u32 index, u32 lit) const {
  // printf("[p] inp %i %i\n", index, lit);
  // Note: literals are 1-indexed, but 0-indexed is easier to work with
  lit -= 2;
  aig_.SetInputPort(index, lit);
}

void AIGReader::on_output(u32 index, u32 lit) const {
  // printf("[p] out %i %i\n", index, lit);
  lit -= 2;
  aig_.SetOutputPort(index, lit);
}

void AIGReader::on_and(u32 index, u32 left_lit, u32 right_lit) const {
  --index;
  left_lit -= 2;
  right_lit -= 2;
  // printf("[p] and %i(#%i) <= %i & %i\n", index, index * 2, left_lit,
  // right_lit);
  aig_.SetNodeDependencies(index*2, left_lit, right_lit);
}

void AIGReader::on_input_name(u32 pos, const std::string& name) const {
  aig_.SetInputNetName(pos, name);
}

void AIGReader::on_output_name(u32 pos, const std::string& name) const {
  aig_.SetOutputNetName(pos, name);
}

void AIGReader::on_comment(const std::string& comment) const {
  aig_.SetModuleName(comment);
}
