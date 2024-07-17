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

GCC_PLUGIN_PATH := $(shell ${CXX} -print-file-name=plugin)

CPPFLAGS += "-I${GCC_PLUGIN_PATH}/include"
CXXFLAGS ?= -O2 -Wall -Wextra
CXXFLAGS += -std=c++14 -fno-rtti -fPIC \
            -fvisibility=hidden -fvisibility-inlines-hidden
LDFLAGS += -Wl,--as-needed

.PHONY: all clean format check install

all: forbidden_funcs.so

clean:
	${RM} forbidden_funcs.so *.o

format:
	clang-format -i *.cc test/*.c

check: forbidden_funcs.so
	test/test.sh

install: all
	mkdir -p "${DESTDIR}${GCC_PLUGIN_PATH}"
	install forbidden_funcs.so "${DESTDIR}${GCC_PLUGIN_PATH}"

forbidden_funcs.so: forbidden_funcs.o
	${CXX} -o $@ -shared ${CXXFLAGS} ${LDFLAGS} $^
