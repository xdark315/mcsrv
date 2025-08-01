// main.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>

#define VERSION "1.0.0"
#define CONFIG_FILE "mcsrv.conf"
#define USER_CONFIG_DIR ".mcsrv"
#define USER_CONFIG_FILE "mcsrv.conf"
#define SYSTEM_CONFIG_FILE "/etc/mcsrv.conf"
#define MAX_CONFIG_LINE 256
#define MAX_CONFIG_VALUE 256

// Configuration structure
typedef struct {
    char folder[MAX_CONFIG_VALUE];
    char run_command[MAX_CONFIG_VALUE];
    char pid_file[MAX_CONFIG_VALUE];
    char fifo_in[MAX_CONFIG_VALUE];
    char fifo_out[MAX_CONFIG_VALUE];
} config_t;

// Global configuration
static config_t config;

void handle_argument(const char *arg);
void print_help(void);
void print_default(void);
void print_version(void);

void start_server(void);
void attach_server(void);
void stop_server(void);
void show_status(void);
pid_t get_server_pid(void);
int is_process_running(pid_t pid);
void cleanup_files(void);
void setup_terminal(void);
void restore_terminal(void);
void sigint_handler(int sig);
void load_config(void);
void init_default_config(void);
char* trim_whitespace(char* str);

static volatile int detach_requested = 0;
static struct termios original_termios;
static int server_fifo_in = -1;

int main(int argc, char *argv[] ) {
    // Load configuration
    load_config();
    
    //read command line arguments
    if (argc < 2) {
        print_default();
        return EXIT_FAILURE;
    }
    
    handle_argument(argv[1]);

    return EXIT_SUCCESS;
}

void handle_argument(const char *arg) {
    if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
        print_help();
    } else if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0) {
        print_version();
    } else if (strcmp(arg, "start") == 0) {
        start_server();
    } else if (strcmp(arg, "attach") == 0) {
        attach_server();
    } else if (strcmp(arg, "stop") == 0) {
        stop_server();
    } else if (strcmp(arg, "status") == 0) {
        show_status();
    } else {
        fprintf(stderr, "Unknown argument: %s\n", arg);
        print_default();
    }
}

void print_default(void) {
    fprintf(stderr, "Usage: mcsrv <argument>\n");
    fprintf(stderr, "Run with --help or -h for more information.\n");
}

void print_version(void) {
    fprintf(stderr, "mcsrv version %s\n", VERSION);
    fprintf(stderr, "Built with C2x standard.\n");
}

void print_help(void) {
    print_default();
    fprintf(stderr, "Available commands:\n");
    fprintf(stderr, "  --help, -h: Show this help message\n");
    fprintf(stderr, "  --version, -v: Show version information\n");
    fprintf(stderr, "  start: Start the server in background (detached)\n");
    fprintf(stderr, "  attach: Attach to a running server\n");
    fprintf(stderr, "  stop: Stop the running server\n");
    fprintf(stderr, "  status: Show server status\n");
}

