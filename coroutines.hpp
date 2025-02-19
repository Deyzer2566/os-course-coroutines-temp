#pragma once
#include <functional>
void new_coroutine(std::function<int(int)> func, int param);
void coroutines_dispatcher();
void wait();
