#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>
#include <iomanip>
#include "console_colors.hpp"

/**
 * @brief for making strings out of defines.
 */
#define QUOTEME_(x) #x
/**
 * @brief for making strings out of defines.
 */
#define QUOTEME(x) QUOTEME_(x)

#if defined __FILE__ && defined __LINE__
    #define LINESTR(a,b)           colors::ConsoleColors::yellow() + tests::basename(std::string(QUOTEME(__FILE__))) + colors::ConsoleColors::defaultText() + ":" + colors::ConsoleColors::cyan() + QUOTEME(__LINE__) + colors::ConsoleColors::defaultText() + ": "+ QUOTEME(a) + " == " + QUOTEME(b) + "?"
    #define LINESTR_OP(a,op,b)     colors::ConsoleColors::yellow() + tests::basename(std::string(QUOTEME(__FILE__))) + colors::ConsoleColors::defaultText() + ":" + colors::ConsoleColors::cyan() + QUOTEME(__LINE__) + colors::ConsoleColors::defaultText() + ": "+ QUOTEME(a) + " " + QUOTEME(op) + " " + QUOTEME(b) + "?"
#else
    #define LINESTR(a,b)           std::string(QUOTEME(a)) + " == " + QUOTEME(b) + "?"
    #define LINESTR_OP(a,op,b)     std::string(QUOTEME(a)) + " " + QUOTEME(op) + " " + QUOTEME(b) + "?"
#endif

#ifndef NDEBUG
    
#else
    #ifndef DEBUG_LEVEL
        #define DEBUG_LEVEL 0
    #endif
#endif

#ifdef DEBUG_LEVEL
    #if defined __FILE__ && defined __LINE__
        #define DEBUG_OUT_NOENDL_LEVEL(str,level)   if (DEBUG_LEVEL >= level)   \
                                                    {                           \
                                                        std::cerr << std::setw(30+(level/2)) << colors::ConsoleColors::yellow() + tests::basename(std::string(QUOTEME(__FILE__))) + colors::ConsoleColors::defaultText() + ":" + colors::ConsoleColors::cyan() + QUOTEME(__LINE__) + colors::ConsoleColors::defaultText() +  ": " << str;            \
                                                    }                           \
                                        
    #else
        #define DEBUG_OUT_NOENDL_LEVEL(str,level)   std::cerr << str;
    #endif
#else
    #define DEBUG_OUT_NOENDL_LEVEL(str,level)     ;
#endif
#define DEBUG_OUT_NOENDL(str,level) DEBUG_OUT_NOENDL_LEVEL(str, level)
#define DEBUG_OUT(str,level)        DEBUG_OUT_NOENDL(str << std::endl, level)


namespace tests
{
    int testBasename();
    
    /**
     * @brief gives the basename of a given path, as the unix basename tool does.
     * 
     * This function works with unix and windows file names.
     * 
     * @return the basename of a filename
     */
    std::string basename(std::string filename);
}

#endif  //DEBUG_HPP 
