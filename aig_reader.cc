#include <stdio.h>

#include <iostream>
#include <lorina/aiger.hpp>
#include <vector>

/*
compile:
g++ -std=c++20 -I external/lorina/include/ -I external/lorina/lib/fmt \
aig_reader.cc

run:
./a.out design0.aig
*/

using namespace lorina;

typedef uint32_t u32;
typedef uint64_t u64;

struct G {
  int n;                                // number of gates
  int i;                                // number of inputs
  int o;                                // number of outputs
  int a;                                // number of and gates
  std::vector<int> inputs;              // input vars
  std::vector<int> outputs;             // output vars
  std::vector<std::array<int, 2>> dep;  // and_dependencies (-1,-1) means input
};

class custom_aiger_reader : public aiger_reader {
 public:
  G& g_;
  custom_aiger_reader(G& g) : g_(g) {}
  void on_header(u64 m, u64 i, u64 l, u64 o, u64 a) const override {
    printf("[h] tot=%i i=%i l=%i o=%i a=%i\n", m, i, l, o, a);
    g_.n = m;
    g_.i = i;
    g_.o = o;
    g_.a = a;
    g_.inputs.assign(i, 0);
    g_.outputs.assign(o, 0);
    g_.dep.assign(m, {-1, -1});
  }
  void on_input(u32 index, u32 lit) const override {
    printf("[p] inp %i %i\n", index, lit);
    lit -= 2;
    g_.inputs[index] = lit;
  }
  void on_output(u32 index, u32 lit) const override {
    printf("[p] out %i %i\n", index, lit);
    lit -= 2;
    g_.outputs[index] = lit;
  }
  void on_and(u32 index, u32 left_lit, u32 right_lit) const override {
    --index;
    left_lit -= 2;
    right_lit -= 2;
    printf("[p] and %i(#%i) <= %i & %i\n", index, index * 2, left_lit,
           right_lit);
    if (left_lit > right_lit) std::swap(left_lit, right_lit);
    g_.dep[index] = {(int)left_lit, (int)right_lit};
  }
  void on_input_name(uint32_t pos, const std::string& name) const override {
    printf("[p] i_name %i %s\n", pos, name.c_str());
  }
  void on_output_name(uint32_t pos, const std::string& name) const override {
    printf("[p] o_name %i %s\n", pos, name.c_str());
  }
  void on_comment(const std::string& comment) const override {
    printf("[c] %s\n", comment.c_str());
  }
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <aiger_file>" << std::endl;
    return 1;
  }

  const std::string filename = argv[1];

  G g;
  custom_aiger_reader reader(g);
  auto result = read_aiger(filename, reader);

  if (result == return_code::parse_error) {
    std::cerr << "[e] parsing failed" << std::endl;
    return EXIT_FAILURE;
  } else {
    std::cerr << "[i] parsing successful" << std::endl;
  }

  assert(g.n >= 0);
  std::vector<int> reads(g.n * 2);
  for (auto x : g.outputs) ++reads[x];
  for (auto [x, y] : g.dep) {
    if (x != -1) ++reads[x];
    if (y != -1) ++reads[y];
  }

  int and_ct = 0, nand_ct = 0;
  int nor_ct = 0, or_ct = 0;
  int xor_ct = 0, xnor_ct = 0;
  for (int i = g.i; i < g.n; ++i) {
    auto [x, y] = g.dep[i];
    const bool set_read = reads[i * 2];      // does the AND get read?
    const bool inv_read = reads[i * 2 + 1];  // does the NAND get read?
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
      printf("n(or) -- %i %i => %i\n", x / 2, y / 2, i);
      // does the inverted form on the and happen? (then or works)
      if (inv_read) ++or_ct;

      /**
       * xor takes the form
       *
       * XOR = !(!a !b)
       * a = x !y
       * b = y !x
       */
      if (g.dep[x / 2][0] != -1 && g.dep[y / 2][0] != -1) {
        // the two inverted inputs to AND are also output of AND
        auto [x1, y1] = g.dep[x / 2];
        auto [x2, y2] = g.dep[y / 2];
        if (x1 > y1) std::swap(x1, y1);
        if (x2 > y2) std::swap(x2, y2);
        if (x1 ^ x2 == 1 && y1 ^ y2 == 1 && x1 / 2 == x2 / 2 &&
            y1 / 2 == y2 / 2) {
          ++xnor_ct;
          if (inv_read) ++xor_ct;
        }
      }
    }
  }

  for (auto x : reads) std::cout << x << " ";
  std::cout << std::endl;
  printf("and=%i nand=%i\n", and_ct, nand_ct);
  printf("or=%i nor=%i\n", or_ct, nor_ct);
  printf("xor=%i xnor=%i\n", xor_ct, xnor_ct);

  return EXIT_SUCCESS;
}