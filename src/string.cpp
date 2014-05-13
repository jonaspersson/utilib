//  (C) Copyright 2008-2014 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  https://github.com/jonaspersson/utilib

#include "string.hpp"
namespace utilib {
        std::string print_str(const char *format, ...)
        {
            va_list arg;
            va_start (arg, format);
#ifdef _WIN32
            int len = _vscprintf( format, arg); // len without terminating '\0'
            CHECK(len>0);
            std::string buf((size_t)len,'\0');
            vsprintf_s (&buf[0], len+1, format, arg);
#else
            int len = vsnprintf(NULL, 0, format, arg);
            CHECK(len>0);
            std::string buf((size_t)len+1,'\0');
            vsprintf (&buf[0], format, arg);
#endif
            va_end (arg);

            return buf;
        }
#ifdef _WIN32
        std::wstring wprint_str(const wchar_t *format, ...)
        {
            va_list arg;
            va_start (arg, format);
            int len = _vscwprintf( format, arg); // len without terminating '\0'
            CHECK(len>0);
            std::wstring buf((size_t)len,'\0');
            vswprintf_s (&buf[0], len+1, format, arg);
            va_end (arg);

            return buf;
        }
#endif

}
