//  (C) Copyright 2008-2014 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "Logger.hpp"
#include "CallStack.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <mutex>

#ifdef _WIN32
#include <stdio.h>
#include <io.h>
#include <windows.h>
#endif

#include <iostream>
#include <boost/filesystem/fstream.hpp>

namespace fs = boost::filesystem;

#ifdef _WIN32
class ConsoleColor
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	WORD       wAttributes;
	HANDLE hStdout;
public:
	ConsoleColor(utilib::Severity s) {
		int c = 0;
		switch(s)
		{
		case utilib::Severity::Debug:     c = FOREGROUND_GREEN; break;
		case utilib::Severity::Info:      c = FOREGROUND_GREEN|FOREGROUND_RED; break;
		case utilib::Severity::Warning:   c = FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY; break;
		case utilib::Severity::Error:     c = FOREGROUND_RED|FOREGROUND_INTENSITY; break;
		case utilib::Severity::Critical:  c = FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY; break;
		case utilib::Severity::Fatal:     c = BACKGROUND_RED|BACKGROUND_INTENSITY; break;
		};
		hStdout=GetStdHandle(STD_OUTPUT_HANDLE); 
		::GetConsoleScreenBufferInfo(hStdout,&info);
		::SetConsoleTextAttribute(hStdout,c); 
	}
	ConsoleColor(ConsoleColor&&) : hStdout((HANDLE)-1) {}

	~ConsoleColor() {
		if(hStdout!=(HANDLE)-1)
			::SetConsoleTextAttribute(hStdout,info.wAttributes);
	}
};
#else
class ConsoleColor
{
  public:
    char const* c;
  mutable std::ostream *os;         
	ConsoleColor(utilib::Severity s) : c(nullptr), os(nullptr) {
      c = "\033[0m";
		switch(s)
		{
        case utilib::Severity::Debug:     c = "\033[32m"; break;
		case utilib::Severity::Info:      c = "\033[36m"; break;
		case utilib::Severity::Warning:   c = "\033[33m"; break;
		case utilib::Severity::Error:     c = "\033[31m"; break;
		case utilib::Severity::Critical:  c = "\033[35m"; break;
		case utilib::Severity::Fatal:     c = "\033[1;35m"; break;
		};
  }
    void operator()(std::ostream& o) const { 
      os = &o; 
      *os << c;
    }
	~ConsoleColor() {
      os && *os << "\033[0m";
  }
};
#endif

inline std::ostream& operator<< (std::ostream& s,const ConsoleColor& c)
{
#ifndef _WIN32

  c(s);
#endif
	return s;
}
namespace utilib {
	std::mutex& mx() {
		static std::mutex mx_;
		return mx_;
	}
	std::vector<std::unique_ptr<LoggerI>>& Logger::loggers()
	{
		static std::vector<std::unique_ptr<LoggerI>> loggers_;
		return loggers_;
	}
    Logger::Logger(std::string name,Severity severity,Setup setup) 
		: name_(std::move(name)), severity_(severity), setup_(setup)
	{
	}
	void Logger::addLogger(std::unique_ptr<LoggerI> log) 
	{
		std::lock_guard<std::mutex> lock(mx());
		loggers().push_back(move(log));
	}
	void Logger::operator()(const char* file,int line,const char* func,std::vector<std::string> const& vars,const char* stmt)
	{
		std::lock_guard<std::mutex> lock(mx());
#if 0 // _DEBUG
		if(loggers().empty())
			assert(false);
#endif
		for(auto const& logger : loggers())
			logger->log(name_,severity_,setup_,file,line,func,vars,stmt);
		if(setup_ & Abort) 
		{
			//fout << "Aborting..." << std::endl;
#if defined _WIN32 &&  defined(_M_IX86)
			__asm int 3;
			_exit(3);
#else
			abort();
			//std::quick_exit(3);
#endif
		}
		//std::abort();
	}
	void ConsoleLogger::log(std::string const& name,Severity severity,Setup setup,const char* file,int line,const char* func,std::vector<std::string> const& vars,const char* stmt)
	{
		//std::lock_guard<std::mutex> lock(mx);
		if(setup & OneLine) {
			ConsoleColor severityColor(severity);
      std::cout << severityColor;
			std::cout << name << ":";
			if(setup & ShowLine) std::cout << "  " << file << ":" << std::dec << line << "   " << func;
			if(!(setup & HideExpr)) std::cout << "  " << stmt;
      if(!(setup & HideData))
        for(auto& s : vars)
          std::cout << " [" << s << "]";
			std::cout << std::endl;
		} else {
			ConsoleColor severityColor(severity);
      std::cout << severityColor;
			std::cout << name << ":";
			if(!(setup & HideExpr))
        std::cout << "  " << stmt;
			std::cout << std::endl;
			if(setup & ShowLine) std::cout << "  " << file << ":" << std::dec << line << "   " << func << std::endl;
      if(!(setup & HideData))
        for(auto& s : vars)
          std::cout << "   " << s << std::endl;
      if(setup & Callstack)
          std::cout << CallStack(3) << std::endl;
			//std::cout << "   " << vars << std::endl;
		}
		if(setup & Abort) {
			std::cout << "Aborting..." << std::endl;
#if defined _WIN32 &&  defined(_M_IX86)
			__asm int 3;
			_exit(3);
#else
        abort();
      //std::quick_exit(3);
#endif
		}
		//std::abort();
	}
	void FileLogger::log(std::string const& name,Severity severity,Setup setup,const char* file,int line,const char* func,std::vector<std::string> const& vars,const char* stmt)
	{
		fs::path p(file_);
		fs::ofstream fout(p,std::ios_base::out|std::ios_base::app);
		//std::lock_guard<std::mutex> lock(mx);
		if(setup & OneLine) {
			fout << name << ":";
			if(setup & ShowLine) fout << "  " << file << ":" << std::dec << line << "   " << func;
			if(!(setup & HideExpr)) fout << "  " << stmt;
			if(!(setup & HideData))
				for(auto& s : vars)
					fout << " [" << s << "]";
			fout << std::endl;
		} else {
			fout << name << ":";
			if(!(setup & HideExpr))
				fout << "  " << stmt;
			fout << std::endl;
			if(setup & ShowLine) fout << "  " << file << ":" << std::dec << line << "   " << func << std::endl;
			if(!(setup & HideData))
				for(auto& s : vars)
					fout << "   " << s << std::endl;
			if(setup & Callstack)
				fout << CallStack(3) << std::endl;
			//std::cout << "   " << vars << std::endl;
		}
	}
}

