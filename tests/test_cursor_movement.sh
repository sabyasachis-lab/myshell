#!/bin/bash

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║     MyShell Cursor Movement - Interactive Demo           ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""
echo "This demo showcases the new cursor movement features:"
echo "  • Left/Right arrow keys to move cursor"
echo "  • Insert text at cursor position"
echo "  • Backspace works at any cursor position"
echo "  • Text shifts properly during editing"
echo ""
echo "Starting MyShell... Try the following:"
echo "  1. Type: 'hello world'"
echo "  2. Press Left arrow several times"
echo "  3. Type some text in the middle"
echo "  4. Use Right arrow to move forward"
echo "  5. Use Backspace to delete at any position"
echo "  6. Press Enter to execute the complete line"
echo ""
echo "Press Ctrl+C when done testing, then type 'exit'"
echo "════════════════════════════════════════════════════════════"
echo ""

cd /home/sabyasachim/sabyasachi/MyShell
export BINPATH=/usr/bin:/bin
./mysh
