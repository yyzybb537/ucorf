/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.net/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/

/**********************************************************\
 *                                                        *
 * CTime.hpp                                              *
 *                                                        *
 * hprose c time unit for cpp.                            *
 *                                                        *
 * LastModified: May 29, 2014                             *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_COMMON_CTIME_HPP
#define HPROSE_COMMON_CTIME_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "config.hpp"

namespace hprose {
    
class CTime {
public: // structors

    CTime() throw()
      : time(0) {
    }

    CTime(time_t time) throw()
      : time(time) {
    }

    CTime(int year, int month, int day, int hour, int min, int sec, int dst = -1) {
        tm t;
        t.tm_year = year - 1900;
        t.tm_mon = month - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = min;
        t.tm_sec = sec;
        t.tm_isdst = dst;
        time = mktime(&t);
    }

public: // operators

    inline CTime & operator=(time_t time) throw() {
        this->time = time;
        return (*this);
    }

    inline CTime & operator+=(time_t span) throw() {
        time += span;
        return (*this);
    }

    inline CTime & operator-=(time_t span) throw() {
        time -= span;
        return (*this);
    }

    inline CTime operator+(time_t span) const throw() {
        return CTime(time + span);
    }

    inline CTime operator-(time_t span) const throw() {
        return CTime(time - span);
    }

    inline time_t operator-(CTime time) const throw() {
        return this->time - time.time;
    }

    inline bool operator==(CTime time) const throw() {
        return this->time == time.time;
    }

    inline bool operator!=(CTime time) const throw() {
        return this->time != time.time;
    }

    inline bool operator>(CTime time) const throw() {
        return this->time > time.time;
    }

    inline bool operator<(CTime time) const throw() {
        return this->time < time.time;
    }

    inline bool operator>=(CTime time) const throw() {
        return this->time >= time.time;
    }

    inline bool operator<=(CTime time) const throw() {
        return this->time <= time.time;
    }

public: // apis

    inline static CTime Now() throw() {
        return CTime(::time(0));
    }

    static long TimeZone() {
        static bool set;
        if (!set) {
            _tzset();
            set = true;
        }
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
        long timezone;
        _get_timezone(&timezone);
        return timezone;
#else
        return ::timezone;
#endif
    }

    inline time_t GetTime() const throw() {
        return time;
    }

    tm * GetGmtTm(tm * ptm = NULL) const {
#ifdef BOOST_WINDOWS
#ifdef __STDC_WANT_SECURE_LIB__
            gmtime_s(ptm, &time);
#else
            *ptm = *gmtime(&time);
#endif
#else
            gmtime_r(&time, ptm);
#endif
            return ptm;
    }

    tm * GetLocalTm(tm * ptm = NULL) const {
#ifdef BOOST_WINDOWS
#ifdef __STDC_WANT_SECURE_LIB__
            localtime_s(ptm, &time);
#else
            *ptm = *localtime(&time);
#endif
#else
            localtime_r(&time, ptm);
#endif
            return ptm;
    }

    std::string ToString() { // ToDo
#ifdef BOOST_WINDOWS
#ifdef __STDC_WANT_SECURE_LIB__
            char buf[30];
            ctime_s(buf, 30, &time);
            return buf;
#else
            return ctime(&t);
#endif
#else
            char buf[30];
            return ctime_r(&time, buf);
#endif
        }

private:

    time_t time;

}; // CTime

}  // namespace hprose

#endif // HPROSE_COMMON_CTIME_HPP
