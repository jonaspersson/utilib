//  (C) Copyright 2008-2013 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
#ifdef _WIN32

#include "DbgHelp.hpp"
#include <iostream>
#include <boost/shared_ptr.hpp>

#if !defined(_M_IX86) && !defined(_M_X64)
#error("Wrong Target Machine! Only Intel 32 bit architecture allowed!");
#endif

const wchar_t* const cDbgHelpDllPath = L"dbghelp.dll";            // path to dbgHelp.dll, the dll is expected to be in the same path as the exe

boost::weak_ptr<DbgHelp>  DbgHelp::mInstance;
boost::mutex            DbgHelp::mInstanceMutex;


boost::shared_ptr<DbgHelp> DbgHelp::instance()
{
  boost::mutex::scoped_lock lock(mInstanceMutex);
  boost::shared_ptr<DbgHelp> inst = mInstance.lock();

  if(!inst.get())
  {
    inst = boost::shared_ptr<DbgHelp>(new DbgHelp(::GetCurrentProcess()));
    mInstance = inst;
  }

  return inst;
}


DbgHelp::DbgHelp(HANDLE process)
: mProcess(process)
, mDbgHelp(0)
, mMutex()
, mSymGetModuleInfo64(0)  
, mSymLoadModule64(0)   
, mSymInitialize(0)   
, mSymUnloadModule64(0)
, mSymEnumerateModules64(0)
, mSymCleanup(0)  
, mSymFunctionTableAccess64(0)
, mSymGetModuleBase64(0)
, mSymGetSymFromAddr64(0)
, mSymSetOptions(0)     
, mSymGetOptions(0)     
, mUnDecorateSymbolName(0)  
, mSymUnDName64(0)  
, mStackWalk64(0)
, mMiniDumpWriteDump(0)
//, mSymRegisterCallback64(0)
// currently not used:
//, mSymEnumSymbols(0)
//, mSymEnumTypes(0)
//, mSymSetContext(0)
//, mSymGetTypeInfo(0)
//, mImagehlpApiVersion(0)
{
  if(!mProcess)
  {
    throw StackDumpException(__FILE__, __LINE__, "DbgHelp: invalid process handle passed");
  }

  //
  // Load dbgHelp.dll and map the functions we need
  //
  if((mDbgHelp = ::LoadLibraryW(cDbgHelpDllPath)) == 0)
  {
    throw StackDumpException(__FILE__, __LINE__, "DbgHelp: Failed to load dbgHelp.dll");        
  }

  // Map function pointers
  if(0 == (mSymGetModuleInfo64    = reinterpret_cast<SymGetModuleInfo64Fnc>   (GetProcAddress(mDbgHelp, "SymGetModuleInfo64"))))      throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymGetModuleInfo64");
  if(0 == (mSymLoadModule64     = reinterpret_cast<SymLoadModule64Fnc>      (GetProcAddress(mDbgHelp, "SymLoadModule64"))))       throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymLoadModule64");     
  if(0 == (mSymInitialize       = reinterpret_cast<SymInitializeFnc>      (GetProcAddress(mDbgHelp, "SymInitialize"))))       throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymInitialize");     
  if(0 == (mSymUnloadModule64     = reinterpret_cast<SymUnloadModule64Fnc>    (GetProcAddress(mDbgHelp, "SymUnloadModule64"))))     throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymUnloadModule64");   
  if(0 == (mSymEnumerateModules64   = reinterpret_cast<SymEnumerateModules64Fnc>  (GetProcAddress(mDbgHelp, "SymEnumerateModules64"))))   throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymEnumerateModules64");
  if(0 == (mSymCleanup        = reinterpret_cast<SymCleanupFnc>       (GetProcAddress(mDbgHelp, "SymCleanup"))))          throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymCleanup");        
  if(0 == (mSymFunctionTableAccess64  = reinterpret_cast<SymFunctionTableAccess64Fnc> (GetProcAddress(mDbgHelp, "SymFunctionTableAccess64"))))  throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymFunctionTableAccess64");  
  if(0 == (mSymGetModuleBase64    = reinterpret_cast<SymGetModuleBase64Fnc>   (GetProcAddress(mDbgHelp, "SymGetModuleBase64"))))      throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymGetModuleBase64");    
  if(0 == (mSymGetSymFromAddr64   = reinterpret_cast<SymGetSymFromAddr64Fnc>    (GetProcAddress(mDbgHelp, "SymGetSymFromAddr64"))))     throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymGetSymFromAddr64");   
  if(0 == (mSymSetOptions       = reinterpret_cast<SymSetOptionsFnc>      (GetProcAddress(mDbgHelp, "SymSetOptions"))))       throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymSetOptions");     
  if(0 == (mSymGetOptions       = reinterpret_cast<SymGetOptionsFnc>      (GetProcAddress(mDbgHelp, "SymGetOptions"))))       throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymGetOptions");     
  if(0 == (mUnDecorateSymbolName    = reinterpret_cast<UnDecorateSymbolNameFnc>   (GetProcAddress(mDbgHelp, "UnDecorateSymbolName"))))    throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: UnDecorateSymbolName");    
  if(0 == (mSymUnDName64        = reinterpret_cast<SymUnDName64Fnc>       (GetProcAddress(mDbgHelp, "SymUnDName64"))))        throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymUnDName64");        
  if(0 == (mStackWalk64       = reinterpret_cast<StackWalk64Fnc>        (GetProcAddress(mDbgHelp, "StackWalk64"))))         throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: StackWalk64");       
  if(0 == (mMiniDumpWriteDump = reinterpret_cast<MiniDumpWriteDumpFnc>  (GetProcAddress(mDbgHelp, "MiniDumpWriteDump"))))         throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: MiniDumpWriteDump");       
  //if(0 == (mSymRegisterCallback64   = reinterpret_cast<SymRegisterCallback64Fnc>  (GetProcAddress(mDbgHelp, "SymRegisterCallback64"))))   throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymRegisterCallback64"); 
// currently not used:
//  if(0 == (mSymEnumSymbols      = reinterpret_cast<SymEnumTypesFnc>       (GetProcAddress(mDbgHelp, "SymEnumSymbols"))))        throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymEnumSymbols");        
//  if(0 == (mSymEnumTypes        = reinterpret_cast<SymEnumSymbolsFnc>     (GetProcAddress(mDbgHelp, "SymEnumTypes"))))        throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymEnumTypes");      
//  if(0 == (mSymSetContext       = reinterpret_cast<SymSetContextFnc>      (GetProcAddress(mDbgHelp, "SymSetContext"))))       throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymSetContext");
//  if(0 == (mSymGetTypeInfo      = reinterpret_cast<SymGetTypeInfoFnc>     (GetProcAddress(mDbgHelp, "SymGetTypeInfo"))))        throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: SymGetTypeInfo");      
//  if(0 == (mImagehlpApiVersion    = reinterpret_cast<ImagehlpApiVersionFnc>   (GetProcAddress(mDbgHelp, "ImagehlpApiVersion"))))      throw StackDumpException(__FILE__, __LINE__, "Function not found in dbghelp.dll: ImagehlpApiVersion");    

  //
  // Initialize dbgHelp lib
  //
  if(!SymInitialize(0, false))            // set no user path since we're manually loading all modules
  {
    ::FreeLibrary(mDbgHelp);
    // don't use NcErrorFacade to throw, since it would try to log a callstack
    throw StackDumpException(__FILE__, __LINE__, "DbgHelp: Failed to initialize");        
  }

  // Set options
  DWORD symOptions = SymGetOptions();
  symOptions &= ~SYMOPT_UNDNAME;        // We undecorate names manually since that enable us to get more info
  symOptions |= SYMOPT_PUBLICS_ONLY;      // Needed to get types of function parameters, sacrificing access to private and local info
//  symOptions |= SYMOPT_DEBUG;
  SymSetOptions(symOptions);
//  SymRegisterCallback64(traceEvents, 0);
}

