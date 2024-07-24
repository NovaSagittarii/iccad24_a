#include "simple_verilog_driver.hh"

enum PARSER_STAGE {
  // not reading anything
  IDLE,

  // reading module name
  MODULE,

  // reading input ports
  PORT_INPUT,

  // reading output ports
  PORT_OUTPUT,

  // reading wires -- don't really need to do anything with this
  PORT_WIRE,

  // reading gates
  GATES,
};

// Note: this simplified parser does not read pin labels!!
// It breaks if they appear.
void verilog::ParserVerilogInterface::read(const std::filesystem::path& path) {
  PARSER_STAGE stage = PARSER_STAGE::IDLE;
  verilog::Port input_port, output_port;

  input_port.dir = verilog::PortDirection::INPUT;
  output_port.dir = verilog::PortDirection::OUTPUT;

  std::ifstream fin(path);
  std::string line;
  while (std::getline(fin, line)) {
    std::string word;
    std::istringstream in(line);
    in >> word;
    if (word == "module") {
      stage = PARSER_STAGE::MODULE;
    } else if (word == "input") {
      stage = PARSER_STAGE::PORT_INPUT;
    } else if (word == "output") {
      stage = PARSER_STAGE::PORT_OUTPUT;
    }

    switch (stage) {
      case PARSER_STAGE::MODULE:
        in >> word;
        add_module(std::string(word));
        stage = PARSER_STAGE::IDLE;
        break;
      case PARSER_STAGE::PORT_INPUT:
      case PARSER_STAGE::PORT_OUTPUT:
      case PARSER_STAGE::PORT_WIRE:
        in = std::istringstream(line);  // reset the line
        while (in >> word) {
          bool final_item = word.back() == ';';
          if (word.back() == ',' || word.back() == ';') word.pop_back();
          switch (stage) {
            case PARSER_STAGE::PORT_INPUT:
              if (word == "input") continue;
              input_port.names.push_back(word);
              if (final_item) {
                stage = PARSER_STAGE::IDLE;
                add_port(std::move(input_port));
              }
              break;
            case PARSER_STAGE::PORT_OUTPUT:
              if (word == "output") continue;
              output_port.names.push_back(word);
              if (final_item) {
                stage = PARSER_STAGE::PORT_WIRE;
                add_port(std::move(output_port));
              }
              break;
            case PARSER_STAGE::PORT_WIRE:
              if (final_item) {
                stage = PARSER_STAGE::GATES;
              }
              break;
          }
        }
        break;
      case PARSER_STAGE::GATES:
        // filter out empty lines
        while (!line.empty() && line.back() != ';') line.pop_back();
        if (!line.empty()) {
          line.pop_back();  // remove the semicolon

          // there's a gate
          in = std::istringstream(line);
          verilog::Instance inst;
          in >> inst.module_name;
          char c;
          while (in >> c && c != '(') {
            inst.inst_name.push_back(c);
          }
          for (int i = 0; i < 3 && in >> word; ++i) {
            if (word.back() == ',' || word.back() == ')') word.pop_back();
            if (word.empty()) {
              // dealing with too much whitespace " , "
              --i;
              continue;
            }
            std::vector<verilog::NetConcat> net;
            net.push_back(word);
            inst.net_names.push_back(net);
          }
          add_instance(std::move(inst));
        }
        break;
    }
  }
}
