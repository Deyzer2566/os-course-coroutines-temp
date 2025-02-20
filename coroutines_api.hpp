#pragma once
#include <functional>
#include <cstdarg>
#include <cstdint>
#include "coroutines.hpp"
#include <iostream>
using namespace std;
template <class T, class R>
class Generator {
public:
    Generator(std::function<T(R)> function, R start, R finish, R step)
    : function(function)
    , start1(start)
    , finish(finish)
    , step(step)
    , current(start)
    {}
    T next() {
        T value = function(current);
        if(!isEnd()) {
            current += step;
        }
        wait();
        return value;
    }
    bool isEnd() {
        return current == finish;
    }
    R start() {
        return start1;
    }
private:
    R start1;
    R finish;
    R current;
    R step;
    std::function<T(R)> function;
};
template <class T, class R>
Generator<T,R> yield(std::function<T(R)> function, R start, R finish, R step) {
    return Generator<T,R>(function, start, finish, step);
}
void await(const std::function<int(int)>& function, int param);
void coroutine_printf(int fd, const char* format, ...);
void coroutine_write(int fd, void* buffer, size_t len);
void coroutine_read(int fd, void* buffer, size_t len);
