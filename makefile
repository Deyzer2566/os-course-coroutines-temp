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
	{ find . -iname '*.cpp' ; find . -name '*.hpp'; find . -name '*.h'; } \
	| xargs clang-tidy -p compile_commands.json

fix-style:
	{ find . -iname '*.cpp' ; find . -name '*.hpp'; find . -name '*.h'; } \
	| xargs clang-tidy --fix-errors -p compile_commands.json

check-format:
	{ find . -iname '*.cpp' ; find . -name '*.hpp'; find . -name '*.h'; } \
	| xargs clang-format -Werror --dry-run --fallback-style=Google --verbose

fix-format:
	{ find . -iname '*.cpp' ; find . -name '*.hpp'; find . -name '*.h'; } \
	| xargs clang-format -i --fallback-style=Google --verbose
