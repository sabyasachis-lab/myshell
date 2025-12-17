# Makefile for MyShell

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = 

# Core dump settings
CORE_PATTERN = core.%e.%p
CORE_SIZE = unlimited 

# Directories
SRCDIR = src
OBJDIR = obj
BINDIR = .

# Target executable
TARGET = mysh

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Default target
all: $(TARGET)

# Create target executable
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $(BINDIR)/$(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"
	@echo "Core dumps enabled - use 'make setup-core' to configure"

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they don't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Clean up build artifacts
clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)
	rm -f core core.*
	@echo "Clean complete"

# Rebuild everything
rebuild: clean all

# Install (copy to /usr/local/bin)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "Installed $(TARGET) to /usr/local/bin/"

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled $(TARGET) from /usr/local/bin/"

# Setup core dump configuration
setup-core:
	@echo "Setting up core dump configuration..."
	@echo "Current core pattern: $$(cat /proc/sys/kernel/core_pattern 2>/dev/null || echo 'unknown')"
	@echo "Current ulimit -c: $$(ulimit -c)"
	@echo "Enabling core dumps..."
	@ulimit -c $(CORE_SIZE)
	@echo "Core dumps enabled with size limit: $(CORE_SIZE)"
	@echo "Core files will be named: $(CORE_PATTERN)"
	@echo "Note: You may need to run 'sudo sysctl kernel.core_pattern=$(CORE_PATTERN)' for custom naming"

# Run the shell with core dumps enabled
run: $(TARGET) setup-core
	@echo "Running $(TARGET) with core dumps enabled..."
	@ulimit -c $(CORE_SIZE) && ./$(TARGET)

# Run with gdb for debugging
debug-run: $(TARGET)
	@echo "Running $(TARGET) in GDB..."
	gdb -ex "set confirm off" -ex "run" -ex "bt" -ex "quit" ./$(TARGET)

# Analyze core dump if it exists
analyze-core: $(TARGET)
	@if [ -f core ]; then \
		echo "Analyzing core dump..."; \
		gdb -batch -ex "bt" -ex "quit" ./$(TARGET) core; \
	elif ls core.* >/dev/null 2>&1; then \
		echo "Analyzing core dump..."; \
		CORE_FILE=$$(ls -t core.* | head -1); \
		echo "Using core file: $$CORE_FILE"; \
		gdb -batch -ex "bt" -ex "quit" ./$(TARGET) $$CORE_FILE; \
	else \
		echo "No core dump found. Run 'make run' to generate one if the program crashes."; \
	fi

# Debug build with extra flags
# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release build with optimization
release: CFLAGS += -O2 -DNDEBUG
release: clean $(TARGET)

# Show help
help:
	@echo "Available targets:"
	@echo "  all         - Build the shell (default)"
	@echo "  clean       - Remove build artifacts and core files"
	@echo "  rebuild     - Clean and build"
	@echo "  run         - Build and run the shell with core dumps enabled"
	@echo "  debug       - Build with debug flags"
	@echo "  debug-run   - Run the shell in GDB debugger"
	@echo "  setup-core  - Configure core dump settings"
	@echo "  analyze-core- Analyze existing core dump with GDB"
	@echo "  release     - Build optimized release version"
	@echo "  install     - Install to /usr/local/bin"
	@echo "  uninstall   - Remove from /usr/local/bin"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Debugging workflow:"
	@echo "  1. make run          # Run with core dumps enabled"
	@echo "  2. make analyze-core # Analyze crash if it occurs"
	@echo "  3. make debug-run    # Run in GDB for interactive debugging"
	@echo ""
	@echo "Logging modes (runtime options):"
	@echo "  Console: ./mysh -v CONSOLE       # Logs to stderr"
	@echo "  File:    ./mysh -v FILE -f path  # Logs to file"
	@echo "  Default: ./mysh                  # No logging"

# Declare phony targets
.PHONY: all clean rebuild install uninstall run debug release help setup-core debug-run analyze-core

# Show variables (for debugging makefile)
print-%:
	@echo $* = $($*)