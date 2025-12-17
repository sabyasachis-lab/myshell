#!/bin/bash

echo "=== Testing External Command Execution ==="
echo ""

# Set BINPATH
export BINPATH=/usr/bin:/bin

cd /home/sabyasachim/sabyasachi/MyShell

# Test 1: External command from BINPATH
echo "Test 1: Running 'ls -la' (from BINPATH)"
echo "ls -la" | timeout 2 ./mysh 2>&1 | grep -A 10 "MyShell"

echo ""
echo "---"
echo ""

# Test 2: Absolute path
echo "Test 2: Running '/bin/echo hello world' (absolute path)"
echo "/bin/echo hello world" | timeout 2 ./mysh 2>&1 | grep -A 5 "MyShell"

echo ""
echo "---"
echo ""

# Test 3: CWD execution
echo "Test 3: Creating test executable in CWD"
cat > /tmp/test_hello.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello from test executable!\n");
    return 0;
}
EOF
gcc /tmp/test_hello.c -o ./test_hello
echo "./test_hello" | timeout 2 ./mysh 2>&1 | grep -A 5 "MyShell"
rm -f ./test_hello

echo ""
echo "---"
echo ""

# Test 4: Command with arguments
echo "Test 4: Running 'echo test arguments' (from BINPATH)"
echo "echo test arguments" | timeout 2 ./mysh 2>&1 | grep -A 5 "MyShell"

echo ""
echo "=== Tests Complete ==="
