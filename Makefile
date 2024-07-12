CC = g++ -std=c++17 -O3 -march=native

main: ../src/main.cc
	$(CC) -o main ../src/main.cc

all: main
clean:
	rm main