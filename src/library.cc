#include "library.hh"

#include <fstream>
#include <iostream>

#include "cell.hh"
#include "nlohmann/json.hpp"

void Library::Load(const std::filesystem::path &file) {
  std::ifstream f(file);
  nlohmann::json data = nlohmann::json::parse(f);
  n_ = std::stoi((std::string)data["information"]["cell_num"]);
  m_ = std::stoi((std::string)data["information"]["attribute_num"]);
  attributes_ = data["information"]["attributes"];
  for (auto &cell_data : data["cells"]) {
    Cell c;
    for (auto &x : cell_data.items()) {
      c.LoadProperty(x.key(), x.value());
    }
  }
}
