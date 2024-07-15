#ifndef ICCAD_SRC_LIBRARY_H_
#define ICCAD_SRC_LIBRARY_H_

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "cell.hh"

/**
 * @brief Represents a cell library. Supports load and lookup.
 */
class Library {
 public:
  Library() {}

  /// @brief Loads the file into the library. This handles parsing.
  /// @param file
  void Load(const std::filesystem::path& file);

  const auto& cells() const { return cells_; }

 private:
  /// @brief how many cells there are
  int n_;

  /// @brief how many attributes per cell there are
  int m_;

  /// @brief name of each attribute
  std::vector<std::string> attributes_;

  std::map<std::string, Cell> cells_;
};

#endif  // ICCAD_SRC_LIBRARY_H_