#pragma once
#include <functional>
void new_coroutine(std::function<int(int)> function, int param);
void coroutines_dispatcher();
void wait();
