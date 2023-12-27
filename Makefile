CC=g++
# FLAGS=-std=c++17 -fsanitize=undefined,address -Wall -Wshadow -DLOCAL -pedantic
FLAGS=-std=c++17 -O3 -static


all:
	$(CC) main.cpp -o main $(FLAGS)