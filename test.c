#include <stdbool.h>
#include <stdio.h>

enum WeekDay {
  Sunday,
  Monday,
  Tuesday,
  Wednesday,
  Thursday,
  Friday,
  Saturday
}WeekDay;

struct Day {
}Day;

int main() {
  // print out the sizeof an int, float, char, and bool
  printf("Size of int: %lu\n", sizeof(int));
  printf("Size of float: %lu\n", sizeof(float));
  printf("Size of char: %lu\n", sizeof(char));
  printf("Size of bool: %lu\n", sizeof(bool));

  // print out the sizeof an enum, struct, and pointer
  printf("Size of enum: %lu\n", sizeof(WeekDay)); // should be 4
  printf("Size of struct: %lu\n", sizeof(Day)); // should be 16
}

