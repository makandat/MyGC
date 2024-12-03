SRC = my_gc.c my_gc.h
TARGET = bin/my_gc.so
OPT = -g -shared -std=c11
$(TARGET) : $(SRC)
	gcc -o $@ $(OPT) $(SRC)
