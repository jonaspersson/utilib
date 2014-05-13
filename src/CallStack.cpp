//  (C) Copyright 2008-2013 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
#ifdef _WIN32
#pragma optimize("y", off) // disable fpo

#include "CallStack.hpp"
#include "DbgHelp.hpp"
#include "string_cast.h"
#include <iostream>
#include <sstream>


//  using generic::string_cast;
  namespace {
    template <class U,class V>
    U const* selectArg(U const* arg,V const*) { return arg; }

    template <class V,class U>
    V const* selectArg(U const*,V const* arg) { return arg; }

    template <class V>
    V const* selectArg(V const*,V const* arg);

    #define LIT(type, str) selectArg<type>(str, L##str)
  }
  CallStack::CallStack()
  {
    try
    {
      // we need a reference to a dbgHelp through out the life time of this object
      mDbgHelp = DbgHelp::instance();     

      // collect the call stack addresses
      collectStackAddresses(0);
    }
    catch(const DbgHelp::StackDumpException& sde)
    {
      std::cerr << "Failed to initialize dbgHelp! " << sde.mFile << "(" << sde.mLine << "): " << sde.mDesc << ", GetLastError == " << sde.mWinError << std::endl;
    }
  }
  CallStack::CallStack(unsigned startEntry)
  {
    try
    {
      // we need a reference to a dbgHelp through out the life time of this object
      mDbgHelp = DbgHelp::instance();     

      // collect the call stack addresses
      collectStackAddresses(startEntry);
    }
    catch(const DbgHelp::StackDumpException& sde)
    {
      std::cerr << "Failed to initialize dbgHelp! " << sde.mFile << "(" << sde.mLine << "): " << sde.mDesc << ", GetLastError == " << sde.mWinError << std::endl;
    }
  }
  CallStack::CallStack(CONTEXT context)
  {
    try
    {
      // we need a reference to a dbgHelp through out the life time of this object
      mDbgHelp = DbgHelp::instance();     

      // collect the call stack addresses
      collectStackAddresses(context,0);
    }
    catch(const DbgHelp::StackDumpException& sde)
    {
      std::cerr << "Failed to initialize dbgHelp! " << sde.mFile << "(" << sde.mLine << "): " << sde.mDesc << ", GetLastError == " << sde.mWinError << std::endl;
    }
  }
  void CallStack::collectStackAddresses(unsigned startEntry) 
  {
    CONTEXT threadContext;
    memset(&threadContext, 0, sizeof(CONTEXT));
    threadContext.ContextFlags = CONTEXT_FULL;
    //  ::GetThreadContext(thread, &threadContext);
    HMODULE kernel32 = ::GetModuleHandleW(L"kernel32.dll");
    // CHECK(kernel32 != NULL) (::GetLastError()); // recursive
    assert(kernel32 != NULL);
    typedef VOID (__stdcall * RtlCaptureContextFnc)(PCONTEXT);
    RtlCaptureContextFnc rtlCaptureContext = reinterpret_cast<RtlCaptureContextFnc>   (GetProcAddress(kernel32, "RtlCaptureContext"));
    if(rtlCaptureContext) rtlCaptureContext(&threadContext);
 
    // else TODO. use external thread suspend GetThreadContext resume
    //::RtlCaptureContext(&threadContext);
    collectStackAddresses(threadContext,startEntry);
  }
  void CallStack::collectStackAddresses(CONTEXT threadContext,unsigned startEntry)
  {
    STACKFRAME64  stackFrame;
    memset(&stackFrame, 0, sizeof(STACKFRAME64));

    // program counter, stack pointer, and frame pointer
#if defined(_M_IX86)
    stackFrame.AddrPC.Mode    = AddrModeFlat;
    stackFrame.AddrPC.Offset    = threadContext.Eip;
    stackFrame.AddrStack.Offset = threadContext.Esp;
    stackFrame.AddrStack.Mode   = AddrModeFlat;
    stackFrame.AddrFrame.Offset = threadContext.Ebp;
    stackFrame.AddrFrame.Mode   = AddrModeFlat;
#elif defined(_M_X64)
    stackFrame.AddrPC.Mode    = AddrModeFlat;
    stackFrame.AddrPC.Offset    = threadContext.Rip;
    stackFrame.AddrStack.Offset = threadContext.Rsp;
    stackFrame.AddrStack.Mode   = AddrModeFlat;
    stackFrame.AddrFrame.Offset = threadContext.Rbp;
    stackFrame.AddrFrame.Mode   = AddrModeFlat;
#else
#error unsupported architecture
#endif
    DbgHelp::Guard lock(mDbgHelp->mutex());
    int maxFrameCount = 128;

    HANDLE thread = ::GetCurrentThread();

    unsigned i=0;
    while(--maxFrameCount && mDbgHelp->StackWalk64(lock, thread, &stackFrame, &threadContext))
    {
      if(startEntry > i++) continue;
      mCallStackAddresses.push_back(stackFrame.AddrPC.Offset);
    }
  }
  // Resolve symbol names for stack addresses and build call stack
  template<typename char_t>
  std::basic_string<char_t> CallStack::resolveCallStack() const
  {
    std::basic_stringstream<char_t> os;
//     bool stop = false;
//     bool lastOk = true;

    for(CallStackAddresses::const_iterator i = mCallStackAddresses.begin(); /*!stop &&*/ i != mCallStackAddresses.end(); ++i)
    {
      const std::basic_string<char_t> module = resolveModule<char_t>((*i));
      const std::basic_string<char_t> symbol = resolveSymbol<char_t>((*i));

      // stop collecting if two consequtive <no module>/ <no symbol
//       if(module.find(LIT(char_t,"<no module>")) == 0 && symbol.find(LIT(char_t,"<no symbol")) == 0)
//       {
//         if(!lastOk)
//         {
//           stop = true;
//         }
//         else
//         {
//           lastOk = false;
//         }
//       }

      os.width(8);
      os.fill('0');
      os << std::hex << static_cast<DWORD>((*i)) << ": ";   // TODO: print all 64 bits of address
      os << module << "! " << symbol << std::endl;
    }

    return os.str();
  }
  // Return the symbol name for given address
  template<typename char_t>
  std::basic_string<char_t> CallStack::resolveSymbol(DWORD64 address) const
  {
    std::basic_string<char_t> symbolName = LIT(char_t,"<no symbol>");
    static const int maxSymbolNameLength  = 500; // TODO varför 500?

    union 
    {
      CHAR        symbolFiller[sizeof(IMAGEHLP_SYMBOL64) + maxSymbolNameLength];  // used to get some space for symbol name
      IMAGEHLP_SYMBOL64 sym;
    };

    memset(&sym, 0, sizeof(IMAGEHLP_SYMBOL64));
    sym.SizeOfStruct    = sizeof(IMAGEHLP_SYMBOL64);
    sym.Address       = address;
    sym.MaxNameLength   = maxSymbolNameLength;
    DWORD64 offset      = 0;

    try
    {
      std::basic_stringstream<char_t> os;

      if(mDbgHelp->SymGetSymFromAddr64(address, &offset, &sym))
      {
        CHAR tmpBuf[maxSymbolNameLength];

        if(mDbgHelp->UnDecorateSymbolName(sym.Name, tmpBuf, maxSymbolNameLength, UNDNAME_COMPLETE))
        {
          os << tmpBuf;
        }
        else
        {
          os << sym.Name;
        }

        if (offset)
        {
          os << " + " << static_cast<DWORD>(offset) << " bytes";  // TODO: print all 64 bits of offset
        }

        symbolName = os.str();
      }
      else
      {
        os << symbolName << " (GetLastError = " << GetLastError() << ")";
        symbolName = os.str();
      }
    }
    catch(...)
    {
      symbolName = LIT(char_t,"<no symbol: exception while retrieving symbol name>");
    }

    return symbolName;
  }
  // Resolve module name for given address
  template<typename char_t>
  std::basic_string<char_t> CallStack::resolveModule(DWORD64 address)  const
  {
    std::basic_string<char_t> moduleName = LIT(char_t,"<no module>");

    IMAGEHLP_MODULE64 mi;
    mi.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

    if(mDbgHelp->SymGetModuleInfo64(address, &mi))
    {
      moduleName = string_cast<std::basic_string<char_t> >((const char*)mi.ImageName);
    }

    return moduleName;
  }
  template<typename char_t>
  std::basic_string<char_t> CallStack::formatLogEntry() const
  {
    std::basic_stringstream<char_t> os;

    try
    {
      os << resolveCallStack<char_t>();
    }
    catch(const DbgHelp::StackDumpException& sde)
    {
      os << "Failed to get stack dump! " << sde.mFile << "(" << sde.mLine << "): " << sde.mDesc << ", GetLastError == " << sde.mWinError << std::endl;
    }
    catch(...)
    {
      os << "Failed to get stack dump, unknown exception occurred " << std::endl;
    }

    return os.str();
  }
  std::ostream& operator<<(std::ostream& os, const CallStack& callStack)
  {
    os << callStack.formatLogEntry<char>();
    return os;
  }
  std::wostream& operator<<(std::wostream& os, const CallStack& callStack)
  {
    os << callStack.formatLogEntry<wchar_t>();
    return os;
  }
#pragma optimize("y", on)
#endif
