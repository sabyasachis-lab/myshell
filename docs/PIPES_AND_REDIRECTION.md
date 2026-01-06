# Pipes and Output Redirection

## Overview
MyShell supports Unix-style pipes (`|`) and output redirection operators (`>` and `>>`), including combinations of both.

## Pipe Operator (`|`)

### Basic Usage
```bash
command1 | command2
```
Pipes the output of `command1` as input to `command2`.

### Examples
```bash
# Simple pipe
echo hello world | /bin/cat

# Multi-stage pipeline
ls | /bin/grep txt | /bin/wc -l

# Up to 16 commands in a pipeline
pwd | /bin/cat | /bin/cat | /bin/cat
```

### Supported Commands
- Both built-in and external commands work in pipelines
- External commands are resolved using BINPATH
- Examples: `echo`, `pwd`, `/bin/cat`, `/bin/grep`, `/bin/wc`, `/bin/tr`, `/bin/sort`

## Output Redirection

### Write Mode (`>`)
```bash
command > file.txt
```
Redirects stdout to `file.txt`, overwriting if it exists.

### Append Mode (`>>`)
```bash
command >> file.txt
```
Redirects stdout to `file.txt`, appending to existing content.

### Examples
```bash
# Simple redirection
echo hello > output.txt

# Append mode
echo world >> output.txt
```

## Combining Pipes and Redirection

### Syntax
```bash
cmd1 | cmd2 | cmd3 > output.txt
```
The output of the **last command** in the pipeline is redirected to the file.

### Examples
```bash
# Count lines from pipeline output
echo one two three | /bin/tr ' ' '\n' | /bin/wc -l > count.txt

# Grep and save results
ls | /bin/grep Makefile > makefiles.txt

# Multi-stage processing with append
pwd | /bin/tr '/' '\n' >> paths.txt
```

## Implementation Details

### Pipeline Execution
1. **Parsing**: Command line is split on `|` into individual commands
2. **Pipe Creation**: Creates N-1 pipes for N commands
3. **Process Forking**: Each command runs in its own child process
4. **File Descriptor Setup**:
   - Command 1: stdin from terminal, stdout to pipe 1
   - Command 2-N-1: stdin from previous pipe, stdout to next pipe
   - Command N: stdin from previous pipe, stdout to terminal or redirect file
5. **Synchronization**: Parent waits for all children to complete

### Output Redirection in Pipelines
- Redirection is applied **only to the last command** in the pipeline
- Uses `fopen()` with mode `"w"` (write) or `"a"` (append)
- File descriptor is set up before executing the final command
- Properly closed after all processes complete

### Limitations
- Maximum 16 commands per pipeline (`MYSHELL_MAX_PIPE_COMMANDS`)
- Input redirection (`<`) not yet implemented
- Here documents (`<<`) not yet implemented

## Error Handling

### Invalid Pipelines
```bash
# Empty command
| /bin/cat         # Error: missing command

# Too many commands
cmd1 | ... | cmd17  # Error: exceeds max pipeline length
```

### File Errors
```bash
# Permission denied
echo test | /bin/cat > /root/file.txt  # Error: cannot open file

# Invalid path
echo test > /nonexistent/dir/file.txt  # Error: cannot open file
```

## Testing

Run the comprehensive test suite:
```bash
cd tests
./test_pipes.sh
```

Tests include:
1. Simple pipes (2 commands)
2. Multi-stage pipelines (3+ commands)
3. Built-in commands in pipes
4. External commands in pipes
5. Pipe with `>` redirection
6. Pipe with `>>` append
7. Complex processing pipelines

## See Also
- `docs/OUTPUT_REDIRECTION.md` - Output redirection without pipes
- `docs/DESIGN_SPEC.md` - Overall shell architecture
- `src/pipe.c` - Pipeline implementation
- `src/output_redirection.c` - Redirection implementation
