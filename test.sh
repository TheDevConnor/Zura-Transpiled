#!/usr/bin/env bash

# Always assume release mode

# Update code before testing
./build.sh release

all_pass=1

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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
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
    all_pass=0
    test12_passed=0
fi

# test 13: For loop with variable
expected_code=10
echo "const main := fn () int { have a: int = 0; loop (i = 0; i < 10) : (i++) { a = a + 1; } return a; };" > zura_files/main.zu
test13_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 13 failed, program expected $expected_code with exit code $exit_code"
    all_pass=0
    test13_passed=0
fi

# test 14: dis on a int variable
expected_output="10"
expected_code=0 # expect success
echo "const main := fn () int { have a: int = 10; dis(a); return 0; };" > zura_files/main.zu
test14_passed=1
./release/zura build zura_files/main.zu -name main -quiet
output=$(./main)
exit_code=$?
if [ "$output" != "$expected_output" ]; then
    echo "Test 14 failed, program expected '$expected_output' with output '$output'"
    all_pass=0
    test14_passed=0
fi
if [ $exit_code -ne $expected_code ]; then
    echo "Test 14 failed, program expected $expected_code with exit code $exit_code"
    all_pass=0
    test14_passed=0
fi

# test 15: dis on a int literal
expected_output="12"
expected_code=1
echo "const main := fn () int { dis(12); return 1; };" > zura_files/main.zu
test15_passed=1
./release/zura build zura_files/main.zu -name main -quiet
output=$(./main)
exit_code=$?
if [ "$output" != "$expected_output" ]; then
    echo "Test 15 failed, program expected '$expected_output' with output '$output'"
    all_pass=0
    test15_passed=0
fi
if [ $exit_code -ne $expected_code ]; then
    echo "Test 15 failed, program expected $expected_code with exit code $exit_code"
    all_pass=0
    test15_passed=0
fi

# test 16: struct with 1 field
expected_code=72
echo "const a:=struct{x:int;};const main:=fn()int{have b:a={x:72};return b.x;};" > zura_files/main.zu
test16_passed=1
./release/zura build zura_files/main.zu -name main -quiet
./main
exit_code=$?
if [ $exit_code -ne $expected_code ]; then
    echo "Test 16 failed, program expected $expected_code with exit code $exit_code"
    all_pass=0
    test16_passed=0
fi


if [ $all_pass -eq 1 ]; then
    echo "All tests passed" && exit 0
else
    echo "Some tests failed..."
fi

# Now that we are done, go over each test**_passed and print results
if [ $test1_passed -eq 1 ]; then
    echo "Test 1 passed (exit code)"
fi

if [ $test2_passed -eq 1 ]; then
    echo "Test 2 passed (variable)"
fi

if [ $test3_passed -eq 1 ]; then
    echo "Test 3 passed (variable reference)"
fi

if [ $test4_passed -eq 1 ]; then
    echo "Test 4 passed (binary operation)"
fi

if [ $test5_passed -eq 1 ]; then
    echo "Test 5 passed (binary operation with variables)"
fi

if [ $test6_passed -eq 1 ]; then
    echo "Test 6 passed (binary operation with variable and literal)"
fi

if [ $test7_passed -eq 1 ]; then
    echo "Test 7 passed (dis with string literal)"
fi

if [ $test8_passed -eq 1 ]; then
    echo "Test 8 passe (dis with string variable)"
fi

if [ $test9_passed -eq 1 ]; then
    echo "Test 9 passed (function returning constant)"
fi

if [ $test10_passed -eq 1 ]; then
    echo "Test 10 passed (function returning parameter)"
fi

if [ $test11_passed -eq 1 ]; then
    echo "Test 11 passed (function doing operation on parameters)"
fi

if [ $test12_passed -eq 1 ]; then
    echo "Test 12 passed (complex binary expressions)"
fi

if [ $test13_passed -eq 1 ]; then
    echo "Test 13 passed (for loop with variable)"
fi

if [ $test14_passed -eq 1 ]; then
    echo "Test 14 passed (dis on int variable)"
fi

if [ $test15_passed -eq 1 ]; then
    echo "Test 15 passed (dis on int literal)"
fi

if [ $test16_passed -eq 1 ]; then
    echo "Test 16 passed (struct with 1 field)"
fi

rm ./main