#include "coroutines.hpp"
#include "coroutines_api.hpp"
#include "coroutines_config.h"
#include <setjmp.h>
#include <list>
#include <functional>
#include <chrono>
#include <memory>
#include <queue>
#include <iostream>
#include <bitset>
#include <tuple>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

extern "C" {
extern void coroutine_wrapper(std::function<int(int)> func, int param, const int stacknum);
}
struct context {
    jmp_buf buf;
};

struct coroutine {
    struct context context;
    struct {
        std::chrono::microseconds running;
        std::chrono::microseconds runnable;
        std::chrono::microseconds blocked;
    } time;
    enum {
        RUNNING=2,
        RUNNABLE,
        BLOCKED,
        ZOMBIE
    } state;
};

std::list<struct coroutine> coroutines;
std::list<struct coroutine>::iterator current_coroutine;
std::queue<std::tuple<std::function<int(int)>, int>> coroutines_queue;
struct context dispatcher_context;
std::bitset<MAX_POOL_SIZE> used_stacks;
uint64_t completed_coroutins = 0;

void coroutines_dispatcher();
bool can_pop_queue();
void update_coroutines_times(std::chrono::microseconds duration);
void wait();

void await(std::function<int(int)> func, int param) {
    // current_coroutine->state = coroutine::BLOCKED;
    func(param);
}
extern "C"{
void coroutine_wrapper_(std::function<int(int)> func, int param, const int stacknum) {
    if(setjmp(dispatcher_context.buf) == 0) {
        func(param);
        current_coroutine->state = coroutine::ZOMBIE;
        used_stacks.reset(stacknum);
        longjmp(dispatcher_context.buf, 1);
    }
}
}
void wait() {
    if(setjmp(current_coroutine->context.buf) == 0) {
        longjmp(dispatcher_context.buf, 1);
    }
}
void update_coroutines_times(std::chrono::microseconds duration) {
    for(std::list<coroutine>::iterator it = coroutines.begin(); it != coroutines.end(); it++) {
        switch(it->state) {
            case coroutine::BLOCKED:
                it->time.blocked += duration;
                break;
            case coroutine::RUNNABLE:
                it->time.runnable += duration;
                break;
            case coroutine::RUNNING:
            case coroutine::ZOMBIE:
                it->time.running += duration;
                break;
        }
    }
}
void print_statistic() {
    /*Для диспетчера в целом
    1. Количество корутин в каждом из состояний
    2. Суммарное время пребывания всех корутин в каждом из состояний
    3. Количество завершенных корутин
    4. Распределение время создания корутины*/
    size_t counter[3]{};
    std::chrono::microseconds times[3]{};
    for(std::list<coroutine>::iterator it = coroutines.begin(); it != coroutines.end(); it++) {
        times[0] += it->time.blocked;
        times[1] += it->time.runnable;
        times[2] += it->time.running;
        switch(it->state) {
            case coroutine::BLOCKED:
                counter[0]++;
                break;
            case coroutine::RUNNABLE:
                counter[1]++;
                break;
            case coroutine::RUNNING:
            case coroutine::ZOMBIE:
                counter[2]++;
                break;
        }
    }
    std::cout<<"Количество корутин в каждом состоянии: "<<counter[0]<<" "<<counter[1]<<" "<<counter[2]<<std::endl;
    std::cout<<"Суммарное время пребывания корутин в каждом состоянии: "<<times[0].count()<<" "<<times[1].count()<<" "<<times[2].count()<<std::endl;
    std::cout<<"Количество завершенных корутин: "<<completed_coroutins<<std::endl;
}
void coroutines_dispatcher() {
    while(true) {
        while(!coroutines_queue.empty() && can_pop_queue()) {
            std::tuple<std::function<int(int)>, int> coroutine_start = coroutines_queue.front();
            coroutines_queue.pop();
            coroutines.emplace_back();
            current_coroutine = std::prev(coroutines.end());
            current_coroutine->state = coroutine::RUNNING;
            int stacknum;
            for(stacknum = 0;stacknum<MAX_POOL_SIZE;stacknum++)
                if(!used_stacks[stacknum])
                    break;
            used_stacks.set(stacknum);
            std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();
            coroutine_wrapper(std::get<0>(coroutine_start), std::get<1>(coroutine_start), stacknum);
            std::chrono::system_clock::time_point end = std::chrono::high_resolution_clock::now();
            update_coroutines_times(std::chrono::duration_cast<std::chrono::microseconds>(end-start));
            if(current_coroutine->state == coroutine::RUNNING)
                current_coroutine->state = coroutine::RUNNABLE;
        }
        /* Ищем корутину с наименьшим временем исполнения */
        current_coroutine = std::min_element(coroutines.begin(), coroutines.end(), [](auto a, auto b){return a.time.running.count() < b.time.running.count();});
        /* Исполняем */
        if(current_coroutine->state == coroutine::RUNNABLE || current_coroutine->state == coroutine::BLOCKED) {
            std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();
            if(setjmp(dispatcher_context.buf) == 0) {
                if(current_coroutine->state == coroutine::RUNNABLE)
                    current_coroutine->state = coroutine::RUNNING;
                longjmp(current_coroutine->context.buf, 1);
            }
            std::chrono::system_clock::time_point end = std::chrono::high_resolution_clock::now();
            update_coroutines_times(std::chrono::duration_cast<std::chrono::microseconds>(end-start));
            if(current_coroutine->state == coroutine::RUNNING)
                current_coroutine->state = coroutine::RUNNABLE;
        }
        if(current_coroutine->state == coroutine::ZOMBIE) {
            // std::cout<<"Корутина завершила работу"<<std::endl;
            // std::cout<<"Время running: "<<current_coroutine->time.running.count()<<" микросекунд"<<std::endl;
            // std::cout<<"Время runnable: "<<current_coroutine->time.runnable.count()<<" микросекунд"<<std::endl;
            // std::cout<<"Время blocked: "<<current_coroutine->time.blocked.count()<<" микросекунд"<<std::endl;
            current_coroutine = coroutines.erase(current_coroutine);
        }
        else
            current_coroutine = std::next(current_coroutine);
        // print_statistic();
        if(coroutines_queue.empty() && coroutines.empty())
            return;
    }
}

void new_coroutine(std::function<int(int)> func, int param) {
    coroutines_queue.push(std::make_tuple(func, param));
}

bool can_pop_queue() {
    return coroutines.size() < MAX_POOL_SIZE;
}

void coroutine_printf(int fd, const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    int len = vsprintf(buffer, format, args);
    va_end(args);  
    coroutine_write(fd, buffer, len);
}

void coroutine_write(int fd, void* buffer, size_t len) {
    current_coroutine->state = coroutine::BLOCKED;
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    char* pointer = (char*)buffer;
    while(pointer < (char*)buffer + len) {
        ssize_t w = write(fd, pointer, len - (pointer - (char*)buffer));
        if(w != -1)
            pointer += w;
        wait();
    }
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & (~O_NONBLOCK));
    current_coroutine->state = coroutine::RUNNING;
}

void coroutine_read(int fd, void* buffer, size_t len) {
    current_coroutine->state = coroutine::BLOCKED;
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    char* pointer = (char*)buffer;
    while(pointer < (char*)buffer + len) {
        ssize_t r = read(fd, pointer, len - (pointer - (char*)buffer));
        if(r != -1)
            pointer += r;
        if(r == 0)
            break;
        wait();
    }
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & (~O_NONBLOCK));
    current_coroutine->state = coroutine::RUNNING;
}