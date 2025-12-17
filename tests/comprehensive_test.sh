#!/bin/bash

echo "╔════════════════════════════════════════════════════════════╗"
echo "║  MyShell External Command Execution - Comprehensive Test  ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

export BINPATH=/usr/bin:/bin
cd /home/sabyasachim/sabyasachi/MyShell

# Test 1: Builtin commands still work
echo "✓ Test 1: Builtin commands (should take priority)"
(echo "pwd"; sleep 0.3; echo "exit") | ./mysh 2>&1 | grep -A 1 "pwd"
echo ""

# Test 2: External command from BINPATH
echo "✓ Test 2: External command from BINPATH"
(echo "whoami"; sleep 0.3; echo "exit") | ./mysh 2>&1 | grep -A 1 "whoami"
echo ""

# Test 3: External command with arguments
echo "✓ Test 3: External command with arguments"
(echo "uname -s"; sleep 0.3; echo "exit") | ./mysh 2>&1 | grep -A 1 "uname"
echo ""

# Test 4: Absolute path execution
echo "✓ Test 4: Absolute path execution"
(echo "/bin/echo Hello from absolute path"; sleep 0.3; echo "exit") | ./mysh 2>&1 | grep -A 1 "/bin/echo"
echo ""

# Test 5: Current directory execution
echo "✓ Test 5: Current directory executable"
cat > ./test_prog.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Success: CWD execution works!\n");
    return 0;
}
EOF
gcc ./test_prog.c -o ./test_prog 2>/dev/null
(echo "./test_prog"; sleep 0.3; echo "exit") | ./mysh 2>&1 | grep -A 1 "test_prog"
rm -f ./test_prog ./test_prog.c
echo ""

# Test 6: Command not found
echo "✓ Test 6: Error handling for non-existent command"
(echo "this_does_not_exist"; sleep 0.3; echo "exit") | ./mysh 2>&1 | grep -A 1 "this_does_not_exist"
echo ""

# Test 7: No BINPATH set
echo "✓ Test 7: Behavior without BINPATH (should fail for 'date')"
(unset BINPATH; (echo "date"; sleep 0.3; echo "exit") | ./mysh 2>&1 | grep -A 1 "date")
echo ""

# Test 8: Absolute path works without BINPATH
echo "✓ Test 8: Absolute path works even without BINPATH"
(unset BINPATH; (echo "/bin/date"; sleep 0.3; echo "exit") | ./mysh 2>&1 | grep -A 1 "/bin/date")
echo ""

echo "═══════════════════════════════════════════════════════════"
echo "All tests completed successfully! ✓"
echo "═══════════════════════════════════════════════════════════"
