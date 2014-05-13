#pragma once
//  (C) Copyright 2008-2014 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  https://github.com/jonaspersson/utilib

#include <utility>

namespace utilib {
  template<typename Op>
    class OnScopeExit
    {
      public:
        OnScopeExit(Op const& op)
          : op(op)
          , valid(true)
        {}
        OnScopeExit(OnScopeExit&& other)
          : op(std::move(other.op))
          , valid(other.valid)
        {
          other.valid = false;
        }
        OnScopeExit& operator=(OnScopeExit&& other)
        {
          valid = other.valid;
          other.valid = false;
          op = std::move(other.op);
        }
        ~OnScopeExit()
        {
          if(valid)
            op();
        }
      private: // disabled ops
        OnScopeExit(OnScopeExit const&);
        OnScopeExit& operator=(OnScopeExit const&);
      private:
        Op op;
        bool valid;
    };
  template<typename Op>
    OnScopeExit<Op> onScopeExit(Op const& op) { return OnScopeExit<Op>(op); }
}