void start_server(void) {
    // Check if server is already running
    pid_t existing_pid = get_server_pid();
    if (existing_pid > 0 && is_process_running(existing_pid)) {
        fprintf(stderr, "Server is already running with PID %d\n", existing_pid);
        return;
    }

    // Clean up any leftover files
    cleanup_files();

    // Create named pipes for communication
    if (mkfifo(config.fifo_in, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo FIFO_IN");
        return;
    }
    if (mkfifo(config.fifo_out, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo FIFO_OUT");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        cleanup_files();
        return;
    }

    if (pid == 0) {
        // Child process - become daemon
        if (setsid() == -1) {
            perror("setsid");
            exit(EXIT_FAILURE);
        }

        // Write PID to file
        FILE *pid_file = fopen(config.pid_file, "w");
        if (pid_file) {
            fprintf(pid_file, "%d\n", getpid());
            fclose(pid_file);
        }

        // Create a pipe reader process that keeps the FIFO open
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t reader_pid = fork();
        if (reader_pid == -1) {
            perror("fork reader");
            exit(EXIT_FAILURE);
        }

        if (reader_pid == 0) {
            // Pipe reader process
            close(pipe_fd[0]); // Close read end
            
            // Keep FIFO_IN open and copy to pipe
            while (1) {
                int fifo_fd = open(config.fifo_in, O_RDONLY);
                if (fifo_fd == -1) {
                    sleep(1);
                    continue;
                }
                
                char buffer[1024];
                ssize_t bytes;
                while ((bytes = read(fifo_fd, buffer, sizeof(buffer))) > 0) {
                    ssize_t written = write(pipe_fd[1], buffer, bytes);
                    if (written == -1) {
                        perror("write to pipe");
                        break;
                    }
                }
                close(fifo_fd);
                // FIFO closed, wait a bit and reopen
                struct timespec sleep_time = {0, 100000000}; // 100ms = 100000000 nanoseconds
                nanosleep(&sleep_time, NULL);
            }
        } else {
            // Main server process
            close(pipe_fd[1]); // Close write end
            
            // Redirect stdin from pipe, stdout/stderr to FIFO_OUT
            if (dup2(pipe_fd[0], STDIN_FILENO) == -1) {
                perror("dup2 stdin");
                exit(EXIT_FAILURE);
            }
            close(pipe_fd[0]);
            
            if (freopen(config.fifo_out, "w", stdout) == NULL) {
                perror("freopen stdout");
                exit(EXIT_FAILURE);
            }
            if (freopen(config.fifo_out, "w", stderr) == NULL) {
                perror("freopen stderr");
                exit(EXIT_FAILURE);
            }

            // change directory to where the script is located
            if (chdir(config.folder) == -1) {
                perror("chdir");
                exit(EXIT_FAILURE);
            }

            // Execute the script
            execl("/bin/bash", "bash", config.run_command, (char *)NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        fprintf(stderr, "Server started in background with PID %d\n", pid);
        fprintf(stderr, "Use 'mcsrv attach' to connect to it\n");
    }
}

void attach_server(void) {
    pid_t server_pid = get_server_pid();
    if (server_pid <= 0 || !is_process_running(server_pid)) {
        fprintf(stderr, "No server is currently running\n");
        fprintf(stderr, "Use 'mcsrv start' to start a server\n");
        return;
    }

    fprintf(stderr, "Attaching to server (PID %d)...\n", server_pid);
    fprintf(stderr, "Press Ctrl+D to detach (server will keep running)\n");
    fprintf(stderr, "Type 'stop' or press Ctrl+C to stop the server\n\n");

    // Set up signal handler for Ctrl+C
    signal(SIGINT, sigint_handler);

    // Open pipes for communication
    int fifo_out = open(config.fifo_out, O_RDONLY | O_NONBLOCK);
    int fifo_in = open(config.fifo_in, O_WRONLY | O_NONBLOCK);
    server_fifo_in = fifo_in; // Store for signal handler

    if (fifo_out == -1 || fifo_in == -1) {
        perror("Failed to open communication pipes");
        return;
    }

    fd_set read_fds;
    char buffer[1024];
    char input_line[1024];
    ssize_t bytes_read;
    detach_requested = 0;

    // Use line buffering for input
    setvbuf(stdin, NULL, _IOLBF, 0);
    setvbuf(stdout, NULL, _IOLBF, 0);

    while (is_process_running(server_pid) && !detach_requested) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(fifo_out, &read_fds);

        struct timeval timeout = {0, 100000}; // 100ms timeout
        int activity = select(fifo_out + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0) {
            if (errno == EINTR) {
                continue; // Interrupted by signal
            }
            perror("select");
            break;
        }

        // Read from server output
        if (FD_ISSET(fifo_out, &read_fds)) {
            bytes_read = read(fifo_out, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("%s", buffer);
                fflush(stdout);
            }
        }

        // Read from stdin (line by line)
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(input_line, sizeof(input_line), stdin) != NULL) {
                // Check if it's a stop command
                if (strncmp(input_line, "stop", 4) == 0 || 
                    strncmp(input_line, "quit", 4) == 0 ||
                    strncmp(input_line, "q\n", 2) == 0 ||
                    strncmp(input_line, "exit", 4) == 0) {
                    ssize_t written = write(fifo_in, input_line, strlen(input_line));
                    if (written == -1) {
                        perror("write stop command");
                    }
                    detach_requested = 1;
                    break;
                }
                
                // Send input to server
                ssize_t written = write(fifo_in, input_line, strlen(input_line));
                if (written == -1) {
                    perror("write to server");
                    break;
                }
            } else {
                // EOF detected (Ctrl+D) - just detach, don't send EOF to server
                if (feof(stdin)) {
                    detach_requested = 1;
                    clearerr(stdin); // Clear EOF flag
                    break;
                } else {
                    // Other error
                    detach_requested = 1;
                    break;
                }
            }
        }
    }

    // Restore default signal handler
    signal(SIGINT, SIG_DFL);
    server_fifo_in = -1;

    close(fifo_out);
    close(fifo_in);
    
    if (detach_requested) {
        if (is_process_running(server_pid)) {
            fprintf(stderr, "\nDetached from server\n");
        } else {
            fprintf(stderr, "\nServer has stopped\n");
        }
    } else {
        fprintf(stderr, "\nServer has stopped\n");
    }
}

void stop_server(void) {
    pid_t server_pid = get_server_pid();
    if (server_pid <= 0 || !is_process_running(server_pid)) {
        fprintf(stderr, "No server is currently running\n");
        return;
    }

    fprintf(stderr, "Stopping server (PID %d)...\n", server_pid);
    
    // Try to send "stop" command to the server via stdin
    int fifo_in = open(config.fifo_in, O_WRONLY | O_NONBLOCK);
    if (fifo_in != -1) {
        const char *stop_cmd = "stop\n";
        ssize_t written = write(fifo_in, stop_cmd, strlen(stop_cmd));
        if (written == -1) {
            perror("write stop command");
        }
        close(fifo_in);
        
        // Wait a bit for graceful shutdown
        sleep(3);
    } else {
        fprintf(stderr, "Could not send stop command to server\n");
    }
    
    // Check if server stopped gracefully
    if (is_process_running(server_pid)) {
        fprintf(stderr, "Server didn't stop gracefully, forcing shutdown...\n");
        if (kill(server_pid, SIGKILL) == -1) {
            perror("kill server");
        }
        sleep(1);
    }

    cleanup_files();
    fprintf(stderr, "Server stopped\n");
}

void show_status(void) {
    pid_t server_pid = get_server_pid();
    if (server_pid <= 0) {
        fprintf(stderr, "No PID file found - server is not running\n");
        return;
    }

    if (is_process_running(server_pid)) {
        fprintf(stderr, "Server is running with PID %d\n", server_pid);
    } else {
        fprintf(stderr, "PID file exists but process %d is not running\n", server_pid);
        cleanup_files();
    }
}

pid_t get_server_pid(void) {
    FILE *pid_file = fopen(config.pid_file, "r");
    if (!pid_file) {
        return -1;
    }

    pid_t pid;
    if (fscanf(pid_file, "%d", &pid) != 1) {
        fclose(pid_file);
        return -1;
    }

    fclose(pid_file);
    return pid;
}

int is_process_running(pid_t pid) {
    return kill(pid, 0) == 0;
}

void cleanup_files(void) {
    if (unlink(config.pid_file) == -1 && errno != ENOENT) {
        perror("unlink pid_file");
    }
    if (unlink(config.fifo_in) == -1 && errno != ENOENT) {
        perror("unlink fifo_in");
    }
    if (unlink(config.fifo_out) == -1 && errno != ENOENT) {
        perror("unlink fifo_out");
    }
}

void setup_terminal(void) {
    struct termios new_termios;
    
    // Get current terminal attributes
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        perror("tcgetattr");
        return;
    }
    
    // Copy current attributes
    new_termios = original_termios;
    
    // Disable canonical mode and echo
    new_termios.c_lflag &= ~(ICANON | ECHO);
    
    // Set minimum characters to read
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;
    
    // Apply new settings
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1) {
        perror("tcsetattr");
    }
}

