// forbidden_func_check - a GCC plugin checking for calls to forbidden functions
// Copyright (C) 2022  Christoph Erhardt
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
