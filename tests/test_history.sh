#!/bin/bash

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║      MyShell Command History - Interactive Demo          ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""
echo "The shell now supports command history with:"
echo "  • Up Arrow (↑)   : Navigate to previous commands"
echo "  • Down Arrow (↓) : Navigate to next commands"
echo "  • 100 command history buffer"
echo "  • Remembers current input when browsing starts"
echo "  • Duplicate prevention"
echo ""
echo "Try this workflow:"
echo "  1. Type and execute: pwd"
echo "  2. Type and execute: whoami"  
echo "  3. Type and execute: date"
echo "  4. Press Up arrow multiple times to see previous commands"
echo "  5. Press Down arrow to move forward in history"
echo "  6. Start typing 'echo test', press Up, see it's saved"
echo "  7. Press Down to restore your typing"
echo ""
echo "Starting MyShell..."
echo "════════════════════════════════════════════════════════════"
echo ""

cd /home/sabyasachim/sabyasachi/MyShell
export BINPATH=/usr/bin:/bin
./mysh
