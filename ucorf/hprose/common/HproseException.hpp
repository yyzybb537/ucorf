/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/

/**********************************************************\
 *                                                        *
 * HproseException.hpp                                    *
 *                                                        *
 * hprose exception unit for cpp.                         *
 *                                                        *
 * LastModified: Jun 15, 2014                             *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_COMMON_HPROSE_EXCEPTION_HPP
#define HPROSE_COMMON_HPROSE_EXCEPTION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <hprose/config.hpp>

#include <boost/throw_exception.hpp>

#ifdef HPROSE_DEBUG_MODE
#define HPROSE_EXCEPTION(s) hprose::HproseException(s, __FILE__, __LINE__)
#else
#define HPROSE_EXCEPTION(s) hprose::HproseException(s)
#endif

// HPROSE_THROW_EXCEPTION(s) = boost::throw_exception(hprose::HproseException(s))
// Use HPROSE_THROW_EXCEPTION instead of boost::throw_exception can avoid some warnings
#ifndef BOOST_EXCEPTION_DISABLE
#define HPROSE_THROW_EXCEPTION(s) throw boost::enable_current_exception(boost::enable_error_info(HPROSE_EXCEPTION(s)))
#else
#define HPROSE_THROW_EXCEPTION(s) throw HPROSE_EXCEPTION(s)
#endif

namespace hprose {

class HproseException : public std::exception {
public:

#ifdef HPROSE_DEBUG_MODE
    HproseException(const std::string & message, const char * file, unsigned int line) throw() {
        std::ostringstream stream;
        stream << "[" << file << ":" << line << "] " << message;
        this->message = stream.str();
    }
#endif

    HproseException(const std::string & message) throw()
      : message(message) {
    }

    virtual ~HproseException() throw() {
    }

    virtual const char * what() const throw() {
        return message.c_str();
    }

private:

    std::string message;

}; // class HproseException

} // namespace hprose

#endif // HPROSE_COMMON_HPROSE_EXCEPTION_HPP
