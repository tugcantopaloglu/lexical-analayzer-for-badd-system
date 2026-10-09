# Lexical Analyzer for Big Add Language

This C project analyzes a limited subset of the custom Big Add Language and writes a token and value report to `myscript.lx`. It was originally developed in Code::Blocks 20.03. It reports the first detected error and returns a nonzero status; source line and column locations are not implemented here.

## Build and run

Build a source copy with a C11 compiler. Both `main.c` copies are kept identical. The committed executable/object files and `.lx` files are historical artifacts, not regenerated build or test output.

```sh
gcc -std=c11 -Wall -Wextra "Only Source Codes/main.c" -o badd-analyzer
./badd-analyzer "Only Source Codes/custom-test.ba"
```

In a Visual Studio developer command prompt:

```bat
cl /std:c11 /W3 "Only Source Codes\main.c" /Fe:badd-analyzer.exe
badd-analyzer.exe "Only Source Codes\custom-test.ba"
```

Pass one `.ba` filename or run without arguments to enter a filename at the prompt. Paths can contain spaces. The report is written to **the current working directory**, replacing an existing `myscript.lx` there. Run from a scratch directory if you want to preserve a previous report. The process exits after analysis without waiting for another keypress.

## Implemented subset and limits

The parser handles period-terminated `int`, `move`, `add`, `sub`, integer/identifier `out`, single-statement counted loops and `{...}` comments. Identifiers must start with a letter and contain at most 20 alphanumeric or underscore characters. Numeric values use the compiler's C `int` range; out-of-range literals and arithmetic are rejected. This analyzer does not implement arbitrary-precision 100-digit arithmetic, quoted-string output, comma-separated output, `newline`, or general multiline loop blocks. Use the [later analyzer and interpreter](https://github.com/tugcantopaloglu/big-add-analyzer-interpreter) for the fuller language.

Input is limited to 1 MiB, 1,000 statements, 999 words per statement and 1,000 symbols. Reconstructed loop bodies must fit a 100-byte buffer. These limits produce errors instead of writing past buffers.

`myscript.ba` and `custom-test.ba` reference `myVar` after declaring `my_Var` and therefore fail at that identifier. `standart-test.ba` contains output syntax beyond this parser's implemented subset and is not a passing acceptance test.

## Regression checks

After compiling, run the standard-library Python test runner:

```sh
python3 tests/regression.py ./badd-analyzer
```

The tests generate reports in temporary directories and check arithmetic, loops, missing operands, invalid numbers, long comments, malformed input and capacity limits. They also verify the expected failures of the checked-in examples. CI builds with GCC and address/undefined-behavior sanitizers. The original screenshots below are retained as historical context.

**Tests:**

Test 1  
![Test 1](https://user-images.githubusercontent.com/53763911/138610202-b7aee3c1-7c5f-429d-9c8c-4da7cce7ec7c.png)  
Test 1 Output  
![Test 1 Output](https://user-images.githubusercontent.com/53763911/138610205-dc4d55cb-f3d7-45be-9b16-e4e8545fd0e3.png)  
Test 2  
![Test 2](https://user-images.githubusercontent.com/53763911/138610206-9c64628c-a966-4204-ac40-0b36cd26496f.png)  
Test 2 Output  
![Test 2 Output](https://user-images.githubusercontent.com/53763911/138610208-0db9a13c-a3c3-41da-a3a3-3bd384c165f8.png)  

**Check out this project second part (Lexical Analyzer with Interpreter):** https://github.com/tugcantopaloglu/big-add-analyzer-interpreter
