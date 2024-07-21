#ifndef SRC_AIG_READER_HH_
#define SRC_AIG_READER_HH_

#include <lorina/aiger.hpp>

#include "aig.hh"

/**
 * Class for reading AIG files, has callbacks needed for lorina::aiger_reader
 *
 * Usage example
```cpp
AIG aig;
AIGReader reader(aig);
auto result = lorina::read_aiger(filename, reader);
if (result == lorina::return_code::parse_error) {
  std::cerr << "[e] parsing failed" << std::endl;
} else {
  std::cerr << "[i] parsing successful" << std::endl;
}
```
 */
class AIGReader : public lorina::aiger_reader {
 public:
  typedef uint32_t u32;
  typedef uint64_t u64;
  AIGReader(AIG& aig) : aig_(aig) {}
  void on_header(u64 m, u64 i, u64 l, u64 o, u64 a) const override;
  void on_input(u32 index, u32 lit) const override;
  void on_output(u32 index, u32 lit) const override;
  void on_and(u32 index, u32 left_lit, u32 right_lit) const override;

  void on_input_name(u32 pos, const std::string& name) const override;
  void on_output_name(u32 pos, const std::string& name) const override;
  void on_comment(const std::string& comment) const override;

 private:
  AIG& aig_;
};

#endif  // SRC_AIG_READER_HH_