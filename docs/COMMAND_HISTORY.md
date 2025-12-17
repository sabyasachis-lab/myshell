# MyShell Command History Implementation

## Overview

MyShell now includes a full-featured command history system that allows users to navigate through previously executed commands using up/down arrow keys, similar to bash and other modern shells.

## Features

### 1. History Storage
- **Capacity**: Stores up to 100 commands in memory
- **Circular Buffer**: Uses a circular buffer implementation for efficient memory usage
- **Duplicate Prevention**: Consecutive duplicate commands are not stored

### 2. Navigation

#### Up Arrow (↑)
- **Action**: Navigate to older (previous) commands
- **Behavior**:
  - First press: Saves current input and shows most recent command
  - Subsequent presses: Move backward through history
  - Stops at oldest available command
- **Visual**: Replaces current line with historical command

#### Down Arrow (↓)
- **Action**: Navigate to newer (more recent) commands
- **Behavior**:
  - Move forward through history
  - When reaching the end: Restores original saved input
  - Returns to "not browsing" state
- **Visual**: Replaces current line with historical command or saved input

### 3. Temporary Input Preservation
- **Feature**: Current typed input is preserved when browsing starts
- **How It Works**:
  1. User types partial command (e.g., "echo test")
  2. User presses Up arrow
  3. System saves "echo test" temporarily
  4. System displays historical commands
  5. User presses Down arrow past end of history
  6. System restores "echo test"

### 4. Smart Behavior
- **Exit History Mode**: Typing any character exits history browsing
- **Cursor Position**: Cursor moves to end when loading history
- **Screen Update**: Efficiently clears and redraws only changed portions

## Data Structures

### Command History Structure
```c
typedef struct command_history {
    char* entries[MYSHELL_HISTORY_SIZE];  // Array of command strings
    unsigned int count;                    // Total commands added
    int current_index;                     // Current position (-1 = not browsing)
    char* temp_buffer;                     // Saved current input
} myshell_command_history_t;
```

### Key Fields
- `entries[]`: Circular buffer holding command strings
- `count`: Total number of commands ever added (can exceed 100)
- `current_index`: Current position in history (-1 when not browsing)
- `temp_buffer`: Saves partial input when history navigation starts

## API Functions

### `myshell_history_init()`
- **Purpose**: Initialize history system
- **Called**: During shell startup
- **Actions**:
  - Sets count to 0
  - Sets current_index to -1
  - Initializes all entries to NULL
  - Allocates temp_buffer

### `myshell_history_add(const char* command)`
- **Purpose**: Add command to history
- **Called**: When user presses Enter
- **Behavior**:
  - Skips empty commands
  - Prevents duplicate consecutive commands
  - Uses circular buffer (oldest overwritten when full)
  - Allocates memory with `strdup()`

### `myshell_history_navigate_up()`
- **Purpose**: Move to previous (older) command
- **Called**: When Up arrow is pressed
- **Algorithm**:
  1. If first time: Save current input to temp_buffer
  2. Set current_index to most recent command
  3. If not first time: Decrement current_index
  4. Load command at current_index into input buffer
  5. Redraw terminal line

### `myshell_history_navigate_down()`
- **Purpose**: Move to next (newer) command
- **Called**: When Down arrow is pressed
- **Algorithm**:
  1. Increment current_index
  2. If past end of history:
     - Restore temp_buffer
     - Set current_index to -1 (exit browsing)
  3. Else: Load command at current_index
  4. Redraw terminal line

### `myshell_history_reset_navigation()`
- **Purpose**: Exit history browsing mode
- **Called**: When user types a character or executes command
- **Actions**:
  - Sets current_index to -1
  - Frees temp_buffer
  - Sets temp_buffer to NULL

## Implementation Details

### Circular Buffer Logic
```c
unsigned int idx = myshell_history.count % MYSHELL_HISTORY_SIZE;
```
- When `count` reaches 100, next command overwrites index 0
- When `count` reaches 101, next command overwrites index 1
- Continues cycling through 0-99

