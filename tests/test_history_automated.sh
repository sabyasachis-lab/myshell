#!/bin/bash

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║   MyShell Command History - Automated Test               ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

cd /home/sabyasachim/sabyasachi/MyShell
export BINPATH=/usr/bin:/bin

# Test 1: Basic command execution and history
echo "Test 1: Execute multiple commands"
echo "───────────────────────────────────────────────────────────"
(sleep 0.1; printf "pwd\n"; sleep 0.3;
 printf "echo test1\n"; sleep 0.3;
 printf "echo test2\n"; sleep 0.3;
 printf "exit\n") | ./mysh 2>&1 | grep -E "(pwd|test1|test2|MyShell)" | head -8
echo ""

# Test 2: Use up arrow to recall previous command
echo "Test 2: Navigate history with up arrow"
echo "───────────────────────────────────────────────────────────"
echo "Commands: pwd, whoami, then Up arrow twice"
(sleep 0.1; printf "pwd\n"; sleep 0.2;
 printf "whoami\n"; sleep 0.2;
 printf "\033[A\033[A"; sleep 0.2;  # Up arrow twice
 printf "\n"; sleep 0.3;
 printf "exit\n") | ./mysh 2>&1 | grep -A 1 "pwd" | tail -4
echo ""

# Test 3: Test that current input is saved
echo "Test 3: Current input saved when browsing history"
echo "───────────────────────────────────────────────────────────"
echo "Type 'echo hello', press Up, then Down to restore"
(sleep 0.1; printf "pwd\n"; sleep 0.2;
 printf "echo hello"; sleep 0.1;  # Don't press Enter yet
 printf "\033[A"; sleep 0.1;      # Up arrow - shows 'pwd'
 printf "\033[B"; sleep 0.1;      # Down arrow - should restore 'echo hello'
 printf "\n"; sleep 0.3;          # Now execute
 printf "exit\n") | ./mysh 2>&1 | grep -A 1 "echo hello" | head -2
echo ""

# Test 4: Verify commands are stored
echo "Test 4: Multiple history navigation"
echo "───────────────────────────────────────────────────────────"
echo "Execute 3 commands, navigate up through all"
(sleep 0.1; 
 printf "echo first\n"; sleep 0.2;
 printf "echo second\n"; sleep 0.2;
 printf "echo third\n"; sleep 0.2;
 printf "\033[A\033[A"; sleep 0.1;  # Up twice to get 'echo second'
 printf "\n"; sleep 0.3;
 printf "exit\n") | ./mysh 2>&1 | grep -E "second" | head -2
echo ""

echo "═══════════════════════════════════════════════════════════"
echo "Basic history tests completed!"
echo "For interactive testing: ./test_history.sh"
echo "═══════════════════════════════════════════════════════════"
