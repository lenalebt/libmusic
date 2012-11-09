 
#ifndef TESTFRAMEWORK_HPP
#define TESTFRAMEWORK_HPP

#include <string>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <iomanip>

#include "debug.hpp"

#define CHECK_EQ(a,b)           if (!check_equality(LINESTR(a,b), a, b)) return EXIT_FAILURE;
#define CHECK_OP(a,op,b)        std::cerr << std::left << std::setw(TEST_PASSED_MSG_WIDTH) << LINESTR_OP(a,op,b) << " - " << std::flush; if (!(a op b)) {std::cerr << "failed!" << std::endl << "\t" << a << " " << std::string(QUOTEME(op)) << " " << b << " ?"<< std::endl; return EXIT_FAILURE;} else {std::cerr << "passed!" << std::endl;}
#define CHECK_EQ_TYPE(a,b,type) if (!check_equality<type, type >(LINESTR(a,b), a, b)) return EXIT_FAILURE;
#define CHECK(a)                if (!check_equality(LINESTR(a,true), a, true)) return EXIT_FAILURE;

#define DOUBLE_EQUALITY_BARRIER 10e-7
#define FLOAT_EQUALITY_BARRIER  10e-5
#define TEST_PASSED_MSG_WIDTH 80

/**
 * @ingroup tests
 * @brief testing functions will be put in this namespace.
 */
namespace tests
{
    /**
     * @brief checks, if two values are identical. Prints out
     *      <code>message</code>, if not.
     * 
     * Von dieser Funktion muss für jeden Typen, der verglichen werden soll,
     * eine Implementierung existieren. Es reicht in den meisten Fällen aus, wenn
     * in der Datei <code>tests.cpp</code> zu instantiieren, wie dies schon mit
     * einigen anderen Funktionen dort der Fall ist.
     * 
     * @param a element a
     * @param b element b
     * @param message the message that will be displayed on the console, if our equality test fails.
     * @return if the test was successful, or not.
     */
    template<typename S, typename T>
    inline bool check_equality(std::string message, S a, T b)
    {
        std::cerr << std::left << std::setw(TEST_PASSED_MSG_WIDTH) << message << " - " << std::flush;
        if (a==b)
        {
            std::cerr << "passed!" << std::endl;
            return true;
        }
        else
        {
            std::cerr << "failed!" << std::endl;
            std::cerr << "\tValue A: " << std::fixed << std::setprecision(15) << a << std::endl;
            std::cerr << "\tValue B: " << std::fixed << std::setprecision(15) << b << std::endl;
            return false;
        }
    }
    
    template<> inline bool check_equality(std::string message, double a, double b)
    {
        std::cerr << std::left << std::setw(TEST_PASSED_MSG_WIDTH) << message << " - " << std::flush;
        if ((a == b) || (fabs((a / b) - 1) < DOUBLE_EQUALITY_BARRIER))
        {
            std::cerr << "passed!" << std::endl;
            return true;
        }
        else
        {
            std::cerr << "failed!" << std::endl;
            std::cerr << "\tValue A: " << std::fixed << std::setprecision(15) << a << std::endl;
            std::cerr << "\tValue B: " << std::fixed << std::setprecision(15) << b << std::endl;
            return false;
        }
    }
    template<> inline bool check_equality(std::string message, float a, float b)
    {
        std::cerr << std::left << std::setw(TEST_PASSED_MSG_WIDTH) << message << " - " << std::flush;
        if ((a == b) || (fabs((a / b) - 1) < FLOAT_EQUALITY_BARRIER))
        {
            std::cerr << "passed!" << std::endl;
            return true;
        }
        else
        {
            std::cerr << "failed!" << std::endl;
            std::cerr << "\tValue A: " << std::fixed << std::setprecision(15) << a << std::endl;
            std::cerr << "\tValue B: " << std::fixed << std::setprecision(15) << b << std::endl;
            return false;
        }
    }
    
    template<> inline bool check_equality(std::string message, float a, double b)
    {
        return check_equality<float, float>(message, a, b);
    }
    template<> inline bool check_equality(std::string message, double a, float b)
    {
        return check_equality<float, float>(message, a, b);
    }
}

#endif //TESTFRAMEWORK_HPP 
