#ifndef CONSOLE_COLORS_HPP
#define CONSOLE_COLORS_HPP

#include <string>

namespace colors
{
    class ConsoleColors
    {
    private:
        
    protected:
        static bool colorsEnabled;
    public:
        static void setColorsEnabled(bool enabled)  {colorsEnabled = enabled;}
        static bool getColorsEnabled()  {return colorsEnabled;}
        
        static std::string yellow()         {if (colorsEnabled) return std::string("\033[33m"); else return std::string("");}
        static std::string blue()           {if (colorsEnabled) return std::string("\033[34m"); else return std::string("");}
        static std::string green()          {if (colorsEnabled) return std::string("\033[32m"); else return std::string("");}
        static std::string red()            {if (colorsEnabled) return std::string("\033[31m"); else return std::string("");}
        static std::string magenta()        {if (colorsEnabled) return std::string("\033[35m"); else return std::string("");}
        static std::string cyan()           {if (colorsEnabled) return std::string("\033[36m"); else return std::string("");}
        static std::string white()          {if (colorsEnabled) return std::string("\033[37m"); else return std::string("");}
        static std::string black()          {if (colorsEnabled) return std::string("\033[30m"); else return std::string("");}
        static std::string defaultText()    {if (colorsEnabled) return std::string("\033[0m");  else return std::string("");}
        
        static std::string bold()           {if (colorsEnabled) return std::string("\033[1m"); else return std::string("");}
        static std::string underline()      {if (colorsEnabled) return std::string("\033[4m"); else return std::string("");}
        static std::string strike()         {if (colorsEnabled) return std::string("\033[9m"); else return std::string("");}
        static std::string darker()         {if (colorsEnabled) return std::string("\033[2m"); else return std::string("");}
        //static std::string text()           {if (colorsEnabled) return std::string(""); else return std::string("");}
        static std::string background()     {if (colorsEnabled) return std::string("\033[7m"); else return std::string("");}
    };
    
}

#endif //CONSOLE_COLORS_HPP
