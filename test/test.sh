#!/bin/bash

# forbidden_funcs - a GCC plugin checking for calls to forbidden functions
# Copyright (C) 2022  Christoph Erhardt
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

set -e -u

BASEDIR=$(cd "$(dirname "$0")" && pwd)

GCC_CALL=(gcc -Wall -Werror -o /dev/null -c "$BASEDIR/test.c")
GCC_CALL_PLUGIN=("${GCC_CALL[@]}" \
  -fplugin="$BASEDIR/../forbidden_funcs.so")

# Sanity check: Call without plugin
"${GCC_CALL[@]}"

# Call to `sprintf()` should cause a compiler error
"${GCC_CALL_PLUGIN[@]}" -fplugin-arg-forbidden_funcs-list=sprintf 2> /dev/null \
  && (echo 'Expected compiler error for sprintf'; exit 1)

# No call to `printf()` in the code -> no error
"${GCC_CALL_PLUGIN[@]}" -fplugin-arg-forbidden_funcs-list=printf

# Calls to `sprintf()` and `strcat()` should cause a compiler error
"${GCC_CALL_PLUGIN[@]}" \
  -fplugin-arg-forbidden_funcs-list=sprintf,strcat 2> /dev/null \
  && (echo 'Expected compiler error for sprintf/strcat'; exit 1)

# Taking the address of `system()`should cause a compiler error
"${GCC_CALL_PLUGIN[@]}" \
  -fplugin-arg-forbidden_funcs-list=system 2> /dev/null \
  && (echo 'Expected compiler error for system'; exit 1)

# `-Wno-error=deprecated` should cause the compilation not to fail
"${GCC_CALL_PLUGIN[@]}" \
  -fplugin-arg-forbidden_funcs-list=sprintf,strcat -Wno-error=deprecated \
  2> /dev/null

echo 'All tests successful.'
