#include "mutils.h"

Timeit::Timeit()
{
    Timeit::start = std::chrono::high_resolution_clock::now();
}

void Timeit::print()
{
    std::chrono::duration<double, std::milli> ms_double = std::chrono::high_resolution_clock::now() - Timeit::start;
    std::cout << "Elapsed time: " << ms_double.count() << " ms\n";
}

double Timeit::ms()
{
    std::chrono::duration<double, std::milli> ms_double = std::chrono::high_resolution_clock::now() - Timeit::start;
    return ms_double.count();
}