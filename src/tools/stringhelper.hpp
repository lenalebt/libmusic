#ifndef STRINGHELPER_HPP
#define STRINGHELPER_HPP

#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>

/**
 * @brief Tests, if a string ends with another string.
 * @return <code>true</code>, if <code>str</code> ends with <code>ending</code>. <code>false</code>
 *      otherwise.
 * @ingroup tools
 */
inline bool endsWith(const std::string& str, const std::string& ending)
{
    if (str.length() >= ending.length())
    {
        return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}

/**
 * @brief Converts a string to lower case.
 * @ingroup tools
 */
inline void tolower(std::string& string)
{
    std::transform(string.begin(), string.end(), string.begin(), (int (*)(int))::tolower);
}

/**
 * @brief Converts a string to upper case.
 * @ingroup tools
 */
inline void toupper(std::string& string)
{
    std::transform(string.begin(), string.end(), string.begin(), (int (*)(int))::toupper);
}

inline bool contains(const std::string& string, const std::string& searchFor)
{
    return (std::string::npos != string.find(searchFor));
}

/**
 * @brief Concatenates a std::string with an integer number as string.
 * @return The concatenated string with the number.
 * @ingroup tools
 */
inline std::string operator+(const std::string& str, int number)
{
    std::stringstream ss;
    ss << number;
    return str + ss.str();
}

#endif //STRINGHELPER_HPP 
