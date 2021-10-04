#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <map>
#include <unordered_map>

// Functions for creating a string from multiple variables and priting multiple variables
// Doesn't work on arrays correctly (as they decay to pointers when passed to a function).
// Templated functions need to have implementation passed in a header file as they are
// created and compiled as needed by each CPP file. They do not conflict even with the same arguments.

template <typename A>
std::string str(A a)
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
std::string str(const std::vector<T> &a)
{
    if (a.size() == 0)
        return "[]";

    std::string s = "[";
    for (const T &el : a)
        s += str(el) + ", ";

    s.erase(s.end() - 2, s.end());
    s += "]";
    return s;
}

template <typename A, typename B>
std::string str(std::unordered_map<A, B> &umap)
{
    std::string s = "{";
    for (auto &el : umap)
        s += str(el.first) + ": " + str(el.second) + ", ";

    s.erase(s.end() - 2, s.end());
    s += "}";
    return s;
}

template <typename A, typename B>
std::string str(std::map<A, B> &map)
{
    std::string s = "{";
    for (auto &el : map)
        s += str(el.first) + ": " + str(el.second) + ", ";

    s.erase(s.end() - 2, s.end());
    s += "}";
    return s;
}

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G>
std::string str(A a, B b, C c, D d, E e, F f, G g)
{
    return str(a) + str(b) + str(c) + str(d) + str(e) + str(f) + str(g);
}

template <typename A, typename B, typename C, typename D, typename E, typename F>
std::string str(A a, B b, C c, D d, E e, F f)
{
    return str(a) + str(b) + str(c) + str(d) + str(e) + str(f);
}

template <typename A, typename B, typename C, typename D, typename E>
std::string str(A a, B b, C c, D d, E e)
{
    return str(a) + str(b) + str(c) + str(d) + str(e);
}

template <typename A, typename B, typename C, typename D>
std::string str(A a, B b, C c, D d)
{
    return str(a) + str(b) + str(c) + str(d);
}

template <typename A, typename B, typename C>
std::string str(A a, B b, C c)
{
    return str(a) + str(b) + str(c);
}

template <typename A, typename B>
std::string str(A a, B b)
{
    return str(a) + str(b);
}

template <typename A, typename B, typename C, typename D, typename E, typename F>
void print(A a, B b, C c, D d, E e, F f)
{
    std::cout << str(a) << " " << str(b) << " " << str(c) << " " << str(d) << " "
              << str(e) << " " << str(f) << std::endl;
}

template <typename A, typename B, typename C, typename D, typename E>
void print(A a, B b, C c, D d, E e)
{
    std::cout << str(a) << " " << str(b) << " " << str(c) << " " << str(d) << " "
              << str(e) << std::endl;
}

template <typename A, typename B, typename C, typename D>
void print(A a, B b, C c, D d)
{
    std::cout << str(a) << " " << str(b) << " " << str(c) << " " << str(d)
              << std::endl;
}

template <typename A, typename B, typename C>
void print(A a, B b, C c)
{
    std::cout << str(a) << " " << str(b) << " " << str(c)
              << std::endl;
}

template <typename A, typename B>
void print(A a, B b)
{
    std::cout << str(a) << " " << str(b)
              << std::endl;
}

template <typename A>
void print(A a)
{
    std::cout << str(a) << std::endl;
}

inline void print()
{
    std::cout << std::endl;
}

// Logs something to the file
template <typename A>
void log(A a, std::string filename = "log.txt")
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

// A helpful class for easy measuring of time some part of the code takes
class Timeit
{
    std::chrono::high_resolution_clock::time_point start;

public:
    // Class methods defined in header files are inlined by default
    Timeit()
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