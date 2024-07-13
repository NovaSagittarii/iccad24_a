#include <sstream>
#include <string>

/**
 * Applies the -1234567 transformation to a string of
 * underscore-delimited (_), skipping the first `skip` items.
 *
 * @param s input string
 * @param skip number of entries to skip, typically 1
 *        for the module name
 */
std::string decode(std::string s, int skip) {
  std::istringstream in(s);
  int32_t mem[0x20] = {};
  std::string segment;

  int i = 0;
  while (std::getline(in, segment, '_')) {
    if (skip) {
      --skip;
    } else {
      mem[i++] = std::stoi(segment);
    }
  }

  std::string out = *(char **)mem;
  return out;
}
