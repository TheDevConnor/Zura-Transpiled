import unittest
import subprocess
import os

def run_test(code: str, expected_exit_code=None, expected_output=None):
    """Helper function to compile and run a Zura program, checking exit code and/or output."""
    # Check if release directory exists and a zura binary is present
    if not os.path.exists("release/zura"):
        raise FileNotFoundError("Zura binary not found. Please build the project in release mode first using './build.sh release'.")

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
        run_test("const main := fn () int { return 5; };", expected_exit_code=5)
    
    def test_return_variable(self):
        run_test("const main := fn () int { have a: int = 10; return a; };", expected_exit_code=10)
    
    def test_variable_reference(self):
        run_test("const main := fn () int { have a: int = 10; have b: int = 5; return a; };", expected_exit_code=10)
    
    def test_binary_operation(self):
        run_test("const main := fn () int { return 13 + 7; };", expected_exit_code=20)
    
    def test_binary_operation_with_variables(self):
        run_test("const main := fn () int { have a: int = 13; have b: int = 12; return a + b; };", expected_exit_code=25)
    
    def test_print_string_literal(self):
        run_test("const main := fn () int { @output(1, \"Hello, World!\"); return 0; };", expected_output="Hello, World!", expected_exit_code=0)
    
    def test_function_returning_parameter(self):
        run_test("const foo := fn (a: int, b: int) int { return b; }; const main := fn () int { return foo(83, 40); };", expected_exit_code=40)
    
    def test_for_loop(self):
        run_test("const main := fn () int { have a: int = 0; loop (i = 0; i < 10) : (i++) { a = a + 1; } return a; };", expected_exit_code=10)

    def test_while_loop(self):
        run_test("const main := fn () int { have a: int = 0; loop(a < 10) { a = a + 1; } return a; };", expected_exit_code=10)
    
    def test_struct_field_access(self):
        run_test("const a:=struct{x:int,};const main:=fn()int{have b:a={x:72};return b.x;};", expected_exit_code=72)
      
    def test_point_to_struct(self):
        run_test("const a:=struct{x:int,};const main:=fn()int{have b:a={x:72};have c:*a=&b;return c.x;};", expected_exit_code=72)

if __name__ == "__main__":
    unittest.main()
