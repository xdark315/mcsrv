CC = g++
CPPFLAGS = -Wall -std=c++11 -O2
OBJ = main.o cmd/cmd.o args/args.o
PROGRAMM = mcsrv

.PHONY: all clean

all: $(PROGRAMM)

clean: 
	$(RM) $(OBJ) $(PROGRAMM)

$(PROGRAMM): $(OBJ) 
	$(CC) $(OBJ) -o $(PROGRAMM)

main.o: main.cpp
cmd.o: cmd/cmd.cpp cmd/cmd.h
args.o: args/args.cpp args/args.h
