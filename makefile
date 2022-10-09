CC = g++
CPPFLAGS = -Wall -std=c++11 -O2
OBJ = main.o cmd.o args.o
PROGRAMM = mcsrv

.PHONY: all clean

all: $(PROGRAMM)

clean: 
	$(RM) $(OBJ) $(PROGRAMM)

$(PROGRAMM): $(OBJ) 
	$(CC) $(OBJ) -o $(PROGRAMM)

main.o: main.cpp
srv_cmd.o: cmd.cpp cmd.h
args.o: args.cpp args.h
