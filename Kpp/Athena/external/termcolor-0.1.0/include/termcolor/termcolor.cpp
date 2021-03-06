//!
//! termcolor
//! ~~~~~~~~~
//!
//! termcolor is a header-only c++ library for printing colored messages
//! to the terminal. Written just for fun with a help of the Force.
//!
//! :copyright: (c) 2013 by Igor Kalnitsky
//! :license: BSD, see LICENSE for details
//!

#pragma warning(push)
#pragma warning(disable: 4800)

// the following snippet of code detects the current OS and
// defines the appropriate macro that is used to wrap some
// platform specific things
#if defined(_WIN32) || defined(_WIN64)
#   define TERMCOLOR_OS_WINDOWS
#elif defined(__APPLE__)
#   define TERMCOLOR_OS_MACOS
#elif defined(__unix__) || defined(__unix)
#   define TERMCOLOR_OS_LINUX
#else
#   error unsupported platform
#endif

#include "termcolor.hpp"

// This headers provides the `isatty()`/`fileno()` functions,
// which are used for testing whether a standart stream refers
// to the terminal. As for Windows, we also need WinApi funcs
// for changing colors attributes of the terminal.
#if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
#   include <unistd.h>
#elif defined(TERMCOLOR_OS_WINDOWS)
#   include <io.h>
#   include <windows.h>
#endif

#include <cstdio>

namespace termcolor
{
    //! Since C++ hasn't a way to hide something in the header from
    //! the outer access, I have to introduce this namespace which
    //! is used for internal purpose and should't be access from
    //! the user code.
    namespace _internal
    {
        //! Since C++ hasn't a true way to extract stream handler
        //! from the a given `std::ostream` object, I have to write
        //! this kind of hack.
        FILE* get_standard_stream(const std::ostream& stream)
        {
            if (&stream == &std::cout)
                return stdout;
            else if ((&stream == &std::cerr) || (&stream == &std::clog))
                return stderr;

            return 0;
        }

        //! Test whether a given `std::ostream` object refers to
        //! a terminal.
        bool is_atty(const std::ostream& stream)
        {
            FILE* std_stream = get_standard_stream(stream);

        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            return ::isatty(fileno(std_stream));
        #elif defined(TERMCOLOR_OS_WINDOWS)
            return ::_isatty(_fileno(std_stream));
        #endif
        }

    #if defined(TERMCOLOR_OS_WINDOWS)

        int & boldfore()
        {
            static int value = 0;
            return value;
        }

        int & boldback()
        {
            static int value = 0;
            return value;
        }

        int & underscore()
        {
            static int value = 0;
            return value;
        }

        int & lastattrs()
        {
            static int value = 0x00ff;
            return value;
        }

        //! Change Windows Terminal colors attribute. If some
        //! parameter is `-1` then attribute won't changed.
        void win_change_attributes(std::ostream& stream, int foreground, int background = -1)
        {
            foreground |= _internal::boldfore();
            background |= _internal::boldback();
            foreground |= _internal::underscore();

            // yeah, i know.. it's ugly, it's windows.
            static WORD defaultAttributes = 0;

            // get terminal handle
            HANDLE hTerminal = INVALID_HANDLE_VALUE;
            if (&stream == &std::cout)
                hTerminal = GetStdHandle(STD_OUTPUT_HANDLE);
            else if (&stream == &std::cerr)
                hTerminal = GetStdHandle(STD_ERROR_HANDLE);

            // save default terminal attributes if it unsaved
            if (!defaultAttributes)
            {
                CONSOLE_SCREEN_BUFFER_INFO info;
                if (!GetConsoleScreenBufferInfo(hTerminal, &info))
                    return;
                defaultAttributes = info.wAttributes;
            }

            // restore all default settings
            if (foreground == -1 && background == -1)
            {
                SetConsoleTextAttribute(hTerminal, defaultAttributes);
                return;
            }

            // get current settings
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (!GetConsoleScreenBufferInfo(hTerminal, &info))
                return;

            if (foreground != -1)
            {
                info.wAttributes &= ~(info.wAttributes & 0x0F);
                info.wAttributes |= static_cast<WORD>(foreground);
            }

            if (background != -1)
            {
                info.wAttributes &= ~(info.wAttributes & 0xF0);
                info.wAttributes |= static_cast<WORD>(background);
            }

            SetConsoleTextAttribute(hTerminal, info.wAttributes);
        }
    #endif

    }

    void set_title(std::string const & title)
    {
    #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
    #elif defined(TERMCOLOR_OS_WINDOWS)
        ::SetConsoleTitleA(title.c_str());
    #endif
    }

