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
	@if [ -n "$$SUDO_USER" ]; then \
		USER_HOME=$$(eval echo ~$$SUDO_USER); \
	else \
		USER_HOME=$$HOME; \
	fi; \
	if [ ! -d "$$USER_HOME/.mcsrv" ]; then \
		mkdir -p "$$USER_HOME/.mcsrv"; \
		if [ -n "$$SUDO_USER" ]; then \
			chown $$SUDO_USER:$$(id -gn $$SUDO_USER) "$$USER_HOME/.mcsrv"; \
		fi; \
	fi; \
	if [ ! -f "$$USER_HOME/.mcsrv/mcsrv.conf" ]; then \
		cp mcsrv.conf "$$USER_HOME/.mcsrv/mcsrv.conf"; \
		if [ -n "$$SUDO_USER" ]; then \
			chown $$SUDO_USER:$$(id -gn $$SUDO_USER) "$$USER_HOME/.mcsrv/mcsrv.conf"; \
		fi; \
		echo "Config file installed to $$USER_HOME/.mcsrv/mcsrv.conf"; \
	else \
		echo "Config file already exists at $$USER_HOME/.mcsrv/mcsrv.conf"; \
	fi

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(executable)
	@if [ -n "$$SUDO_USER" ]; then \
		USER_HOME=$$(eval echo ~$$SUDO_USER); \
	else \
		USER_HOME=$$HOME; \
	fi; \
	echo "Note: Config file $$USER_HOME/.mcsrv/mcsrv.conf preserved"

$(executable): $(objects)
	$(CC) $(LDFLAGS) $(objects) -o $(executable)

main.o: main.c mcsrv.conf

# Variables par défaut pour l'installation
PREFIX ?= /usr/local
DESTDIR ?=