void restore_terminal(void) {
    // Restore original terminal attributes
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_termios) == -1) {
        perror("tcsetattr restore");
    }
}

void sigint_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    
    // Send stop command to server if we have a pipe open
    if (server_fifo_in != -1) {
        const char *stop_cmd = "stop\n";
        ssize_t written = write(server_fifo_in, stop_cmd, strlen(stop_cmd));
        if (written == -1) {
            // Can't use perror in signal handler, just ignore
        }
    }
    
    // Set flag to exit main loop
    detach_requested = 1;
}

char* trim_whitespace(char* str) {
    char *end;
    
    // Trim leading space
    while (*str == ' ' || *str == '\t') str++;
    
    if (*str == 0) return str;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    
    end[1] = '\0';
    
    return str;
}

void init_default_config(void) {
    strcpy(config.folder, "test_srv");
    strcpy(config.run_command, "./run.sh");
    strcpy(config.pid_file, "/tmp/mcsrv.pid");
    strcpy(config.fifo_in, "/tmp/mcsrv_in");
    strcpy(config.fifo_out, "/tmp/mcsrv_out");
}

void load_config(void) {
    // Initialize with default values
    init_default_config();
    
    char config_path[512];
    FILE *file = NULL;
    
    // Try to find config file in order of priority:
    // 1. Current directory
    file = fopen(CONFIG_FILE, "r");
    if (file) {
        strcpy(config_path, CONFIG_FILE);
    } else {
        // 2. User's home directory (~/.mcsrv/mcsrv.conf)
        const char *home = getenv("HOME");
        if (home) {
            snprintf(config_path, sizeof(config_path), "%s/%s/%s", 
                     home, USER_CONFIG_DIR, USER_CONFIG_FILE);
            file = fopen(config_path, "r");
        }
        
        if (!file) {
            // 3. System config file
            file = fopen(SYSTEM_CONFIG_FILE, "r");
            if (file) {
                strcpy(config_path, SYSTEM_CONFIG_FILE);
            }
        }
    }
    
    if (!file) {
        fprintf(stderr, "Warning: No config file found, using defaults\n");
        return;
    }
    
    fprintf(stderr, "Loading config from: %s\n", config_path);
    
    char line[MAX_CONFIG_LINE];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        char *trimmed = trim_whitespace(line);
        if (trimmed[0] == '#' || trimmed[0] == '\0') {
            continue;
        }
        
        // Find the space separator
        char *space = strchr(trimmed, ' ');
        if (!space) {
            space = strchr(trimmed, '\t');
        }
        
        if (space) {
            *space = '\0';
            char *key = trimmed;
            char *value = trim_whitespace(space + 1);
            
            // Parse configuration values
            if (strcmp(key, "folder") == 0) {
                strncpy(config.folder, value, MAX_CONFIG_VALUE - 1);
                config.folder[MAX_CONFIG_VALUE - 1] = '\0';
            } else if (strcmp(key, "run_command") == 0) {
                strncpy(config.run_command, value, MAX_CONFIG_VALUE - 1);
                config.run_command[MAX_CONFIG_VALUE - 1] = '\0';
            } else if (strcmp(key, "pid_file") == 0) {
                strncpy(config.pid_file, value, MAX_CONFIG_VALUE - 1);
                config.pid_file[MAX_CONFIG_VALUE - 1] = '\0';
            } else if (strcmp(key, "fifo_in") == 0) {
                strncpy(config.fifo_in, value, MAX_CONFIG_VALUE - 1);
                config.fifo_in[MAX_CONFIG_VALUE - 1] = '\0';
            } else if (strcmp(key, "fifo_out") == 0) {
                strncpy(config.fifo_out, value, MAX_CONFIG_VALUE - 1);
                config.fifo_out[MAX_CONFIG_VALUE - 1] = '\0';
            }
        }
    }
    
    fclose(file);
}