    std::ostream& push_attrs(std::ostream& stream)
    {
    #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
    #elif defined(TERMCOLOR_OS_WINDOWS)
        HANDLE hTerminal = INVALID_HANDLE_VALUE;
        if (&stream == &std::cout)
            hTerminal = GetStdHandle(STD_OUTPUT_HANDLE);
        else if (&stream == &std::cerr)
            hTerminal = GetStdHandle(STD_ERROR_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(hTerminal, &info))
        {
            _internal::lastattrs() = info.wAttributes;
        }
    #endif
        return stream;
    }

    std::ostream& pop_attrs(std::ostream& stream)
    {
    #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
    #elif defined(TERMCOLOR_OS_WINDOWS)
        auto attrs = _internal::lastattrs();
        _internal::win_change_attributes(stream, attrs & 0x0f, attrs & 0xf0);
    #endif
        return stream;
    }

    std::ostream& reset(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[00m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::boldfore() = 0;
            _internal::boldback() = 0;
            _internal::win_change_attributes(stream, -1, -1);
        #endif
        }
        return stream;
    }

    std::ostream& bold(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[1m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
        #endif
        }
        return stream;
    }

    std::ostream& dark(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[2m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
        #endif
        }
        return stream;
    }

    std::ostream& underline(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[4m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::underscore() = COMMON_LVB_UNDERSCORE;
        #endif
        }
        return stream;
    }

    std::ostream& blink(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[5m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
        #endif
        }
        return stream;
    }

    std::ostream& reverse(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[7m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
        #endif
        }
        return stream;
    }

    std::ostream& concealed(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[8m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
        #endif
        }
        return stream;
    }

    std::ostream& boldfore(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::boldfore() = FOREGROUND_INTENSITY;
        #endif
        }
        return stream;
    }

    std::ostream& boldback(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::boldback() = FOREGROUND_INTENSITY;
        #endif
        }
        return stream;
    }

    std::ostream& darkfore(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::boldfore() = 0;
        #endif
        }
        return stream;
    }

    std::ostream& darkback(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::boldback() = 0;
        #endif
        }
        return stream;
    }

    std::ostream& grey(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[30m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream,
                0   // grey (black)
            );
        #endif
        }
        return stream;
    }

    std::ostream& red(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[31m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream,
                FOREGROUND_RED
            );
        #endif
        }
        return stream;
    }

    std::ostream& green(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[32m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream,
                FOREGROUND_GREEN
            );
        #endif
        }
        return stream;
    }

    std::ostream& yellow(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[33m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream,
                FOREGROUND_GREEN | FOREGROUND_RED
            );
        #endif
        }
        return stream;
    }

    std::ostream& blue(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[34m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream,
                FOREGROUND_BLUE
            );
        #endif
        }
        return stream;
    }

    std::ostream& magenta(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[35m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream,
                FOREGROUND_BLUE | FOREGROUND_RED
            );
        #endif
        }
        return stream;
    }

    std::ostream& cyan(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[36m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream,
                FOREGROUND_BLUE | FOREGROUND_GREEN
            );
        #endif
        }
        return stream;
    }

    std::ostream& white(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[37m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream,
                FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
            );
        #endif
        }
        return stream;
    }

     std::ostream& on_grey(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[40m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream, -1,
                0   // grey (black)
            );
        #endif
        }
        return stream;
    }

    std::ostream& on_red(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[41m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream, -1,
                BACKGROUND_RED
            );
        #endif
        }
        return stream;
    }

    std::ostream& on_green(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[42m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream, -1,
                BACKGROUND_GREEN
            );
        #endif
        }
        return stream;
    }

    std::ostream& on_yellow(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[43m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream, -1,
                BACKGROUND_GREEN | BACKGROUND_RED
            );
        #endif
        }
        return stream;
    }

    std::ostream& on_blue(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[44m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream, -1,
                BACKGROUND_BLUE
            );
        #endif
        }
        return stream;
    }

    std::ostream& on_magenta(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[45m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream, -1,
                BACKGROUND_BLUE | BACKGROUND_RED
            );
        #endif
        }
        return stream;
    }

     std::ostream& on_cyan(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[46m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream, -1,
                BACKGROUND_GREEN | BACKGROUND_BLUE
            );
        #endif
        }
        return stream;
    }

    std::ostream& on_white(std::ostream& stream)
    {
        if (_internal::is_atty(stream))
        {
        #if defined(TERMCOLOR_OS_MACOS) || defined(TERMCOLOR_OS_LINUX)
            stream << "\033[47m";
        #elif defined(TERMCOLOR_OS_WINDOWS)
            _internal::win_change_attributes(stream, -1,
                BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED
            );
        #endif
        }

        return stream;
    }
}

#undef TERMCOLOR_OS_WINDOWS
#undef TERMCOLOR_OS_MACOS
#undef TERMCOLOR_OS_LINUX

#pragma warning(pop)