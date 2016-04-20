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
 * HproseFormatter.hpp                                    *
 *                                                        *
 * hprose formatter unit for cpp.                         *
 *                                                        *
 * LastModified: Jun 15, 2014                             *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_IO_HPROSE_FORMATTER_HPP
#define HPROSE_IO_HPROSE_FORMATTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "HproseWriter.hpp"
#include "HproseReader.hpp"

namespace hprose {

class HproseFormatter {
public:

    template<typename ValueType>
    inline static void Serialize(const ValueType & value, std::ostream & stream, bool simple = false) {
        HproseWriter writer(stream, simple);
        writer.Serialize(value);
    }

    template<typename ValueType>
    inline static std::string Serialize(const ValueType & value, bool simple = false) {
        std::ostringstream stream;
        HproseWriter writer(stream, simple);
        writer.Serialize(value);
        return stream.str();
    }

#ifndef BOOST_NO_INITIALIZER_LISTS
    template<typename ValueType>
    inline static void Serialize(const std::initializer_list<ValueType> & value, std::ostream & stream, bool simple = false) {
        HproseWriter writer(stream, simple);
        writer.Serialize(value);
    }

    template<typename ValueType>
    inline static std::string Serialize(const std::initializer_list<ValueType> & value, bool simple = false) {
        std::ostringstream stream;
        HproseWriter writer(stream, simple);
        writer.Serialize(value);
        return stream.str();
    }
#endif

    template<typename ReturnType>
    inline static ReturnType Unserialize(std::istream & stream, bool simple = false) {
        HproseReader reader(stream, simple);
        return reader.Unserialize<ReturnType>();
    }

    inline static Any Unserialize(std::istream & stream, bool simple = false) {
        return Unserialize<Any>(stream, simple);
    }

    template<typename ReturnType>
    inline static ReturnType Unserialize(const std::string & data, bool simple = false) {
        std::istringstream stream(data);
        return Unserialize<ReturnType>(stream, simple);
    }

    inline static Any Unserialize(const std::string & data, bool simple = false) {
        return Unserialize<Any>(data, simple);
    }

}; // class HproseFormatter

} // namespace hprose

#endif // HPROSE_IO_HPROSE_FORMATTER_HPP
