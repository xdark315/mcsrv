CC = gcc
CFLAGS = -std=c2x -O2 -Wall -Wextra -Wpedantic
LDFLAGS = 
objects = main.o
executable = mcsrv

.PHONY: all clean install uninstall

all: $(executable)

clean:
	$(RM) $(objects) $(executable)

install: $(executable)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(executable) $(DESTDIR)$(PREFIX)/bin/
	@echo "Installing config file..."
	@if [ ! -d "$$HOME/.mcsrv" ]; then mkdir -p "$$HOME/.mcsrv"; fi
	@if [ ! -f "$$HOME/.mcsrv/mcsrv.conf" ]; then \
		cp mcsrv.conf "$$HOME/.mcsrv/mcsrv.conf"; \
		echo "Config file installed to $$HOME/.mcsrv/mcsrv.conf"; \
	else \
		echo "Config file already exists at $$HOME/.mcsrv/mcsrv.conf"; \
	fi

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(executable)
	@echo "Note: Config file $$HOME/.mcsrv/mcsrv.conf preserved"

$(executable): $(objects)
	$(CC) $(LDFLAGS) $(objects) -o $(executable)

main.o: main.c mcsrv.conf

# Variables par défaut pour l'installation
PREFIX ?= /usr/local
DESTDIR ?=