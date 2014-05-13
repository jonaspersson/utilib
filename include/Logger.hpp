#pragma once
//  (C) Copyright 2008-2014 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  https://github.com/jonaspersson/utilib

#include <string>
#include <vector>
#include <memory>

namespace utilib {
  enum Setup { Abort = (1<<0), OneLine = (1<<1), TimeStamp = (1<<2), Callstack = (1<<3), HideExpr = (1<<4), ShowLine = (1<<5), HideData = (1<<6) };
  inline Setup operator|(Setup lhs,Setup rhs) { return static_cast<Setup>((int)lhs|(int)rhs); }
  enum class Severity { Debug, Info, Warning, Error, Critical, Fatal };
  struct LoggerI {
	  virtual ~LoggerI() {}
	  virtual void log(std::string const& name,Severity severity,Setup setup,const char* file,int line,const char* func,std::vector<std::string> const& vars,const char* stmt) = 0;
  };
  class ConsoleLogger : public LoggerI {
	  void log(std::string const& name,Severity severity,Setup setup,const char* file,int line,const char* func,std::vector<std::string> const& vars,const char* stmt) override;
  };
  class FileLogger : public LoggerI {
  public:
	  FileLogger(std::string file) : file_(move(file)) {}
  private:
	  void log(std::string const& name,Severity severity,Setup setup,const char* file,int line,const char* func,std::vector<std::string> const& vars,const char* stmt) override;
	  std::string file_;
  };
  class Logger
  {
    public:
    Logger(std::string name,Severity severity=Severity::Error,Setup setup=Setup::ShowLine);
    void operator()(const char* file,int line,const char* func,std::vector<std::string> const& vars,const char* stmt);
	static void addLogger(std::unique_ptr<LoggerI>);
    private:
    std::string name_;
    Severity severity_;
    Setup setup_;
	static std::vector<std::unique_ptr<LoggerI>>& loggers();
  };
}
