#!/usr/bin/env bash

# Always assume release mode

# Update code before testing
./build.sh release

exit_code=$?
expected_code=5
echo "const main := fn () int { return 5; };" > zura_files/main.zu
test1_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?

if [ $exit_code -ne $expected_code ]; then
    echo "Test 1 failed, program expected $expected_code with exit code $exit_code"
    test1_passed=0
fi

# Test 2: Expect returned variable 19
expected_code=10
echo "const main := fn () int { have a: int = 10; return a; };" > zura_files/main.zu
test2_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 2 failed, program expected $expected_code with exit code $exit_code"
    test2_passed=0
fi

# Test 3: Use 2 variable references
expected_code=10
echo "const main := fn () int { have a: int = 10; have b: int = 5; return a; };" > zura_files/main.zu
test3_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 3 failed, program expected $expected_code with exit code $exit_code"
    test3_passed=0
fi

# Test 4: Binary operations on literals
expected_code=20
echo "const main := fn () int { return 13 + 7; };" > zura_files/main.zu
test4_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 4 failed, program expected $expected_code with exit code $exit_code"
    test4_passed=0
fi

# Test 5: Binary operations on variables
expected_code=25
echo "const main := fn () int { have a: int = 13; have b: int = 12; return a + b; };" > zura_files/main.zu
test5_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 5 failed, program expected $expected_code with exit code $exit_code"
    test5_passed=0
fi

# Test 6: Binary operations on variables and literals
expected_code=30
echo "const main := fn () int { have a: int = 13; return a + 17; };" > zura_files/main.zu
test6_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 6 failed, program expected $expected_code with exit code $exit_code"
    test6_passed=0
fi

# Test 7: Print a string literal using dis
expected_output="Hello, World!"
echo "const main := fn () int { dis(\"$expected_output\"); return 0; };" > zura_files/main.zu
test7_passed=1
./release/zura build zura_files/main.zu -name main -quiet
output=$(./main)
if [ "$output" != "$expected_output" ]; then
    echo "Test 7 failed, program expected '$expected_output' with output '$output'"
    test7_passed=0
fi

# Test 8: Print a string variable from dis
expected_output="Goodbye, World!"
echo "const main := fn () int { have a: int = 14; have b: str = \"$expected_output\"; dis(b); return 0; };" > zura_files/main.zu
test8_passed=1
./release/zura build zura_files/main.zu -name main -quiet
output=$(./main)
if [ "$output" != "$expected_output" ]; then
    echo "Test 8 failed, program expected '$expected_output' with output '$output'"
    test8_passed=0
fi

# Test 9: Basic function to return constant
expected_code=35
echo "const foo := fn () int { return 35; }; const main := fn () int { return foo(); };" > zura_files/main.zu
test9_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 9 failed, program expected $expected_code with exit code $exit_code"
    test9_passed=0
fi

# Test 10: Function returns parameter
expected_code=40
echo "const foo := fn (a: int, b: int) int { return b; }; const main := fn () int { return foo(83, 40); };" > zura_files/main.zu
test10_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 10 failed, program expected $expected_code with exit code $exit_code"
    test10_passed=0
fi

# Test 11: Function does operation on parameters
expected_code=45
echo "const foo := fn (a: int, b: int) int { return a + b; }; const main := fn () int { return foo(20, 25); };" > zura_files/main.zu
test11_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 11 failed, program expected $expected_code with exit code $exit_code"
    test11_passed=0
fi

# Test 12: Complex binary expressions
expected_code=28
echo "const main := fn () int { return 4 + 3 * 8; };" > zura_files/main.zu
test12_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 12 failed, program expected $expected_code with exit code $exit_code"
    test12_passed=0
fi

# Now that we are done, go over each test**_passed and print results
if [ $test1_passed -eq 1 ]; then
    echo "Test 1 passed"
fi

if [ $test2_passed -eq 1 ]; then
    echo "Test 2 passed"
fi

if [ $test3_passed -eq 1 ]; then
    echo "Test 3 passed"
fi

if [ $test4_passed -eq 1 ]; then
    echo "Test 4 passed"
fi

if [ $test5_passed -eq 1 ]; then
    echo "Test 5 passed"
fi

if [ $test6_passed -eq 1 ]; then
    echo "Test 6 passed"
fi

if [ $test7_passed -eq 1 ]; then
    echo "Test 7 passed"
fi

if [ $test8_passed -eq 1 ]; then
    echo "Test 8 passed"
fi

if [ $test9_passed -eq 1 ]; then
    echo "Test 9 passed"
fi

if [ $test10_passed -eq 1 ]; then
    echo "Test 10 passed"
fi

if [ $test11_passed -eq 1 ]; then
    echo "Test 11 passed"
fi

if [ $test12_passed -eq 1 ]; then
    echo "Test 12 passed"
fi

rm ./main