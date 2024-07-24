# This file is meant to be executed from the ./build folder.

CC_FLAGS = -Wall -Wextra -pedantic -O2 -Wshadow -Wformat=2 \
	-Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 \
	-Wduplicated-cond -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG \
	-D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -fsanitize=address \
	-fsanitize=undefined -fno-sanitize-recover -fstack-protector
CC = g++ -std=c++17 $(CC_FLAGS) -I $(SRC_PATH)
CC17 = g++ -std=c++17 -O2 -I $(SRC_PATH)
CC20 = g++ -std=c++20 -O2 -I $(SRC_PATH)

SRC_PATH = ../src

.PHONY: all clean main run_itm ece
all: main
clean:
	rm main **/*.o

main: ../src/main.cc
	$(CC) -o main ../src/main.cc

cf:
	(cd build; make -f ../Makefile cost_estimator)
run_sa:
	(cd build; make -f ../Makefile sa) && ./build/sa
run_itm:
	(cd build; make -f ../Makefile itm) && ./build/itm
ece:
	echo "read_verilog a_logic_after.v; cec a_logic_before.v" | ../abc/abc

VERILOG_PARSER_SRC_PATH = ../external/Parser-Verilog/parser-verilog
VERILOG_OUTPUT_PATH = verilog_parser_tmp
VERILOG_INCLUDES = -I $(VERILOG_OUTPUT_PATH) -I $(VERILOG_PARSER_SRC_PATH)

JSON_SRC_PATH = ../external/json/include
JSON_INCLUDES = -I $(JSON_SRC_PATH)

LORINA_SRC_PATH = ../external/lorina
LORINA_INCLUDES = -I $(LORINA_SRC_PATH)/include -I $(LORINA_SRC_PATH)/lib/fmt

aig_reader.o: $(SRC_PATH)/aig_reader.cc $(SRC_PATH)/aig_reader.hh
	$(CC17) $(LORINA_INCLUDES) \
		-c $(SRC_PATH)/aig_reader.cc -o $@

aig.o: $(SRC_PATH)/aig.cc $(SRC_PATH)/aig.hh
	$(CC17) $(LORINA_INCLUDES) \
		-c $(SRC_PATH)/aig.cc -o $@

iterative_technology_mapper.o: $(SRC_PATH)/iterative_technology_mapper.cc \
	$(SRC_PATH)/iterative_technology_mapper.hh
	$(CC17) $(LORINA_INCLUDES) \
		-c $(SRC_PATH)/iterative_technology_mapper.cc -o $@

simulated_annealing_mapper.o: $(SRC_PATH)/simulated_annealing_mapper.cc \
	$(SRC_PATH)/simulated_annealing_mapper.hh
	$(CC17) $(LORINA_INCLUDES) \
		-c $(SRC_PATH)/simulated_annealing_mapper.cc -o $@

itm: iterative_technology_mapper.o aig.o aig_reader.o cell.o library.o
	$(CC17) -o $@ $^

sa: simulated_annealing_mapper.o iterative_technology_mapper.o \
	aig.o aig_reader.o cell.o library.o
	$(CC17) -o $@ $^

library.o: $(SRC_PATH)/library.hh $(SRC_PATH)/library.cc
	$(CC17) $(JSON_INCLUDES) -I $(SRC_PATH) \
		-c $(SRC_PATH)/library.cc

netlist.o: $(SRC_PATH)/netlist.hh $(SRC_PATH)/netlist.cc
	$(CC17) $(VERILOG_INCLUDES) -I $(SRC_PATH) \
		-c $(SRC_PATH)/netlist.cc

cell.o: $(SRC_PATH)/cell.hh $(SRC_PATH)/cell.cc
	$(CC17) -c $(SRC_PATH)/cell.cc

cost_estimator.o: ../cost/cost_estimator.cc ../cost/cost_estimator.hh 
	$(CC17) $(VERILOG_INCLUDES) -I ../cost \
		-c ../cost/cost_estimator.cc -o $@

# cost_estimator: verilog_parser.tab.o verilog_lexer.yy.o \
# 	cost_estimator.o library.o cell.o netlist.o
# 	$(CC17) -o $@ $^

simple_verilog_driver.o: $(SRC_PATH)/simple_verilog_driver.cc $(SRC_PATH)/simple_verilog_driver.hh
	$(CC17) $(VERILOG_INCLUDES) -c $(SRC_PATH)/simple_verilog_driver.cc -o $@

cost_estimator: cost_estimator.o library.o cell.o netlist.o \
	simple_verilog_driver.o 
	$(CC17) -o $@ $^

###
# Parser-Verilog library
###

$(VERILOG_OUTPUT_PATH)/verilog_lexer.yy.cc: $(VERILOG_PARSER_SRC_PATH)/verilog_lexer.l
	mkdir -p $(VERILOG_OUTPUT_PATH)
	flex -o $@ $^

verilog_lexer.yy.o: $(VERILOG_OUTPUT_PATH)/verilog_lexer.yy.cc
	$(CC17) $(VERILOG_INCLUDES) -c $^

$(VERILOG_OUTPUT_PATH)/verilog_parser.tab.cc: $(VERILOG_PARSER_SRC_PATH)/verilog_parser.yy
	mkdir -p $(VERILOG_OUTPUT_PATH)
	bison -d -o $@ $^

verilog_parser.tab.o: $(VERILOG_OUTPUT_PATH)/verilog_parser.tab.cc
	$(CC17) $(VERILOG_INCLUDES) -c $^

###
# nlohmann/json library
###

# precompiled_lib_json: $(JSON_SRC_PATH)/nlohmann/json.hpp
# 	$(CC17) $(JSON_INCLUDES) -c $^
