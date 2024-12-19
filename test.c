#include <stdio.h>
#include <stdbool.h>

enum TokenKind {
  IDENTIFIER,
  VARIABLE,
  EQUALS,
  SEMICOLON,

  _EOF,
};

struct Token {
  enum TokenKind kind;
  const char *value;
} Token;

void print(struct Token tk) {
  printf("Token: %d, %s\n", tk.kind, tk.value);
}

bool is_eof(struct Token tk) {
  return tk.kind == _EOF;
}

int main() {
  struct Token tokens[] = {
    {IDENTIFIER, "x"},
    {EQUALS, "="},
    {VARIABLE, "5"},
    {SEMICOLON, ";"},
    {_EOF, ""},
  };
  
  for (int i = 0; !is_eof(tokens[i]); i++) {
    print(tokens[i]);
  }

  return 0;
}