DbgHelp::~DbgHelp()
{
  try
  {
    boost::mutex::scoped_lock lock(mInstanceMutex);

    // Unload all modules loades to free memory
    if (!SymEnumerateModules64(unloadModule, this))
    {
      std::cerr << "~DbgHelp(), SymEnumerateModules64 failed, GetLastError = " <<  GetLastError();

    }

    // Let dbgHelp release resources
    if(!SymCleanup())
    {
      std::cerr << "~DbgHelp(), SymCleanup failed, GetLastError = " <<  GetLastError();
    }

    // Unload dbgHelp.dll
    ::FreeLibrary(mDbgHelp);
  }
  catch(...)
  {
    // better safe than sorry...
  }
}

BOOL CALLBACK DbgHelp::unloadModule(PCSTR /*moduleName*/, DWORD64 baseOfDll, PVOID thiS)
{
  if(thiS)
  {
    reinterpret_cast<DbgHelp*>(thiS)->SymUnloadModule64(baseOfDll);
  }

  return TRUE;
}

BOOL DbgHelp::SymGetModuleInfo64(DWORD64 address, PIMAGEHLP_MODULE64 moduleInfo) const
{
  Guard lock(mutex());

  return mSymGetModuleInfo64(mProcess, address, moduleInfo);
}

