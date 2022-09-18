#include <stdio.h>
#include <string.h>

void legalCall(void) {
  // No forbidden function being called here
  puts("Hello world!");
}

void illegalCall(const char message[]) {
  char buffer[64];
  // `sprintf()` is dangerous
  sprintf(buffer, "Hello world! %s", message);
  puts(buffer);
}

void anotherIllegalCall(const char message[]) {
  char buffer[64] = "Hello world! ";
  // `strcat()` is dangerous
  strcat(buffer, message);
  puts(buffer);
}

int callWithoutDeclaration(int (*func)(int)) {
  // Calls to function pointers do not have an associated declaration
  return func(42);
}
