#ifndef ICCAD_SRC_LIBRARY_H_
#define ICCAD_SRC_LIBRARY_H_

#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief Represents a cell library. Supports load and lookup.
 */
class Library {
 public:
  Library() {}
  void Load(const std::filesystem::path &file);

 private:
  /// @brief how many cells there are
  int n_;

  /// @brief how many attributes per cell there are
  int m_;

  /// @brief name of each attribute
  std::vector<std::string> attributes_;
};

#endif  // ICCAD_SRC_LIBRARY_H_