DWORD64 DbgHelp::SymLoadModule64(HANDLE file, PCSTR imageName, PCSTR moduleName, DWORD64 baseOfDll, DWORD sizeOfDll) const
{
  Guard lock(mutex());

  return mSymLoadModule64(mProcess, file, imageName, moduleName, baseOfDll, sizeOfDll);
}

BOOL DbgHelp::SymInitialize(PCSTR searchPath, BOOL invadeProcess) const
{
  Guard lock(mutex());

  return mSymInitialize(mProcess, searchPath, invadeProcess);
}

BOOL DbgHelp::SymUnloadModule64(DWORD64 baseOfDll) const
{
  Guard lock(mutex());

  return mSymUnloadModule64(mProcess, baseOfDll);
}

BOOL DbgHelp::SymEnumerateModules64(PSYM_ENUMMODULES_CALLBACK64 enumModulesCallback, PVOID userContext) const
{
  Guard lock(mutex());

  return mSymEnumerateModules64(mProcess, enumModulesCallback, userContext);
}

BOOL DbgHelp::SymCleanup() const
{
  Guard lock(mutex());

  return mSymCleanup(mProcess);
}

PVOID DbgHelp::SymFunctionTableAccess64(DWORD64 addressBase) const
{
  Guard lock(mutex());

  return mSymFunctionTableAccess64(mProcess, addressBase);
}

DWORD64 DbgHelp::SymGetModuleBase64(DWORD64 address) const
{
  Guard lock(mutex());

  return mSymGetModuleBase64(mProcess, address);
}

BOOL DbgHelp::SymGetSymFromAddr64(DWORD64 address, PDWORD64 displacement, PIMAGEHLP_SYMBOL64 symbol) const
{
  Guard lock(mutex());

  return mSymGetSymFromAddr64(mProcess, address, displacement, symbol);
}

DWORD DbgHelp::SymSetOptions(DWORD options) const
{
  Guard lock(mutex());

  return mSymSetOptions(options);
}

DWORD DbgHelp::SymGetOptions() const
{
  Guard lock(mutex());

  return mSymGetOptions();
}

DWORD DbgHelp::UnDecorateSymbolName(PCSTR decoratedName, PSTR undecoratedName, DWORD undecoratedLength, DWORD flags) const
{
  Guard lock(mutex());

  return mUnDecorateSymbolName(decoratedName, undecoratedName, undecoratedLength, flags);
}

BOOL DbgHelp::SymUnDName64(PIMAGEHLP_SYMBOL64 symbol, PSTR undecoratedName, DWORD undecoratedLength) const
{
  Guard lock(mutex());

  return mSymUnDName64(symbol, undecoratedName, undecoratedLength);
}

// BOOL DbgHelp::SymRegisterCallback64(PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction, ULONG64 UserContext) const
// {
//  Guard lock(mutex());
//  return mSymRegisterCallback64(mProcess, CallbackFunction, UserContext);
// }

BOOL DbgHelp::StackWalk64(DbgHelp::Guard& /*lock*/, HANDLE thread, LPSTACKFRAME64 stackFrame, PVOID contextRecord) const
{
//  if(!lock.locked())
//  {
//    throw StackDumpException(__FILE__, __LINE__, "DbgHelp::StackWalk64() Tried to execute without lock!");        
//  }

  // TODO: Q: Is the lock locked on the correct instance? A: No need to check since this is a singleton...

#if defined(_M_IX86)
  return mStackWalk64(IMAGE_FILE_MACHINE_I386, mProcess, thread, stackFrame, contextRecord, 0, mSymFunctionTableAccess64, getModuleBase, 0);
#elif defined(_M_X64)
  return mStackWalk64(IMAGE_FILE_MACHINE_AMD64, mProcess, thread, stackFrame, contextRecord, 0, mSymFunctionTableAccess64, getModuleBase, 0);
  #else
#error unsupported architecture
#endif
}

