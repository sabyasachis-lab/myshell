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

echo ""
echo "=== Testing Pipe with Output Redirection ==="

# Test 8: Simple pipe with redirection
echo "echo hello pipe | /bin/cat > pipe_out.txt
exit" | timeout 2 ../mysh > /dev/null 2>&1
if [ -f pipe_out.txt ] && grep -q "hello pipe" pipe_out.txt; then
    echo "✓ Test 8 passed: Simple pipe with > redirection"
    rm pipe_out.txt
else
    echo "✗ Test 8 failed: Simple pipe with > redirection"
fi

# Test 9: Multi-pipe with redirection
echo "echo one two three | /bin/tr ' ' '\n' | /bin/wc -l > count.txt
exit" | timeout 2 ../mysh > /dev/null 2>&1
if [ -f count.txt ] && [ "$(cat count.txt | tr -d ' ')" = "3" ]; then
    echo "✓ Test 9 passed: Multi-pipe with > redirection"
    rm count.txt
else
    echo "✗ Test 9 failed: Multi-pipe with > redirection"
    [ -f count.txt ] && rm count.txt
fi

# Test 10: Pipe with append redirection
echo "echo first line | /bin/cat > append.txt
echo second line | /bin/cat >> append.txt
exit" | timeout 2 ../mysh > /dev/null 2>&1
if [ -f append.txt ] && grep -q "first line" append.txt && grep -q "second line" append.txt; then
    echo "✓ Test 10 passed: Pipe with >> append redirection"
    rm append.txt
else
    echo "✗ Test 10 failed: Pipe with >> append redirection"
    [ -f append.txt ] && rm append.txt
fi

echo ""
echo "All pipe tests completed!"
