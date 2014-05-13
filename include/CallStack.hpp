#pragma once
//  (C) Copyright 2008-2014 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  https://github.com/jonaspersson/utilib

#ifdef _WIN32

#include <list>
#include <string>
#include <iosfwd>
#include <windows.h>

#include <boost/shared_ptr.hpp>

class DbgHelp;

class CallStack 
{
public:
  CallStack();
  CallStack(unsigned startEntry);
  CallStack(CONTEXT);

private:
  //CallStack(const CallStack&);
  //CallStack& operator=(const CallStack&);

  friend std::ostream& operator<<(std::ostream& os, const CallStack& callStack);
  friend std::wostream& operator<<(std::wostream& os, const CallStack& callStack);

  void    collectStackAddresses(CONTEXT,unsigned startEntry);
  void    collectStackAddresses(unsigned startEntry);
  template<typename char_t>
  std::basic_string<char_t> resolveCallStack() const;
  template<typename char_t>
  std::basic_string<char_t> resolveSymbol(DWORD64 address) const;
  template<typename char_t>
  std::basic_string<char_t> resolveModule(DWORD64 address) const;
  template<typename char_t>
  std::basic_string<char_t> formatLogEntry() const;

  typedef std::list<DWORD64>    CallStackAddresses;
  CallStackAddresses        mCallStackAddresses;
  boost::shared_ptr<DbgHelp>  mDbgHelp;
};

std::ostream& operator<<(std::ostream& os, const CallStack& callStack);
std::wostream& operator<<(std::wostream& os, const CallStack& callStack);
#else

#include <iostream>
#include <execinfo.h>
  class CallStack 
  {
    std::vector<std::string> stack;
  public:
    CallStack() { do_trace(3); }
    CallStack(unsigned startEntry) { do_trace(startEntry+3); }
  private:
  void do_trace (unsigned startEntry)
    {
#ifndef _WIN32

      void *array[10];

      size_t size = backtrace (array, 10);
      char** strings = backtrace_symbols (array, size);

      for (size_t i = startEntry; i < size; i++)
        stack.push_back(strings[i]);

      free (strings); 
#endif
  }
  public:
  friend std::ostream& operator<<(std::ostream& os, const CallStack& callStack) { 
    for(auto const& s:callStack.stack)
      os << s << std::endl;;
    return os; 
  }
  };

  //inline std::ostream& operator<<(std::ostream& os, const CallStack& callStack) { os << " --- "; return os; }
  inline std::wostream& operator<<(std::wostream& os, const CallStack& callStack) { os << L" --- "; return os; }

#endif
