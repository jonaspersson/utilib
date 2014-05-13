 #pragma once
//  (C) Copyright 2008-2012 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)


#include <windows.h>
#include <dbghelp.h>
#include <boost/weak_ptr.hpp>
#pragma warning (push)
#pragma warning (disable : 4244)
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>
#pragma warning (pop)

class DbgHelp
{
public:
  static boost::shared_ptr<DbgHelp> instance();
  ~DbgHelp();

  // expose mutex to allow externally locking of some methods
  boost::recursive_mutex& mutex() const { return mMutex; }

  typedef boost::recursive_mutex::scoped_lock Guard;
  
  // dbgHelp.dll functions
  BOOL  SymGetModuleInfo64(DWORD64 address, PIMAGEHLP_MODULE64 moduleInfo) const;
  DWORD64 SymLoadModule64(HANDLE file, PCSTR imageName, PCSTR moduleName, DWORD64 baseOfDll, DWORD sizeOfDll) const;
  BOOL  SymInitialize(PCSTR userSearchPath, BOOL invadeProcess) const;
  BOOL  SymUnloadModule64(DWORD64 baseOfDll) const;
  BOOL  SymEnumerateModules64(PSYM_ENUMMODULES_CALLBACK64 enumModulesCallback, PVOID userContext) const;
  BOOL  SymCleanup() const;
  PVOID SymFunctionTableAccess64(DWORD64 addressBase) const;
  DWORD64 SymGetModuleBase64(DWORD64 address) const;
  BOOL  SymGetSymFromAddr64(DWORD64 address, PDWORD64 displacement, PIMAGEHLP_SYMBOL64 symbol) const;
  DWORD SymSetOptions(DWORD options) const;
  DWORD SymGetOptions() const;
  DWORD UnDecorateSymbolName(PCSTR decoratedName, PSTR undecoratedName, DWORD undecoratedLength, DWORD flags) const;
  BOOL  SymUnDName64(PIMAGEHLP_SYMBOL64 symbol, PSTR undecoratedName, DWORD undecoratedLength) const;
  BOOL  StackWalk64(Guard& lock, HANDLE thread, LPSTACKFRAME64 stackFrame, PVOID contextRecord) const;
  BOOL  MiniDumpWriteDump(DWORD ProcessId, HANDLE hFile, 
    MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
    PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, 
    PMINIDUMP_CALLBACK_INFORMATION CallbackParam) const;


  //BOOL  SymRegisterCallback64(PSYMBOL_REGISTERED_CALLBACK64 callbackFunction, ULONG64 userContext) const;
// currently not used:
//  BOOL SymEnumTypes(ULONG64 baseOfDll, PCTSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK enumSymbolsCallback, PVOID userContext) const;
//  BOOL SymEnumSymbols(ULONG64 baseOfDll, PCTSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK enumSymbolsCallback, PVOID userContext) const;
//  BOOL SymSetContext(PIMAGEHLP_STACK_FRAME stackFrame, PIMAGEHLP_CONTEXT Context) const;
//  BOOL SymGetTypeInfo(DWORD64 ModBase, ULONG TypeId, IMAGEHLP_SYMBOL_TYPE_INFO GetType, PVOID pInfo) const;
//  LPAPI_VERSION ImagehlpApiVersion() const;
  
  //
  // Internal exception signaling failure of stack traversal
  //
  struct StackDumpException
  {
    StackDumpException(const char* const file, int line, const char* const desc = 0)
    : mFile(file)
    , mLine(line)
    , mDesc(desc)
    , mWinError(::GetLastError())
    {}

    const char* const mFile;
    int         mLine;
    const char* const mDesc;
    DWORD       mWinError;
  };

private:
  explicit DbgHelp(HANDLE process);
  
  // N/A
  DbgHelp(const DbgHelp&);
  DbgHelp& operator=(const DbgHelp&);

