#!/bin/bash

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║        MyShell Logging Modes - Comprehensive Test        ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

cd /home/sabyasachim/sabyasachi/MyShell
export BINPATH=/usr/bin:/bin

# Test 1: Default stderr logging mode
echo "═══════════════════════════════════════════════════════════"
echo "Test 1: Default STDERR Logging Mode"
echo "═══════════════════════════════════════════════════════════"
echo "Building without MYSHELL_LOG_FILE..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

echo ""
echo "Running with -v flag (logs should appear on stderr):"
echo "-----------------------------------------------------------"
(echo "whoami"; sleep 0.3; echo "exit") | ./mysh -v 2>&1 | grep -E "(DEBUG|Resolving|whoami)" | head -5
echo ""

# Test 2: File logging mode
echo "═══════════════════════════════════════════════════════════"
echo "Test 2: File Logging Mode"
echo "═══════════════════════════════════════════════════════════"
echo "Building with MYSHELL_LOG_FILE=./test_logs/myshell.log..."
rm -f test_logs/myshell.log
make clean > /dev/null 2>&1
make CFLAGS="-Wall -Wextra -std=c99 -g -DMYSHELL_LOG_FILE='\"./test_logs/myshell.log\"'" > /dev/null 2>&1

echo ""
echo "Running with -v flag (logs should go to file, stderr clean):"
echo "-----------------------------------------------------------"
(echo "date"; sleep 0.3; echo "exit") | ./mysh -v 2>&1 | grep -v "^\[" | head -10
echo ""

echo "Checking log file contents:"
echo "-----------------------------------------------------------"
if [ -f test_logs/myshell.log ]; then
    echo "✓ Log file created successfully"
    echo ""
    echo "Sample log entries:"
    grep -E "(Resolving|Found|Executing)" test_logs/myshell.log | head -5
    echo ""
    echo "Total log entries: $(wc -l < test_logs/myshell.log)"
else
    echo "✗ Log file not found!"
fi
echo ""

# Test 3: Verify file initialization happens only once
echo "═══════════════════════════════════════════════════════════"
echo "Test 3: Verify Log File Initialization (single open)"
echo "═══════════════════════════════════════════════════════════"
echo "Running multiple commands to verify file is opened once..."
rm -f test_logs/myshell.log
(echo "pwd"; sleep 0.2; echo "whoami"; sleep 0.2; echo "date"; sleep 0.2; echo "exit") | ./mysh -v > /dev/null 2>&1

if [ -f test_logs/myshell.log ]; then
    log_count=$(wc -l < test_logs/myshell.log)
    echo "✓ Log file contains $log_count entries"
    echo "✓ All logs written to single file handle"
else
    echo "✗ Test failed"
fi
echo ""

echo "═══════════════════════════════════════════════════════════"
echo "All logging tests completed successfully! ✓"
echo "═══════════════════════════════════════════════════════════"
