# This file is meant to be executed from the ./build folder.

CC_FLAGS = -Wall -Wextra -pedantic -std=c++11 -O2 -Wshadow -Wformat=2 \
	-Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 \
	-Wduplicated-cond -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG \
	-D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -fsanitize=address \
	-fsanitize=undefined -fno-sanitize-recover -fstack-protector
CC = g++ -std=c++20 $(CC_FLAGS)
CC17 = g++ -std=c++17 -O2

SRC_PATH = ../src

main: ../src/main.cc
	$(CC) -o main ../src/main.cc

all: main
clean:
	rm main

VERILOG_PARSER_SRC_PATH = ../external/Parser-Verilog/parser-verilog
VERILOG_OUTPUT_PATH = verilog_parser_tmp
VERILOG_INCLUDES = -I $(VERILOG_OUTPUT_PATH) -I $(VERILOG_PARSER_SRC_PATH)

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

verilog_parser.o: ../cost/verilog_parser.cc
	$(CC17) $(VERILOG_INCLUDES) -c $^

verilog_parser: verilog_parser.tab.o verilog_lexer.yy.o verilog_parser.o
	$(CC17) -o $@ $^
