# This file is meant to be executed from the ./build folder.

CC_FLAGS = -Wall -Wextra -pedantic -O2 -Wshadow -Wformat=2 \
	-Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 \
	-Wduplicated-cond -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG \
	-D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -fsanitize=address \
	-fsanitize=undefined -fno-sanitize-recover -fstack-protector
CC = g++ -std=c++17 $(CC_FLAGS) -I $(SRC_PATH)
CC17 = g++ -std=c++17 -O2 -I $(SRC_PATH)

SRC_PATH = ../src

all: main
clean:
	rm main **/*.o

main: ../src/main.cc
	$(CC) -o main ../src/main.cc

VERILOG_PARSER_SRC_PATH = ../external/Parser-Verilog/parser-verilog
VERILOG_OUTPUT_PATH = verilog_parser_tmp
VERILOG_INCLUDES = -I $(VERILOG_OUTPUT_PATH) -I $(VERILOG_PARSER_SRC_PATH)

JSON_SRC_PATH = ../external/json/include
JSON_INCLUDES = -I $(JSON_SRC_PATH)

library.o: $(SRC_PATH)/library.hh $(SRC_PATH)/library.cc
	$(CC17) $(JSON_INCLUDES) -I $(SRC_PATH) \
		-c $(SRC_PATH)/library.cc

cell.o: $(SRC_PATH)/cell.hh $(SRC_PATH)/cell.cc
	$(CC17) -c $(SRC_PATH)/cell.cc

verilog_parser.o: ../cost/verilog_parser.cc ../cost/verilog_parser.hh 
	$(CC17) $(VERILOG_INCLUDES) -I ../cost \
		-c ../cost/verilog_parser.cc

verilog_parser: verilog_parser.tab.o verilog_lexer.yy.o \
	verilog_parser.o library.o cell.o
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
