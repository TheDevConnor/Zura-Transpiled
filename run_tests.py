import unittest
import subprocess
import os
import random
import string

def run_test(code: str, expected_exit_code=None, expected_output=None):
    """Helper function to compile and run a Zura program, checking exit code and/or output."""

    with open("zura_files/main.zu", "w") as f:
        f.write(code)
    
    subprocess.run(["./release/zura", "build", "zura_files/main.zu", "-name", "main", "-quiet"], check=True)
    
    result = subprocess.run("./main", capture_output=True, text=True)
    exit_code = result.returncode
    output = result.stdout.strip()
    
    if expected_exit_code is not None:
        assert exit_code == expected_exit_code, f"Expected exit code {expected_exit_code}, got {exit_code}"
    
    if expected_output is not None:
        assert output == expected_output, f"Expected output '{expected_output}', got '{output}'"

class TestZuraPrograms(unittest.TestCase):
    def test_return_constant(self):
        run_test("const main := fn () int! { return 5; };", expected_exit_code=5)
    
    def test_return_variable(self):
        run_test("const main := fn () int! { have a: int! = 10; return a; };", expected_exit_code=10)
    
    def test_variable_order(self):
        run_test("const main := fn () int! { have a: int! = 10; have b: int = 5; return a; };", expected_exit_code=10)
    
    def test_binary_operation_add(self):
        run_test("const main := fn () int! { return 13 + 7; };", expected_exit_code=20)
    
    def test_binary_operation_add_with_variables(self):
        run_test("const main := fn () int! { have a: int! = 13; have b: int = 12; return a + b; };", expected_exit_code=25)
    
    def test_binary_operation_mul(self):
        run_test("const main := fn () int! { return 13 * 7; };", expected_exit_code=91)
    
    def test_binary_operation_mul_with_variables(self):
        run_test("const main := fn () int! { have a: int! = 13; have b: int = 12; return a * b; };", expected_exit_code=156)
    
    # Compiler Optimizer Tests (All of these cases will be simplified by the compiler optimizer and we are testing its capabilities here)
    def test_binary_operation_oop(self):
        run_test("const main := fn () int! { return 18 - 7 * 4 / 2; };", expected_exit_code=4)

    def test_binary_signed_ints(self):
        run_test("const main := fn () int! { have x: int? = -18; have y: int? = -9; return x / y; };", expected_exit_code=2)

    def test_a_lot_of_operations(self):
        run_test("const main := fn () int! { have x: int! = 4; return x + 10 + 10 + 10 + 10 + 10 + 10 + 10 + 10 + 10 + 10 + 10 + 10; };", expected_exit_code=124)

    def test_add_to_itself(self):
        run_test("const main := fn () int! { have x: int! = 4; return x + x; };", expected_exit_code=8)

    def test_add_negatives(self):
        run_test("const main := fn () int! { have x: int? = -4; return 7 + x; };", expected_exit_code=3)
    
    def test_print_variable(self):
        run_test("const main := fn () int! { have a: int! = 10; @output(1, a); return 0; };", expected_output="10", expected_exit_code=0)
      
    def test_print_negative_variable(self):
        run_test("const main := fn () int! { have a: int? = -10; @output(1, a); return 0; };", expected_output="-10", expected_exit_code=0)
    
    def test_print_variable_and_string(self):
        run_test("const main := fn () int! { have a: int! = 10; @output(1, \"The number is: \", a); return 0; };", expected_output="The number is: 10", expected_exit_code=0)

    def test_print_string_literal(self):
        run_test("const main := fn () int! { @output(1, \"Hello, World!\"); return 0; };", expected_output="Hello, World!", expected_exit_code=0)
    
    # End of Compiler Optimizer Tests
    def test_function_returning_parameter(self):
        run_test("const foo := fn (a: int!, b: int!) int { return b; }; const main := fn () int! { return foo(83, 40); };", expected_exit_code=40)
    
    def test_for_loop(self):
        run_test("const main := fn () int! { have a: int! = 0; loop (i = 0; i < 10) : (i++) { a = a + 1; } return a; };", expected_exit_code=10)

    def test_while_loop(self):
        run_test("const main := fn () int! { have a: int! = 0; loop(a < 10) { a = a + 1; } return a; };", expected_exit_code=10)
    
    def test_struct_field_access(self):
        run_test("const a := struct { x: int!, }; const main := fn () int! { have b: a = { x: 72 }; return b.x; };", expected_exit_code=72)
      
    def test_struct_field_access_on_ptr(self):
        run_test("const a := struct { x: int!, }; const main := fn () int! { have b: a = { x: 72 }; have c: *a = &b; return c.x; };", expected_exit_code=72)

    def test_variable_equal_deref_struct(self):
        run_test("const a := struct { x: int!, }; const main := fn () int! { have b: a = { x: 72 }; have c: *a = &b; have d: a = c&; return d.x; };", expected_exit_code=72)

    def test_big_struct_field_access_on_ptr(self):
        run_test("const a := struct { x: int!, y: int!, z: int!, }; const main := fn () int! { have b: a = { x: 72, y: 12, z: 42 }; have c: *a = &b; return c.y + c.z; };", expected_exit_code=54)
    
    def test_variable_equal_deref_big_struct(self):
        run_test("const a := struct { x: int!, y: int!, z: int!, }; const main := fn () int! { have b: a = { x: 72, y: 12, z: 42 }; have c: *a = &b; have d: a = c&; return d.y + d.z; };", expected_exit_code=54)

    def test_factorial_function(self):
        run_test("const factorial := fn (n: int!) int! { have res: int! = 1; loop (i=2; i<=n) : (i++) { res = res * i; } return res; }; const main := fn () int! { @outputln(1, factorial(50)); return 0; };", expected_output="15188249005818642432")

    def test_unsigned_arrays(self):
        run_test("const main := fn () int! { have a: [5]int! = [1, 2, 3, 4, 5]; return a[2]; };", expected_exit_code=3)

    def test_signed_arrays(self):
        run_test("const main := fn () int! { have a: [5]int? = [-1, -2, -3, -4, -5]; return (a[2] * -1); };", expected_exit_code=3)

    def test_array_of_struct_multiple_fields(self):
        run_test("const k:=enum{a,b,c,d,e};const t:=struct{z:k,v:str};const main:=fn()int!{have l:[1]t=[{z:k.a,v:\"3.14159\"}];@output(1,l[0].v);return @cast<int!>(l[0].z);};", expected_output="3.14159", expected_exit_code=0)

    # no ! or ? symbol in the underlying type
    def test_inferred_signedness_arrays(self):
        run_test("const main := fn () int! { have a: [5]int = [1, 2, 3, 4, 5]; return a[2]; };", expected_exit_code=3)

    def test_array_of_structs(self):
        run_test("const a := struct { x: int!, y: int!, }; const main := fn () int! { have b: [2]a = [{ x: 1, y: 2 }, { x: 3, y: 4 }]; return b[1].x; };", expected_exit_code=3)
    
    def test_loop_over_array(self):
        run_test("const main := fn () int! { have a: [5]int! = [1, 2, 3, 4, 5]; have total: int! = 0; loop (i=0; i<5) : (i++) { total = total + a[i]; } return total; };", expected_exit_code=15)
   
    def test_useless_malloc(self):
        run_test("const main := fn () int! { have x: *void = @alloc(8); @free(x, 8); return 0; };", expected_exit_code=0)

    def test_read_file_char_arr(self):
        # create filenames of random characters until we find one not in use
        filename = "".join(random.choices(string.ascii_letters, k=8))
        while os.path.exists(f'test{filename}.txt'):
            filename = "".join(random.choices(string.ascii_letters, k=8))
        
        expected_output = ''.join(random.choices(string.ascii_letters, k=100))
        with open(f'test{filename}.txt', "w") as f:
            f.write(expected_output)
        
        run_test(f"const main := fn () int! {{ have chars: [100]char = [0]; have filepath: str = \"test{filename}.txt\"; have fd: int = @open(filepath, true, false, false); @input(fd, chars, 100); @close(fd); @output(1, chars); return 42; }};", expected_output=expected_output, expected_exit_code=42)
        # delete test file
        os.remove(f'test{filename}.txt') # NOTE: If test fails, the program will exit before this line is reached

    def test_read_file_malloc(self):
        # instead of using a [100]char, we will use a *char = @alloc(100)
        filename = "".join(random.SystemRandom().choices(string.ascii_letters, k=8))
        while os.path.exists(f'test{filename}.txt'):
            filename = "".join(random.SystemRandom().choices(string.ascii_letters, k=8))
        
        expected_output = ''.join(random.SystemRandom().choices(string.ascii_letters, k=100))
        with open(f'test{filename}.txt', "w") as f:
            f.write(expected_output)
        
        run_test(f"const main := fn () int! {{ have chars: *char = @cast<*char>(@alloc(100)); have filepath: str = \"test{filename}.txt\"; have fd: int = @open(filepath, true, false, false); @input(fd, chars, 100); @close(fd); @output(1, chars); @free(chars, 100); return 42; }};", expected_output=expected_output, expected_exit_code=42)
        os.remove(f'test{filename}.txt') # NOTE: If test fails, the program will exit before this line is reached

if __name__ == "__main__":
    # Run the build.sh script to build the project in release mode
    if not os.path.exists("./build.sh"):
        raise FileNotFoundError("Zura build script not found. In the worst case scenario, run the cmake --build command manually.")
    subprocess.run(["./build.sh", "release"], check=True)

    if not os.path.exists("./release/zura"):
        raise FileNotFoundError("Zura executable not found after building. Check the build script output for errors.")
    unittest.main()
