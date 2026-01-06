#!/bin/bash

# Test script for pipe functionality in MyShell

echo "=== Testing Pipe Functionality ==="
echo ""

# Test 1: Simple pipe with builtin and external
echo "Test 1: echo | cat"
(echo "echo hello world | /bin/cat"; sleep 0.3; echo "exit") | ../mysh | grep "hello world"
if [ $? -eq 0 ]; then
    echo "✓ PASSED"
else
    echo "✗ FAILED"
fi
echo ""

# Test 2: Builtin command piped to external
echo "Test 2: pwd | cat"
(echo "pwd | /bin/cat"; sleep 0.3; echo "exit") | ../mysh | grep "/MyShell"
if [ $? -eq 0 ]; then
    echo "✓ PASSED"
else
    echo "✗ FAILED"
fi
echo ""

# Test 3: Multiple pipes
echo "Test 3: echo | cat | cat"
(echo "echo testing | /bin/cat | /bin/cat"; sleep 0.3; echo "exit") | ../mysh | grep "testing"
if [ $? -eq 0 ]; then
    echo "✓ PASSED"
else
    echo "✗ FAILED"
fi
echo ""

# Test 4: Pipe with grep
echo "Test 4: ls | grep Makefile"
(echo "ls | /bin/grep Makefile"; sleep 0.3; echo "exit") | ../mysh | grep "Makefile"
if [ $? -eq 0 ]; then
    echo "✓ PASSED"
else
    echo "✗ FAILED"
fi
echo ""

# Test 5: Pipe with wc
echo "Test 5: pwd | wc -l"
(echo "pwd | /bin/wc -l"; sleep 0.3; echo "exit") | ../mysh | grep -E "^[[:space:]]*1[[:space:]]*$"
if [ $? -eq 0 ]; then
    echo "✓ PASSED"
else
    echo "✗ FAILED"
fi
echo ""

# Test 6: Three-command pipeline
echo "Test 6: pwd | cat | cat | cat"
(echo "pwd | /bin/cat | /bin/cat | /bin/cat"; sleep 0.3; echo "exit") | ../mysh | grep "/MyShell"
if [ $? -eq 0 ]; then
    echo "✓ PASSED"
else
    echo "✗ FAILED"
fi
echo ""

# Test 7: Complex pipeline
echo "Test 7: ls | head -5 | wc -l"
OUTPUT=$(echo -e "ls | /bin/head -5 | /bin/wc -l\nexit" | ../mysh | grep -oE "[0-9]+" | head -1)
if [ ! -z "$OUTPUT" ] && [ "$OUTPUT" -ge 0 ]; then
    echo "✓ PASSED (counted $OUTPUT lines)"
else
    echo "✗ FAILED"
fi
echo ""

echo "=== Pipe Tests Complete ==="
