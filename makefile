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

