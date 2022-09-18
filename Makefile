GCC_PLUGIN_PATH := $(shell ${CXX} -print-file-name=plugin)

CPPFLAGS += "-I${GCC_PLUGIN_PATH}/include"
CXXFLAGS ?= -std=c++14 -O2 -Wall -Wextra
CXXFLAGS += -fno-rtti -fPIC

.PHONY: all clean check install

all: forbidden_func_check.so

clean:
	${RM} forbidden_func_check.so *.o

check: forbidden_func_check.so
	test/test.sh

install: all
	install forbidden_func_check.so "${GCC_PLUGIN_PATH}"

forbidden_func_check.so: forbidden_func_check.o
	${CXX} -o $@ -shared ${CXXFLAGS} $^