BOOL DbgHelp::MiniDumpWriteDump(DWORD ProcessId, HANDLE hFile, 
  MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
  PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, 
  PMINIDUMP_CALLBACK_INFORMATION CallbackParam) const
{
  Guard lock(mutex());

  return mMiniDumpWriteDump(mProcess, ProcessId, hFile, 
    DumpType, ExceptionParam, UserStreamParam, CallbackParam);
}

//
// Load the module for requested address
//
DWORD64 CALLBACK DbgHelp::getModuleBase(HANDLE process, DWORD64 address)
{
  IMAGEHLP_MODULE64 moduleInfo;
  moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
  DWORD64 rc = 0;

  // Check if present already
  if(DbgHelp::instance()->SymGetModuleInfo64(address, &moduleInfo))
  {
    rc = moduleInfo.BaseOfImage;
  }
  else
  {
    // Find out the name of the module at requested address
    MEMORY_BASIC_INFORMATION memoryBasicInfo;

    if(::VirtualQueryEx(process, reinterpret_cast<void*>(address), &memoryBasicInfo, sizeof(MEMORY_BASIC_INFORMATION)))
    {
      char file[MAX_PATH] = { 0 };
      const DWORD cch = ::GetModuleFileNameA(static_cast<HINSTANCE>(memoryBasicInfo.AllocationBase), file, MAX_PATH);

      // Load the module, ignore the return code since we can't do anything with it.
      DbgHelp::instance()->SymLoadModule64(0, cch ? file : 0, 0, reinterpret_cast<DWORD64>(memoryBasicInfo.AllocationBase), 0);
      rc = reinterpret_cast<DWORD64>(memoryBasicInfo.AllocationBase);
    }
  }

  return rc;
}

// BOOL CALLBACK DbgHelp::traceEvents(HANDLE /*process*/, ULONG actionCode, ULONG64 callbackData, ULONG64 /*userContext*/)
// {
//  Guard lock(DbgHelp::instance()->mutex());
// 
//  switch(actionCode)
//  {
//    case CBA_DEBUG_INFO:            std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_DEBUG_INFO: " << (const char*) callbackData; break;
// //   case CBA_DEFERRED_SYMBOL_LOAD_CANCEL:   std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_DEFERRED_SYMBOL_LOAD_CANCEL" << std::endl; return FALSE;
//    case CBA_DEFERRED_SYMBOL_LOAD_COMPLETE:   std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_DEFERRED_SYMBOL_LOAD_COMPLETE:  " << ((IMAGEHLP_DEFERRED_SYMBOL_LOAD64*) callbackData)->FileName << std::endl;break;
//    case CBA_DEFERRED_SYMBOL_LOAD_FAILURE:    std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_DEFERRED_SYMBOL_LOAD_FAILURE:  " << ((IMAGEHLP_DEFERRED_SYMBOL_LOAD64*) callbackData)->FileName << std::endl;break;
// //   case CBA_DEFERRED_SYMBOL_LOAD_PARTIAL:    std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_DEFERRED_SYMBOL_LOAD_PARTIAL:  " << ((IMAGEHLP_DEFERRED_SYMBOL_LOAD64*) callbackData)->FileName << std::endl;break;
//    case CBA_DEFERRED_SYMBOL_LOAD_START:    std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_DEFERRED_SYMBOL_LOAD_START:  " << ((IMAGEHLP_DEFERRED_SYMBOL_LOAD64*) callbackData)->FileName << std::endl;break;
//    case CBA_DUPLICATE_SYMBOL:          std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_DUPLICATE_SYMBOL: " << ((IMAGEHLP_DUPLICATE_SYMBOL64*) callbackData)->Symbol[0].Name << std::endl; break;
// //   case CBA_EVENT:               std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_EVENT" << std::endl; break;
// //   case CBA_READ_MEMORY:           std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_READ_MEMORY" << std::endl; break;
//    case CBA_SET_OPTIONS:           std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_SET_OPTIONS" << std::endl; break;
//    case CBA_SYMBOLS_UNLOADED:          std::cerr << std::hex << ::GetCurrentThreadId() << "CBA_SYMBOLS_UNLOADED" << std::endl; break;
//    default:                  std::cerr << "Unknown event!" << std::endl; break;
//  }
// 
//  return FALSE;
// }
//}
#endif
