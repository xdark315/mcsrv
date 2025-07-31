# mcsrv - Detachable Server Manager

A lightweight C utility for managing detachable server processes with SSH-style configuration.

## Features

- **Detachable Server Architecture**: Start a server process and attach/detach from it at will
- **Named Pipe Communication**: Uses POSIX FIFOs for reliable client-server communication
- **SSH-style Configuration**: Flexible configuration file format with priority-based loading
- **Signal Handling**: Proper cleanup and graceful shutdown
- **User Configuration Support**: Per-user configuration in `~/.mcsrv/`

## Quick Start

```bash
# Build the project
make

# Start a server
./mcsrv start

# Attach to the running server
./mcsrv attach

# Detach from server (Ctrl+D)
# Server continues running in background

# Stop the server
./mcsrv stop
```

## Installation

```bash
# Install to /usr/local/bin (requires sudo)
sudo make install

# Install to custom location
make install PREFIX=/home/user/bin

# Uninstall
sudo make uninstall
```

The installation process will:
- Copy the binary to `$PREFIX/bin/mcsrv`
- Create `~/.mcsrv/` directory if it doesn't exist
- Install default config to `~/.mcsrv/mcsrv.conf` (preserves existing config)

## Configuration

Configuration files are loaded in priority order:
1. `./mcsrv.conf` (current directory)
2. `~/.mcsrv/mcsrv.conf` (user config)
3. `/etc/mcsrv.conf` (system config)

### Configuration Format

```bash
# Working directory for the server
folder /path/to/working/directory

# Command to run when server starts
run_command "your_command_here"

# Process ID file location
pid_file /tmp/mcsrv.pid

# Named pipe files for communication
fifo_in /tmp/mcsrv_in
fifo_out /tmp/mcsrv_out
```

### Default Configuration

```bash
folder /tmp
run_command "bash"
pid_file /tmp/mcsrv.pid
fifo_in /tmp/mcsrv_in
fifo_out /tmp/mcsrv_out
```

## Commands

- `mcsrv start` - Start the server daemon
- `mcsrv attach` - Attach to running server
- `mcsrv stop` - Stop the server daemon
- `mcsrv status` - Check server status

## Usage Examples

### Basic Server Management

```bash
# Start a bash session server
./mcsrv start

# Attach to the server
./mcsrv attach

# Run commands in the server
ls -la
cd /var/log
tail -f messages

# Detach (Ctrl+D) - server keeps running
# Reattach later
./mcsrv attach

# Stop the server
./mcsrv stop
```

### Custom Configuration

Create `~/.mcsrv/mcsrv.conf`:

```bash
folder /home/user/projects
run_command "python3 -i"
pid_file /home/user/.mcsrv/server.pid
fifo_in /home/user/.mcsrv/input
fifo_out /home/user/.mcsrv/output
```

## Architecture

### Process Model
- **Main Process**: Handles client commands and manages server lifecycle
- **Server Daemon**: Runs the configured command in a detached process
- **Pipe Reader**: Dedicated process for handling client input to prevent blocking

### Communication
- Uses POSIX named pipes (FIFOs) for bidirectional communication
- Input pipe: Client → Server
- Output pipe: Server → Client
- Robust handling of client disconnections

### Signal Handling
- `SIGTERM`/`SIGINT`: Graceful shutdown with cleanup
- `SIGPIPE`: Handled to prevent crashes on broken pipes
- `SIGCHLD`: Automatic zombie process cleanup

## Building

### Requirements
- C23-compatible compiler (GCC 12+ or Clang 15+)
- POSIX-compliant system (Linux, macOS, *BSD)
- GNU Make

### Compilation Flags
```bash
CFLAGS = -std=c23 -O2 -Wall -Wextra -Wpedantic
```

### Development on macOS
The project is designed for Linux servers but can be developed on macOS. The code uses POSIX standards for maximum portability.

## Troubleshooting

### Server Won't Start
- Check if another instance is running: `ps aux | grep mcsrv`
- Verify write permissions for PID file and FIFO locations
- Check configuration file syntax

### Can't Attach
- Ensure server is running: `./mcsrv status`
- Check FIFO file permissions
- Verify configuration matches between start and attach

### Server Stops Unexpectedly
- Check system logs for error messages
- Ensure the `run_command` is valid and executable
- Verify sufficient disk space for PID and FIFO files

## File Structure

```
mcsrv/
├── main.c           # Main source code
├── mcsrv.conf      # Default configuration
├── makefile        # Build system
├── README.md       # This file
└── test.sh         # Test script
```

## License

This project is open source. Please check the license file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly on target platforms
5. Submit a pull request

## Platform Support

- **Primary Target**: Linux servers
- **Development**: macOS (with full compatibility)
- **Tested**: Ubuntu, CentOS, macOS
- **Architecture**: POSIX-compliant systems

## Security Considerations

- FIFO files are created with restricted permissions
- PID files prevent multiple server instances
- No privilege escalation required for normal operation
- User configuration files respect standard Unix permissions
