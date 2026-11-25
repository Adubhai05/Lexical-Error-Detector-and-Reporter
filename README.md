# Lexical Error Detector and Reporter

A console-based lexical analyzer written in C++ that scans C source code,
breaks it into tokens and reports lexical errors with their exact location.
Developed as a Compiler Design course project.

## Features
- Analyzes a source file or code typed in the console
- Classifies tokens: keywords, identifiers, numbers, strings, characters,
  operators, punctuators, comments and preprocessor lines
- Shows line, column, source line and a `^~~~` marker for each problem
- Displays a token table and a summary of lines, tokens, errors and warnings

## Detects
- Identifiers starting with a digit (`2abc`)
- Invalid numbers (`12.3.4`, `09`, `0xZZ`, `3e`)
- Illegal characters (`@`, `$`)
- Invalid operators (`===`, `!==`, `=<`, `=>`)
- Empty or oversized character literals (`''`, `'ab'`)
- Unclosed strings, characters and block comments
- Keywords with swapped letters (`whlie`, `retrun`), shown as warnings

## Files
- `lexer.cpp`: source code of the analyzer
- `valid.c`: sample input without errors
- `errors.c`: sample input with errors

## How to Run
```bash
g++ -std=c++11 lexer.cpp -o lexer
./lexer
```
On Windows, run `lexer.exe`. Choose option 1 and enter `valid.c` or `errors.c`.

## Limitations
- Performs lexical analysis only, not syntax or semantic checking
- Does not validate escape sequences inside strings or characters
