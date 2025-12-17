#!/bin/bash

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║   MyShell Cursor Movement - Automated Verification       ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

cd /home/sabyasachim/sabyasachi/MyShell
export BINPATH=/usr/bin:/bin

# Test 1: Basic typing (no cursor movement)
echo "Test 1: Basic typing without cursor movement"
echo "───────────────────────────────────────────────────────────"
echo -n "Input: 'echo hello'"
(sleep 0.1; printf "echo hello\n"; sleep 0.3; printf "exit\n") | ./mysh 2>&1 | grep -A 1 "echo hello" | tail -2
echo ""

# Test 2: Type, move left, insert
echo "Test 2: Insert text in the middle"
echo "───────────────────────────────────────────────────────────"
echo "Input: Type 'helloworld', move left 5 times, insert ' '"
# h e l l o w o r l d
# Move left 5 times puts cursor between 'o' and 'w'
# Typing space should give "hello world"
(sleep 0.1; printf "helloworld"; sleep 0.1; 
 printf "\033[D\033[D\033[D\033[D\033[D"; sleep 0.1;  # 5 left arrows
 printf " "; sleep 0.1;  # Insert space
 printf "\n"; sleep 0.3; printf "exit\n") | ./mysh 2>&1 | grep -A 1 "hello world" | head -2
echo ""

# Test 3: Type at end
echo "Test 3: Verify we can execute commands normally"
echo "───────────────────────────────────────────────────────────"
(sleep 0.1; printf "pwd\n"; sleep 0.3; printf "exit\n") | ./mysh 2>&1 | grep -A 1 "pwd" | tail -2
echo ""

# Test 4: Cursor at beginning
echo "Test 4: Move to beginning and type"
echo "───────────────────────────────────────────────────────────"
echo "Input: Type 'world', move left 5 times, type 'hello '"
# Should result in "hello world"
(sleep 0.1; printf "world"; sleep 0.1;
 printf "\033[D\033[D\033[D\033[D\033[D"; sleep 0.1;  # Move to beginning
 printf "hello "; sleep 0.1;
 printf "\n"; sleep 0.3; printf "exit\n") | ./mysh 2>&1 | grep -A 1 "hello world" | head -2
echo ""

echo "═══════════════════════════════════════════════════════════"
echo "Automated tests completed!"
echo "═══════════════════════════════════════════════════════════"
echo ""
echo "To try interactive testing, run: ./test_cursor_movement.sh"
