CC=g++
FLAGS=-std=c++17 -fsanitize=undefined,address -Wall -Wshadow -DLOCAL -pedantic

all:
	$(CC) main.cpp -o main $(FLAGS)