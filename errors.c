#include <stdio.h>

/* this is a valid comment */
int main() {
    int 2abc = 5;              // invalid identifier (starts with digit)
    int a = 12.3.4;            // invalid number (two decimal points)
    int oct = 09;              // invalid octal literal
    int hex = 0xZZ;            // invalid hex literal
    float f = 3e;              // malformed exponent
    int @name = 3;             // illegal character inside identifier
    int cost = 10 $ 5;         // invalid character
    whlie (cost > 0) {         // misspelled keyword
        cost--;
    }
    retrun 0;                  // misspelled keyword
    if (cost === 5) { }        // malformed operator
    char c = '';               // empty character literal
    char d = 'ab';             // too many characters
    char *s = "hello\q world"; // unknown escape sequence (warning)
    char *t = "unterminated;
    int ok = 10;
    /* this comment never ends
    int z = 1;
}
