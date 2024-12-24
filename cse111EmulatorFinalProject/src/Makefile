SOURCES := main.cpp cpu.cpp cpu.h console.cpp console.h gpu.cpp gpu.h duck.h

format:
	clang-format ${SOURCES} -i --style=Google

verify-format:
	clang-format ${SOURCES} --dry-run --Werror --style=Google