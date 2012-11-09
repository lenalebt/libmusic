#ifndef CONSOLE_COLORS_HPP
#define CONSOLE_COLORS_HPP

#include <string>

/**
 * @brief The color namespace contains all classes and functions related to color output on the console
 * @ingroup tools
 */
namespace colors
{
    /**
     * @brief This class is for colors on the console.
     * 
     * This class has several functions, each returning a string. These strings
     * are color escapes for the console. Just write them to
     * <code>stdout</code> and the following text will be colored.
     * It is known to be working with
     * <code>bash</code>.
     * 
     * Example:
     * @code
     * std::cout << colors::ConsoleColors::red()         << "I am red!" << std::endl;
     * std::cout << colors::ConsoleColors::green()       << "And I am green." << std::endl;
     * std::cout << colors::ConsoleColors::defaultText() << "I am normal text." << std::endl;
     * @endcode
     * You need to call defaultText() in order to reset the text color.
     * If you don't call it, the text will not be reset to the standard
     * color, so reset it right after you used it with a different color.
     * 
     * @ingroup tools
     * 
     * @author Lena Brueder
     * @date 2012-07-16
     */
    class ConsoleColors
    {
    private:
        
    protected:
        static bool colorsEnabled;
    public:
        /**
         * @brief Change the state of console colors.
         * 
         * If you disable the console colors, every call to a function of
         * this class will return an empty string.
         * 
         * @param enabled If the console colors should be enabled, or not.
         */
        static void setColorsEnabled(bool enabled)  {colorsEnabled = enabled;}
        /**
         * @brief Returns if colors are enabled, or not.
         * @return if colors are enabled, or not.
         */
        static bool getColorsEnabled()  {return colorsEnabled;}
        
        /** @brief Return the color string for yellow.
         *  @return The appropriate color string (empty, if colors were disabled). */
        static std::string yellow()         {if (colorsEnabled) return std::string("\033[33m"); else return std::string("");}
        /** @brief Return the color string for blue.
         *  @return The appropriate color string (empty, if colors were disabled). */
        static std::string blue()           {if (colorsEnabled) return std::string("\033[34m"); else return std::string("");}
        /** @brief Return the color string for green.
         *  @return The appropriate color string (empty, if colors were disabled). */
        static std::string green()          {if (colorsEnabled) return std::string("\033[32m"); else return std::string("");}
        /** @brief Return the color string for red.
         *  @return The appropriate color string (empty, if colors were disabled). */
        static std::string red()            {if (colorsEnabled) return std::string("\033[31m"); else return std::string("");}
        /** @brief Return the color string for magenta.
         *  @return The appropriate color string (empty, if colors were disabled). */
        static std::string magenta()        {if (colorsEnabled) return std::string("\033[35m"); else return std::string("");}
        /** @brief Return the color string for cyan.
         *  @return The appropriate color string (empty, if colors were disabled). */
        static std::string cyan()           {if (colorsEnabled) return std::string("\033[36m"); else return std::string("");}
        /** @brief Return the color string for white.
         *  @return The appropriate color string (empty, if colors were disabled). */
        static std::string white()          {if (colorsEnabled) return std::string("\033[37m"); else return std::string("");}
        /** @brief Return the color string for black.
         *  @return The appropriate color string (empty, if colors were disabled). */
        static std::string black()          {if (colorsEnabled) return std::string("\033[30m"); else return std::string("");}
        /** @brief Return the color string for default text.
         *  @return The appropriate color string (empty, if colors were disabled). */
        static std::string defaultText()    {if (colorsEnabled) return std::string("\033[0m");  else return std::string("");}
        
        /** @brief Return the string for bold text.
         *  @return The appropriate string (empty, if colors were disabled). */
        static std::string bold()           {if (colorsEnabled) return std::string("\033[1m"); else return std::string("");}
        /** @brief Return the string for underlined.
         *  @return The appropriate string (empty, if colors were disabled). */
        static std::string underline()      {if (colorsEnabled) return std::string("\033[4m"); else return std::string("");}
        /** @brief Return the string for striked text.
         *  @return The appropriate string (empty, if colors were disabled). */
        static std::string strike()         {if (colorsEnabled) return std::string("\033[9m"); else return std::string("");}
        /** @brief Return the color string for a darker color (needs to be combined with a color).
         *  @code
         *  std::cout << colors::ConsoleColors::red()         << "I am red!" << std::endl;
         *  std::cout << colors::ConsoleColors::darker()      << "And I am red, but darker." << std::endl;
         *  std::cout << colors::ConsoleColors::defaultText() << std::endl;
         *  @endcode
         *  @return The appropriate string (empty, if colors were disabled). */
        static std::string darker()         {if (colorsEnabled) return std::string("\033[2m"); else return std::string("");}
        //static std::string text()           {if (colorsEnabled) return std::string(""); else return std::string("");}
        /** @brief Return the string for setup of background colors (needs to be combined with a color).
         *  @return The appropriate string (empty, if colors were disabled). */
        static std::string background()     {if (colorsEnabled) return std::string("\033[7m"); else return std::string("");}
    };
    
}

#endif //CONSOLE_COLORS_HPP
