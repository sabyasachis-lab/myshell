# MyShell - A Custom Shell Implementation in C

MyShell is a feature-rich custom shell implementation written in C, with support for raw terminal input, command history, output redirection, and more.

## Features

- **Raw Terminal Mode**: Character-by-character input processing
- **Command History**: 100-entry persistent history with up/down arrow navigation
- **Cursor Movement**: Left/right arrow keys with character insertion/deletion
- **Output Redirection**: Support for `>` (write) and `>>` (append) operators
- **External Commands**: Execute programs from BINPATH or current directory
- **Built-in Commands**: echo, cd, pwd, ls, cat, touch, mkdir, rm, cp, mv, env, exit, quit, help
- **Hash Table Lookup**: O(1) command resolution using djb2 algorithm
- **Runtime Logging**: Console or file logging with configurable verbosity

## Quick Start

### From Source Archive

1. **Extract the archive**:
   ```bash
   tar -xzf myshell_YYYYMMDD_HHMMSS.tar.gz
   cd MyShell
   ```

2. **Run the configuration script**:
   ```bash
   ./configure
   ```
   This will check for and optionally install required dependencies (gcc, make, build-essential).

3. **Build MyShell**:
   ```bash
   make
   ```

4. **Run MyShell**:
   ```bash
   ./mysh
   ```

### Building from Repository

```bash
git clone <repository-url>
cd MyShell
./configure
make
./mysh
```

## Usage

### Basic Usage

```bash
./mysh                          # Start shell with no logging
./mysh -v CONSOLE              # Start with console logging
./mysh -v FILE -f mylog.log    # Start with file logging
./mysh --help                  # Show help message
```

### Interactive Commands

- **exit, quit** - Exit the shell
- **Ctrl+D** - Exit the shell
- **Ctrl+C** - Clear current input (shell continues running)
- **Up/Down Arrow** - Navigate command history
- **Left/Right Arrow** - Move cursor within current line

### Command Examples

```bash
echo hello world               # Echo text
pwd                           # Print working directory
cd /path/to/dir               # Change directory
ls                            # List directory contents
cat file.txt                  # Display file contents
echo output > file.txt        # Write output to file
echo more >> file.txt         # Append output to file
/bin/date                     # Run external command with absolute path
whoami                        # Run external command from BINPATH
```

## Project Structure

```
MyShell/
├── src/                      # Source code
│   ├── main.c               # Entry point
│   ├── myshell.c/h          # Core shell logic
│   ├── builtin_commands.c/h # Built-in command implementations
│   ├── external_commands.c/h# External command execution
│   ├── output_redirection.c/h# Output redirection handling
│   ├── hash_table.c/h       # Hash table for command lookup
│   ├── util.c/h             # Utility functions
│   └── log.h                # Logging macros
├── tests/                   # Test scripts
│   ├── test_external.sh     # Test external command execution
│   ├── test_history*.sh     # Test command history
│   ├── test_cursor*.sh      # Test cursor movement
│   ├── test_logging*.sh     # Test logging functionality
│   └── comprehensive_test.sh# Run all tests
├── docs/                    # Documentation
│   ├── DESIGN_SPEC.md       # Design specification
│   ├── FUNCTIONAL_SPEC.md   # Functional specification
│   ├── COMMAND_HISTORY.md   # Command history implementation
│   ├── CURSOR_MOVEMENT.md   # Cursor movement implementation
│   └── LOGGING.md           # Logging system documentation
├── Makefile                 # Build configuration
├── configure                # Dependency checker and installer
├── pack.sh                  # Script to create distribution archive
└── README.md               # This file
```

## Dependencies

- **GCC** (version 4.8 or higher)
- **Make** (version 3.81 or higher)
- **Standard C Library** (glibc or equivalent)
- **POSIX-compliant system** (Linux, macOS, BSD)

The `configure` script will automatically check and help install these dependencies on:
- Ubuntu/Debian (apt)
- Fedora (dnf)
- CentOS/RHEL (yum)
- Arch Linux (pacman)

## Building and Testing

### Build

```bash
make              # Build mysh executable
make clean        # Remove build artifacts
```

### Test

```bash
cd tests
./comprehensive_test.sh        # Run all tests
./test_external.sh            # Test external commands
./test_history_automated.sh   # Test command history
./test_cursor_automated.sh    # Test cursor movement
```

## Creating Distribution Archive

To create a distribution archive:

```bash
./pack.sh
```

This creates a timestamped `.tar.gz` archive in the parent directory with all source files, tests, documentation, and build scripts.

## Command History

MyShell maintains a persistent command history:
- **Storage**: Up to 100 commands in memory
- **Persistence**: Saved to `~/.myshell_history` on exit, loaded on startup
- **Navigation**: Use Up/Down arrows to browse history
- **Smart Behavior**: Current input saved when browsing starts, restored when returning

## Output Redirection

MyShell supports standard output redirection:
- `command > file.txt` - Write output to file (overwrites)
- `command >> file.txt` - Append output to file

Works with both built-in and external commands.

## Technical Details

- **Language**: C99 with POSIX extensions
- **Terminal Control**: Raw mode using termios
- **Process Management**: fork/exec pattern for external commands
- **Memory Management**: Dynamic allocation with proper cleanup
- **Signal Handling**: SIGINT, SIGTERM, SIGQUIT, SIGTSTP

## Known Limitations

- No pipe support (`|`)
- No input redirection (`<`)
- No background jobs (`&`)
- No job control (fg, bg, jobs)
- No command substitution
- No wildcard expansion (globbing)

## Contributing

Feel free to submit issues and enhancement requests!

## License

[Specify your license here]

## Authors

[Specify authors here]

## Version History

- **v1.0.0** - Initial release with core features
  - Command execution (built-in and external)
  - Command history with persistent storage
  - Cursor movement and line editing
  - Output redirection (>, >>)
  - Runtime logging configuration
