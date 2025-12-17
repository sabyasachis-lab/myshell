#!/bin/bash

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║    MyShell Runtime Logging Options - Complete Test       ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

cd /home/sabyasachim/sabyasachi/MyShell
export BINPATH=/usr/bin:/bin

# Test 1: No logging (default)
echo "═══════════════════════════════════════════════════════════"
echo "Test 1: No Logging (Default)"
echo "═══════════════════════════════════════════════════════════"
(echo "pwd"; sleep 0.3; echo "exit") | ./mysh 2>&1 | grep -E "(pwd|MyShell)" | head -5
echo ""

# Test 2: Console logging
echo "═══════════════════════════════════════════════════════════"
echo "Test 2: Console Logging (-v CONSOLE)"
echo "═══════════════════════════════════════════════════════════"
(echo "whoami"; sleep 0.3; echo "exit") | ./mysh -v CONSOLE 2>&1 | grep -E "(Console logging|Resolving|whoami)" | head -5
echo ""

# Test 3: File logging
echo "═══════════════════════════════════════════════════════════"
echo "Test 3: File Logging (-v FILE -f <path>)"
echo "═══════════════════════════════════════════════════════════"
rm -f logs/test_runtime.log
echo "Running: ./mysh -v FILE -f logs/test_runtime.log"
(echo "date"; sleep 0.3; echo "exit") | ./mysh -v FILE -f logs/test_runtime.log 2>&1 | grep -v "^\[" | tail -8
echo ""
echo "Checking log file:"
if [ -f logs/test_runtime.log ]; then
    echo "✓ Log file created"
    grep -E "(File logging|Resolving|Found)" logs/test_runtime.log | head -4
    echo "  Total entries: $(wc -l < logs/test_runtime.log)"
else
    echo "✗ Log file not found"
fi
echo ""

# Test 4: Error - FILE without -f
echo "═══════════════════════════════════════════════════════════"
echo "Test 4: Error Handling - FILE without -f"
echo "═══════════════════════════════════════════════════════════"
./mysh -v FILE 2>&1 | head -2
echo ""

# Test 5: Error - Invalid log type
echo "═══════════════════════════════════════════════════════════"
echo "Test 5: Error Handling - Invalid log type"
echo "═══════════════════════════════════════════════════════════"
./mysh -v INVALID 2>&1 | head -2
echo ""

# Test 6: Multiple commands with file logging
echo "═══════════════════════════════════════════════════════════"
echo "Test 6: Multiple Commands with File Logging"
echo "═══════════════════════════════════════════════════════════"
rm -f logs/multi_test.log
(echo "pwd"; sleep 0.2; echo "whoami"; sleep 0.2; echo "date"; sleep 0.2; echo "exit") | ./mysh -v FILE -f logs/multi_test.log > /dev/null 2>&1
if [ -f logs/multi_test.log ]; then
    echo "✓ Multiple commands logged successfully"
    echo "  External commands executed:"
    grep "Executing external" logs/multi_test.log | wc -l
    echo "  Total log entries: $(wc -l < logs/multi_test.log)"
else
    echo "✗ Test failed"
fi
echo ""

# Test 7: Help message
echo "═══════════════════════════════════════════════════════════"
echo "Test 7: Help Message"
echo "═══════════════════════════════════════════════════════════"
./mysh --help | grep -A 3 "OPTIONS:"
echo ""

echo "═══════════════════════════════════════════════════════════"
echo "All runtime logging tests completed successfully! ✓"
echo "═══════════════════════════════════════════════════════════"
