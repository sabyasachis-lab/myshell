#!/bin/bash

echo "Testing pipe with output redirection..."

# Test 1: Simple pipe with redirection
echo "echo test output | /bin/cat > test_out1.txt" | timeout 2 ../mysh
if [ -f test_out1.txt ] && grep -q "test output" test_out1.txt; then
    echo "✓ Test 1 passed: echo | cat > file"
else
    echo "✗ Test 1 failed"
fi

# Test 2: Multi-pipe with redirection
echo "pwd | /bin/cat | /bin/cat > test_out2.txt" | timeout 2 ../mysh
if [ -f test_out2.txt ] && [ -s test_out2.txt ]; then
    echo "✓ Test 2 passed: pwd | cat | cat > file"
else
    echo "✗ Test 2 failed"
fi

# Test 3: Append mode
echo "echo line1 > test_out3.txt" | timeout 2 ../mysh
echo "echo line2 >> test_out3.txt" | timeout 2 ../mysh
if [ -f test_out3.txt ] && [ "$(wc -l < test_out3.txt)" -eq 2 ]; then
    echo "✓ Test 3 passed: append mode >>"
else
    echo "✗ Test 3 failed"
fi

# Test 4: Pipe with append
echo "echo first | /bin/cat > test_out4.txt" | timeout 2 ../mysh
echo "echo second | /bin/cat >> test_out4.txt" | timeout 2 ../mysh
if [ -f test_out4.txt ] && grep -q "first" test_out4.txt && grep -q "second" test_out4.txt; then
    echo "✓ Test 4 passed: pipe with append"
else
    echo "✗ Test 4 failed"
fi

# Cleanup
rm -f test_out*.txt

echo "All tests completed!"
