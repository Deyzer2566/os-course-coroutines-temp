.PHONY: run
CPP_FILES = \
	coroutines.cpp \
	main.cpp
ASM_FILES = \
	coroutine_wrapper.s
run: coroutines
	./coroutines
coroutines: $(CPP_FILES) $(ASM_FILES)
	g++ $(CPP_FILES) $(ASM_FILES) -o coroutines

check-style:
	find . -iname '*.h' -iname '*.hpp' -o -iname '*.cpp' \
	| xargs clang-tidy -p compile_commands.json

fix-style:
	find . -iname '*.h' -iname '*.hpp' -o -iname '*.cpp' \
	| xargs clang-tidy --fix-errors -p compile_commands.json

check-format:
	find . -iname '*.h' -iname '*.hpp' -o -iname '*.cpp' \
	| xargs clang-format -Werror --dry-run --fallback-style=Google --verbose

fix-format:
	find . -iname '*.h' -iname '*.hpp' -o -iname '*.cpp' \
	| xargs clang-format -i --fallback-style=Google --verbose
