#include <functional>
#include <iostream>
#include <vector>

#include "coroutines.hpp"
#include "coroutines_api.hpp"
auto lol(int a) -> int {
  std::vector<int> multipliers;
  int b = a;
  Generator<int, int> nums =
      yield(static_cast<std::function<int(int)>>([](int a) { return a; }), 2, a, 1);
  for (int i = nums.start(); !nums.isEnd();) {
    if (b % i == 0) {
      multipliers.push_back(i);
      b /= i;
    } else {
      i = nums.next();
    }
  }
  coroutine_printf(1, "Множители числа %d:", a);
  for (int const multiplier : multipliers) {
    coroutine_printf(1, "%d ", multiplier);
  }
  coroutine_printf(1, "\n");
  return a + 1;
}

auto main() -> int {
  for (int i = 15; i < 20; i++) {
    new_coroutine(static_cast<std::function<int(int)>>(lol), i);
  }
  coroutines_dispatcher();
}