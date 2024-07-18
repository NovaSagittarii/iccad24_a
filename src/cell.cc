#include "cell.hh"

void Cell::LoadProperty(const std::string& key, const std::string& value) {
  if (key == "cell_name") {
    name_ = value;
  } else if (key == "cell_type") {
    Cell::Type gate_type = Cell::Type::kUnknown;
    // clang-format off
    if      (value == "not")  gate_type = Cell::Type::kNot;
    else if (value == "buf")  gate_type = Cell::Type::kBuf;
    else if (value == "or")   gate_type = Cell::Type::kOr;
    else if (value == "and")  gate_type = Cell::Type::kAnd;
    else if (value == "nor")  gate_type = Cell::Type::kNor;
    else if (value == "nand") gate_type = Cell::Type::kNand;
    else if (value == "xor")  gate_type = Cell::Type::kXor;
    else if (value == "xnor") gate_type = Cell::Type::kXnor;
    type_ = gate_type;
  } else {
    if (key.back() == 'i') {
      int x = std::stoi(value);
      i_properties_.push_back(x);

      if (key == "data_3_i") {
        pd_ = x;
      }
    } else if (key.back() == 'f') {
      double x = std::stof(value);
      f_properties_.push_back(x);

      if (key == "data_1_f") {
        a_ = x;
      } else if (key == "data_2_f") {
        b_ = x;
      } else if (key == "data_4_f") {
        leakage_power_ = x;
      } else if (key == "data_5_f") {
        c_ = x;
      } else if (key == "data_6_f") {
        area_ = x;
      } else if (key == "data_7_f") {
        max_c_ = x;
      }
    }
  }
}
