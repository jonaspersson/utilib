#pragma once
//  (C) Copyright 2008-2014 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  https://github.com/jonaspersson/utilib

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <memory>
#include <cstdint>

#ifndef _WIN32
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#endif
#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif

#include <map>
#include <deque>
namespace std {
	wostream& operator<<(wostream& os, string const& str);
	ostream& operator<<(ostream& os, wstring const& str);
}
	namespace details {
      inline bool do_check(bool cond) { return cond; }

      class Base {
        protected:
          const char* file_;
          int line_;
          const char* func_;
          const char* stmt_;
          mutable std::vector<std::string> vars_;
        public:
          Base(const char* file,int line,const char* func,const char* stmt) 
            : file_(file),line_(line),func_(func),stmt_(stmt),vars_() {}
          operator bool() const { return false; }
          template<typename T>
            void var(const char* c,T const& t) const { 
              std::ostringstream os; 
              //if(::strchr(c,'"'))
              if(c && *c=='"')
                os << "Note: " << t; 
              else
                os << c << " = " << t; 
              vars_.push_back(os.str());
            }
      }; // Base -----------------------------------------------------------------

      template<typename Op>
        struct Impl : Base {
          Impl(Op op, const char* file,int line,const char* func,const char* stmt) 
            : Base(file,line,func,stmt)
            , op_(std::move(op))
          {}
          ~Impl() { if(!!file_) op_(file_, line_, func_, vars_, stmt_); }
          Op op_;
		  Impl(Impl&& other) : Base(other), op_(std::move(other.op_)) { other.file_ = nullptr; }
		private: // disable
          Impl(Impl const&);
        };
      template<typename Op>
        Impl<Op> impl(Op op, const char* file,int line,const char* func,const char* stmt) { return Impl<Op>(op,file,line,func,stmt); }
     template<typename Op>
	 Impl<Op> impl(Op op, const char* file,int line,const char* func,std::string const& stmt) { return Impl<Op>(op,file,line,func,stmt.c_str()); }
}

#define LOG_IMPL_B(x) LOG_IMPL_OP(x, A)
#define LOG_IMPL_A(x) LOG_IMPL_OP(x, B)
#define LOG_IMPL_OP(x, next) LOG_IMPL_B.var(#x,(x)), (void)LOG_IMPL_ ## next

// LOG_IF(x)  - calls a logging function if x is true
#define LOG_IF_IMPL(expr,op) \
if ( ::details::do_check(!(expr)) ); else \
if ( auto const LOG_IMPL_B = \
  ::details::impl(op,__FILE__,__LINE__,__FUNCTION__,#expr)); else \
  if ( auto const& LOG_IMPL_A = LOG_IMPL_B ); else \
     (void)LOG_IMPL_A, (void)LOG_IMPL_A

// LOG_UNLESS(x)  - calls a logging function if x is false
#define LOG_UNLESS_IMPL(expr,op) \
if ( ::details::do_check(!!(expr)) ); else \
if ( auto const LOG_IMPL_B = \
  ::details::impl(op,__FILE__,__LINE__,__FUNCTION__,#expr)); else \
  if ( auto const& LOG_IMPL_A = LOG_IMPL_B ); else \
     (void)LOG_IMPL_A, (void)LOG_IMPL_A

// LOG_MESSAGE(x)  - passes string x to logging function
#define LOG_MESSAGE_IMPL(expr,op) \
if ( auto const LOG_IMPL_B = \
  ::details::impl(op,__FILE__,__LINE__,__FUNCTION__,expr)); else \
  if ( auto const& LOG_IMPL_A = LOG_IMPL_B ); else \
     (void)LOG_IMPL_A, (void)LOG_IMPL_A

enum {LOG_NOOP_A, LOG_NOOP_B };
#define LOG_NOOP_B(x) LOG_NOOP_OP(x, A)
#define LOG_NOOP_A(x) LOG_NOOP_OP(x, B)
#define LOG_NOOP_OP(x, next) LOG_NOOP_ ## next
#define LOG_NOOP if (true); else (void)LOG_NOOP_B, (void)LOG_NOOP_A
