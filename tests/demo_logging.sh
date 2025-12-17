#!/bin/bash

echo "════════════════════════════════════════════════════════════"
echo "  MyShell Runtime Logging - Quick Demo"
echo "════════════════════════════════════════════════════════════"
echo ""

cd /home/sabyasachim/sabyasachi/MyShell
export BINPATH=/usr/bin:/bin

echo "📝 Example 1: No logging (clean output)"
echo "Command: ./mysh"
echo "────────────────────────────────────────────────────────────"
(echo "pwd"; sleep 0.3; echo "exit") | timeout 2 ./mysh 2>&1 | tail -5
echo ""

echo "📝 Example 2: Console logging (debug to stderr)"
echo "Command: ./mysh -v CONSOLE"
echo "────────────────────────────────────────────────────────────"
(echo "whoami"; sleep 0.3; echo "exit") | timeout 2 ./mysh -v CONSOLE 2>&1 | grep -E "(Console|Token|whoami)" | head -4
echo "    ... (truncated for brevity)"
echo ""

echo "📝 Example 3: File logging (clean console, logs to file)"
echo "Command: ./mysh -v FILE -f demo.log"
echo "────────────────────────────────────────────────────────────"
rm -f demo.log
(echo "date"; sleep 0.3; echo "exit") | timeout 2 ./mysh -v FILE -f demo.log 2>&1 | tail -6
echo ""
echo "Log file contents:"
if [ -f demo.log ]; then
    echo "    $(wc -l < demo.log) lines written to demo.log"
    echo ""
    echo "Sample entries:"
    head -5 demo.log | sed 's/^/    /'
    echo "    ..."
    rm -f demo.log
else
    echo "    (log file not found)"
fi
echo ""

echo "════════════════════════════════════════════════════════════"
echo "✓ Demo complete! Run './mysh --help' for more information."
echo "════════════════════════════════════════════════════════════"
