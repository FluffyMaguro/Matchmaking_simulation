#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <map>
#include <unordered_map>
#include <array>

// Functions for creating a string from multiple variables and priting multiple variables
// Doesn't work on arrays correctly (as they decay to pointers when passed to a function).

// CREATING STRINGS

template <typename A>
std::string str(const A &a)
{
    std::ostringstream ss;
    ss << a;
    return ss.str();
}

inline std::string str(const std::string &s)
{
    return s;
}

template <typename T>
std::string str(const std::vector<T> &v)
{
    if (v.size() == 0)
        return "[]";

    std::string s = "[";
    for (const T &el : v)
        s += str(el) + ", ";

    s.erase(s.end() - 2, s.end());
    s += "]";
    return s;
}

template <typename T, std::size_t N>
std::string str(const std::array<T, N> &a)
{
    if (N == 0)
        return "[]";

    std::string s = "[";
    for (const T &el : a)
        s += str(el) + ", ";

    s.erase(s.end() - 2, s.end());
    s += "]";
    return s;
}

template <typename A, typename B>
std::string str(const std::unordered_map<A, B> &umap)
{
    std::string s = "{";
    for (auto &el : umap)
        s += str(el.first) + ": " + str(el.second) + ", ";

    s.erase(s.end() - 2, s.end());
    s += "}";
    return s;
}

template <typename A, typename B>
std::string str(const std::map<A, B> &map)
{
    std::string s = "{";
    for (auto &el : map)
        s += str(el.first) + ": " + str(el.second) + ", ";

    s.erase(s.end() - 2, s.end());
    s += "}";
    return s;
}

inline std::string str()
{
    return "";
}

template <typename T, typename... Types>
std::string str(const T var1, const Types... var2)
{
    std::string result;
    result += str(var1);
    result += str(var2...);
    return result;
}

// PRINTING

template <typename A>
void _print(const A a)
{
    std::cout << a << ' ';
}

template <typename A>
void _print(const std::vector<A> &v)
{
    std::cout << str(v) << ' ';
}

template <typename A, typename B>
void _print(const std::map<A, B> &map)
{
    std::cout << str(map) << ' ';
}

template <typename A, typename B>
void _print(const std::unordered_map<A, B> &map)
{
    std::cout << str(map) << ' ';
}

template <typename A, std::size_t N>
void _print(const std::array<A, N> &a)
{
    std::cout << str(a) << ' ';
}

inline void print()
{
    std::cout << std::endl;
}

template <typename T, typename... Types>
void print(const T var1, const Types... var2)
{
    _print(var1);
    print(var2...);
}

// LOGGING

template <typename A>
void log(const A a, std::string filename = "log.txt")
{
    std::string s = str(a);
    std::cout << s << std::endl;
    std::ofstream myfile(filename, std::ios::out | std::ios::app);
    if (myfile.is_open())
    {
        myfile << s << std::endl;
        myfile.close();
    }
    else
        std::cout << "Can't save to the file. Won't open.\n";
}

// TIMER

class Timeit
{
    std::chrono::high_resolution_clock::time_point start;
    bool scoped = false;

public:
    Timeit(bool scoped = false)
    {
        start = std::chrono::high_resolution_clock::now();
        this->scoped = scoped;
    }

    ~Timeit()
    {
        if (scoped)
            print();
    }

    void reset()
    {
        start = std::chrono::high_resolution_clock::now();
    }

    void print()
    {
        std::chrono::duration<double, std::milli> ms_double = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Elapsed time: " << ms_double.count() << " ms\n";
    }
    double ms()
    {
        std::chrono::duration<double, std::milli> ms_double = std::chrono::high_resolution_clock::now() - start;
        return ms_double.count();
    }

    double s()
    {
        return ms() / 1000;
    }
};