### Oldest Available Command
```c
unsigned int oldest = (count >= HISTORY_SIZE) ? 
                      (count - HISTORY_SIZE) : 0;
```
- If fewer than 100 commands: oldest is at index 0
- If 100+ commands: oldest is the one about to be overwritten

### Screen Update Algorithm
When loading a history entry:
1. **Clear current line**:
   - Move cursor to beginning
   - Print spaces over entire line
   - Return cursor to beginning
2. **Display history entry**:
   - Print the command
   - Update cursor_pos to end

### Integration with Input Processing
```c
void myshell_process_input_char(char c) {
    // Reset history on any non-arrow key
    if (c != 27 && current_index != -1) {
        myshell_history_reset_navigation();
    }
    
    // On Enter: Add to history
    if (c == '\n' || c == '\r') {
        myshell_history_add(buffer);
        myshell_history_reset_navigation();
        // ... process command
    }
}
```

## Usage Examples

### Example 1: Basic History Navigation
```
> pwd
/home/user/MyShell
> whoami
user
> date
Mon Dec 15 2025
> [Press Up]     → Shows: date
> [Press Up]     → Shows: whoami
> [Press Up]     → Shows: pwd
> [Press Down]   → Shows: whoami
> [Press Enter]  → Executes: whoami
```

### Example 2: Partial Input Preservation
```
> pwd
/home/user/MyShell
> echo test      [DON'T press Enter]
> [Press Up]     → Shows: pwd (saves "echo test")
> [Press Down]   → Shows: echo test (restored!)
> [Press Enter]  → Executes: echo test
```

### Example 3: Type to Exit Browsing
```
> pwd
> whoami
> [Press Up]     → Shows: whoami
> [Press Up]     → Shows: pwd
> [Type 'd']     → Exits browsing, shows: d
> [Type 'ate']   → Shows: date
> [Press Enter]  → Executes: date
```

## Performance Characteristics

- **Add to History**: O(1) - Direct array access
- **Navigate Up/Down**: O(1) - Direct array access
- **Screen Update**: O(n) where n = command length
- **Memory Usage**: O(100) - Fixed 100-entry buffer

## Memory Management

### Allocation
- Each history entry: `malloc()` via `strdup()`
- Temp buffer: `malloc()` via `strdup()`

### Deallocation
- Old entries: `free()` when overwritten
- Temp buffer: `free()` when exiting browsing or on shutdown
- All entries: `free()` in `myshell_abort()`

## Testing

Run test scripts:
```bash
# Interactive testing
./test_history.sh

# Automated tests
./test_history_automated.sh
```

## Limitations & Future Enhancements

### Current Limitations
- History not persisted to disk (lost on exit)
- No history search (Ctrl+R)
- No history expansion (!!, !n)
- No history manipulation commands (history, fc)

### Potential Future Enhancements
1. **Persistent History**:
   - Save to `~/.myshell_history`
   - Load on startup
   - Configurable history file location

2. **History Search**:
   - Ctrl+R for reverse search
   - Incremental search display
   - Search highlighting

3. **History Commands**:
   - `history` - List all commands
   - `history -c` - Clear history
   - `!n` - Execute command number n
   - `!!` - Execute last command
   - `!string` - Execute last command starting with string

4. **Advanced Features**:
   - Multi-line command support
   - Timestamp recording
   - Configurable history size
   - History deduplication across all entries

## Integration

The history feature integrates seamlessly with:
- ✅ Cursor movement (left/right arrows)
- ✅ Character insertion at any position
- ✅ Backspace at any position
- ✅ External command execution
- ✅ Builtin commands
- ✅ Logging system
- ✅ Signal handling

## Compatibility

- Works in standard POSIX terminals
- Uses standard ANSI escape sequences
- Tested on Linux with bash
- Arrow keys: ESC [ A (up), ESC [ B (down)
