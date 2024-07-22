#include "library.hh"

#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"

void Library::Load(const std::filesystem::path &file) {
  std::ifstream f(file);
  nlohmann::json data = nlohmann::json::parse(f);
  n_ = std::stoi((std::string)data["information"]["cell_num"]);
  m_ = std::stoi((std::string)data["information"]["attribute_num"]);
  attributes_ = (std::vector<std::string>)data["information"]["attributes"];
  for (auto &cell_data : data["cells"]) {
    Cell c;
    for (auto &x : cell_data.items()) {
      c.LoadProperty(x.key(), x.value());
    }
    cells_[c.name()] = c;
  }
  std::cout << "[load] lib n=" << n_ << " m=" << m_ << std::endl;
}

const std::vector<const Cell*> Library::GetCellsByType(Cell::Type type) const {
  std::vector<const Cell*> out;
  for (const auto &[cell_name, cell] : cells_) {
    if (cell.type() == type) {
      out.push_back(&cell);
    }
  }
  return std::move(out);
}
