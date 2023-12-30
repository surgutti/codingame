CC=g++
# FLAGS=-std=c++17 -fsanitize=undefined,address -Wall -Wshadow -DLOCAL -pedantic
FLAGS=-std=c++17 -O3
DRAW_FLAGS=-lsfml-graphics -lsfml-window -lsfml-system

all:
	$(CC) main.cpp -o main $(FLAGS)

draw:
	$(CC) draw.cpp -o draw $(FLAGS) $(DRAW_FLAGS)