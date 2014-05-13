#pragma once
//  (C) Copyright 2008-2013 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifdef max
# undef max
#endif
#ifdef min
# undef min
#endif
#define BOOST_CHRONO_VERSION 2
#include <boost/chrono.hpp>  // should use std::chrono, but msvc use a low res timer.
#include <map>
#include <mutex>

class PerfLog
{
  public:
  typedef boost::chrono::high_resolution_clock::duration duration;
  private:
  struct Log {
    duration min;
    duration max;
    duration total;
    unsigned count;
    Log()
      : min(duration::max())
      , max(duration::min())
      , total(duration::zero())
      , count(0)
    {}
  };
  std::map<std::string,Log> logs;
  mutable std::mutex mx;
	  PerfLog(PerfLog const&); // = delete
  public:
	  PerfLog() {}
  void operator()(std::string const& id, duration d)
  { 
	  assert(d>=duration::zero());
    std::unique_lock<std::mutex> lk(mx);
    auto& log = logs[id];
    if(d<log.min) log.min=d;
    if(d>log.max) log.max=d;
    log.total += d;
    ++ log.count;
  }
  friend std::ostream& operator << (std::ostream& os, PerfLog const& lhs) {
	  std::unique_lock<std::mutex> lk(lhs.mx);
	  using namespace boost::chrono;
	  if(!lhs.logs.empty())
		  os << "label: min | max | mean | total | count" << std::endl;
	  for(auto const& c: lhs.logs)
		  if(c.second.min < milliseconds(5))
			  os /*<< symbol_format*/ << c.first << ": " 
			  << duration_cast<microseconds>(c.second.min) << " | " 
			  << duration_cast<microseconds>(c.second.max) << " | " 
			  << duration_cast<microseconds>(c.second.total/c.second.count) << " | " 
			  << duration_cast<microseconds>(c.second.total) << " | " 
			  << c.second.count << std::endl;

		  else
			  os /*<< symbol_format*/ << c.first << ": " 
			  << duration_cast<milliseconds>(c.second.min) << " | " 
			  << duration_cast<milliseconds>(c.second.max) << " | " 
			  << duration_cast<milliseconds>(c.second.total/c.second.count) << " | " 
			  << duration_cast<milliseconds>(c.second.total) << " | " 
			  << c.second.count << std::endl;
	  return os;
  }
};
template<typename Log>
class MeasureScope
{
  typedef boost::chrono::high_resolution_clock clock;
  typedef clock::duration duration;
  typedef clock::time_point time_point;
  std::string id;
  time_point start;
  Log log;
  duration threshold;
  MeasureScope(MeasureScope const& other); // = delete
  MeasureScope& operator=(MeasureScope const& other); // = delete
  public:
  MeasureScope(std::string id,Log log,duration threshold) 
    : id(std::move(id))
    , start(clock::now()) 
    , log(std::move(log))
    , threshold(threshold)
  {
  }
  MeasureScope(MeasureScope&& other) 
	      : id(std::move(other.id))
    , start(std::move(other.start)) 
    , log(std::move(other.log))
	, threshold(std::move(other.threshold))
  {
	  other.threshold = duration::max();
  }
  ~MeasureScope() {
    auto t = clock::now()-start;
    if(threshold<t)
      (log)(id,t);
  }      
};
template<typename Log>
MeasureScope<Log> make_MeasureScope(std::string id,Log log,PerfLog::duration threshold=boost::chrono::seconds(0)) { return MeasureScope<Log>(id,log,threshold); }
