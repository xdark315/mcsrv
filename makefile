CC = g++
CPPFLAGS = -Wall -std=c++11 -O2
OBJ = main.o cmd/cmd.o args/args.o cfg/cfg.o
PROGRAMM = mcsrv

.PHONY: all clean install uninstall

all: $(PROGRAMM)

clean: 
	$(RM) $(OBJ) $(PROGRAMM)

install : $(PROGRAMM)
	install -m 755 $(PROGRAMM) /usr/bin

uninstall :
	$(RM) /usr/bin/$(PROGRAMM)

$(PROGRAMM): $(OBJ) 
	$(CC) $(OBJ) -o $(PROGRAMM)

main.o: main.cpp
cmd.o: cmd/cmd.cpp cmd/cmd.h
args.o: args/args.cpp args/args.h
cfg.o: cfg/cfg.cpp cfg/cfg.h