  // dbgHelp.dll function types
  typedef BOOL      (__stdcall *SymGetModuleInfo64Fnc)    (HANDLE process, DWORD64 address, PIMAGEHLP_MODULE64 moduleInfo);
  typedef DWORD64   (__stdcall *SymLoadModule64Fnc)     (HANDLE process, HANDLE file, PCSTR imageName, PCSTR moduleName, DWORD64 baseOfDll, DWORD  sizeOfDll);
  typedef BOOL      (__stdcall *SymInitializeFnc)     (HANDLE process, PCSTR userSearchPath, BOOL invadeProcess);
  typedef BOOL      (__stdcall *SymUnloadModule64Fnc)   (HANDLE process, DWORD64 baseOfDll);
  typedef BOOL      (__stdcall *SymEnumerateModules64Fnc) (HANDLE process, PSYM_ENUMMODULES_CALLBACK64 enumModulesCallback, PVOID userContext);
  typedef BOOL      (__stdcall *SymCleanupFnc)        (HANDLE process);
  typedef PVOID     (__stdcall *SymFunctionTableAccess64Fnc)(HANDLE process, DWORD64 addressBase);
  typedef DWORD64   (__stdcall *SymGetModuleBase64Fnc)    (HANDLE process, DWORD64 address);
  typedef BOOL      (__stdcall *SymGetSymFromAddr64Fnc)   (HANDLE process, DWORD64 address, PDWORD64 displacement, PIMAGEHLP_SYMBOL64 symbol);
  typedef DWORD     (__stdcall *SymSetOptionsFnc)     (DWORD options);
  typedef DWORD     (__stdcall *SymGetOptionsFnc)     ();
  typedef DWORD     (__stdcall *UnDecorateSymbolNameFnc)  (PCSTR decoratedName, PSTR undecoratedName, DWORD undecoratedLength, DWORD flags);
  typedef BOOL      (__stdcall *SymUnDName64Fnc)      (PIMAGEHLP_SYMBOL64 symbol, PSTR undecoratedName, DWORD undecoratedLength);
  typedef BOOL      (__stdcall *StackWalk64Fnc)       (DWORD MachineType, HANDLE hProcess, HANDLE thread, LPSTACKFRAME64 stackFrame, PVOID contextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
  typedef BOOL      (__stdcall *MiniDumpWriteDumpFnc)    (HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
  //typedef BOOL      (__stdcall *SymRegisterCallback64Fnc) (HANDLE process, PSYMBOL_REGISTERED_CALLBACK64 callbackFunction, ULONG64 userContext);
// currently not used:
//  typedef BOOL      (__stdcall *SymEnumTypesFnc)      (HANDLE process, ULONG64 baseOfDll, PCTSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK enumSymbolsCallback, PVOID userContext);
//  typedef BOOL      (__stdcall *SymEnumSymbolsFnc)      (HANDLE process, ULONG64 baseOfDll, PCTSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK enumSymbolsCallback, PVOID userContext);
//  typedef BOOL      (__stdcall *SymSetContextFnc)     (HANDLE process, PIMAGEHLP_STACK_FRAME stackFrame, PIMAGEHLP_CONTEXT context);
//  typedef BOOL      (__stdcall *SymGetTypeInfoFnc)      (HANDLE process, DWORD64 modBase, ULONG typeId, IMAGEHLP_SYMBOL_TYPE_INFO getType, PVOID info);
//  typedef LPAPI_VERSION (__stdcall *ImagehlpApiVersionFnc)    ();

  HANDLE const                    mProcess;
  HMODULE                         mDbgHelp;
  mutable boost::recursive_mutex  mMutex;
  static boost::weak_ptr<DbgHelp> mInstance;
  static boost::mutex             mInstanceMutex;

  // dbgHelp.dll functions 
  SymGetModuleInfo64Fnc       mSymGetModuleInfo64;  
  SymLoadModule64Fnc          mSymLoadModule64;   
  SymInitializeFnc            mSymInitialize;   
  SymUnloadModule64Fnc        mSymUnloadModule64;
  SymEnumerateModules64Fnc    mSymEnumerateModules64;
  SymCleanupFnc               mSymCleanup;  
  SymFunctionTableAccess64Fnc mSymFunctionTableAccess64;
  SymGetModuleBase64Fnc       mSymGetModuleBase64;
  SymGetSymFromAddr64Fnc      mSymGetSymFromAddr64;
  SymSetOptionsFnc            mSymSetOptions;     
  SymGetOptionsFnc            mSymGetOptions;
  UnDecorateSymbolNameFnc     mUnDecorateSymbolName;
  SymUnDName64Fnc             mSymUnDName64;
  StackWalk64Fnc              mStackWalk64;
  MiniDumpWriteDumpFnc        mMiniDumpWriteDump;
  //SymRegisterCallback64Fnc      mSymRegisterCallback64;
// currently not used:        
//  SymEnumSymbolsFnc         mSymEnumSymbols;
//  SymEnumTypesFnc           mSymEnumTypes;
//  SymSetContextFnc          mSymSetContext;
//  SymGetTypeInfoFnc         mSymGetTypeInfo;
//  ImagehlpApiVersionFnc       mImagehlpApiVersion;

  // dbgHelp.dll call backs 
  static DWORD64 CALLBACK getModuleBase(HANDLE process, DWORD64 address);
  static BOOL CALLBACK unloadModule(PCSTR moduleName, DWORD64 baseOfDll, PVOID thiS);
  //static BOOL CALLBACK traceEvents(HANDLE process, ULONG actionCode, ULONG64 callbackData, ULONG64 userContext);
};

