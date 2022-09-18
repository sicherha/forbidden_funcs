#!/bin/bash

set -e -u

BASEDIR=$(cd "$(dirname "$0")" && pwd)

GCC_CALL=(gcc -Wall -Werror -o /dev/null -c "$BASEDIR/test.c")
GCC_CALL_PLUGIN=("${GCC_CALL[@]}" \
  -fplugin="$BASEDIR/../forbidden_func_check.so")

# Sanity check: Call without plugin
"${GCC_CALL[@]}"

# Call to `sprintf()` should cause a compiler error
"${GCC_CALL_PLUGIN[@]}" -fplugin-arg-forbidden_func_check-list=sprintf && exit 1

# No call to `printf()` in the code -> no error
"${GCC_CALL_PLUGIN[@]}" -fplugin-arg-forbidden_func_check-list=printf

# Calls to `sprintf()` and `strcat()` should cause a compiler error
"${GCC_CALL_PLUGIN[@]}" -fplugin-arg-forbidden_func_check-list=sprintf,strcat \
  && exit 1

# `-Wno-error=deprecated` should cause the compilation not to fail
"${GCC_CALL_PLUGIN[@]}" -fplugin-arg-forbidden_func_check-list=sprintf,strcat \
  -Wno-error=deprecated

echo 'All tests successful.'
