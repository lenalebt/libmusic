#ifndef STRINGHELPER_HPP
#define STRINGHELPER_HPP

#include <string>
#include <algorithm>
#include <cctype>

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

inline void tolower(std::string& string)
{
    std::transform(string.begin(), string.end(), string.begin(), (int (*)(int))::tolower);
}

inline void toupper(std::string& string)
{
    std::transform(string.begin(), string.end(), string.begin(), (int (*)(int))::toupper);
}

inline bool contains(const std::string& string, const std::string& searchFor)
{
    return (std::string::npos != string.find(searchFor));
}

#endif //STRINGHELPER_HPP 
