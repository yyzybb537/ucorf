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
 * HproseWriter.hpp                                       *
 *                                                        *
 * hprose writer unit for cpp.                            *
 *                                                        *
 * LastModified: Apr 27, 2015                             *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_IO_HPROSE_WRITER_HPP
#define HPROSE_IO_HPROSE_WRITER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <ostream>

#include "ClassManager.hpp"
#include "HproseTags.hpp"
#include "HproseTypes.hpp"

namespace hprose {

class HproseWriter {
public:

    HproseWriter(std::ostream & stream, bool simple = false)
        : stream(stream), simple(simple) {
    }

public:

    template<typename ValueType>
    inline void Serialize(const ValueType & value) {
        Serialize(value, NonCVType<ValueType>());
    }

    template<typename ValueType, size_t ArraySize>
    inline void Serialize(const ValueType (&value)[ArraySize]) {
        Serialize(value, NonCVArrayType<ValueType>());
    }

#ifndef BOOST_NO_INITIALIZER_LISTS
    template<typename ValueType>
    inline void Serialize(const std::initializer_list<ValueType> & value) {
        Serialize(value, NonCVListType<ValueType>());
    }
#endif

public:
    template <typename T>
    void Write(T t);

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value>::type Write(T t)
    {
        WriteInteger(t);
    }

    void Write(const char* t)
    {
        WriteString(t);
    }

    template<typename Element, typename Traits, typename Allocator>
    void Write(const std::basic_string<Element, Traits, Allocator> & s)
    {
        WriteString(s);
    }

    inline void WriteNull() {
        stream << HproseTags::TagNull;
    }

    inline void WriteEmpty() {
        stream << HproseTags::TagEmpty;
    }

    inline void WriteBool(const bool & b) {
        stream << (b ? HproseTags::TagTrue : HproseTags::TagFalse);
    }

    template<typename ValueType>
    inline void WriteEnum(const ValueType & e) {
        HPROSE_STATIC_ASSERT(NonCVType<ValueType>::value == EnumType::value, "Require EnumType");
        WriteInteger(static_cast<int>(e), SignedType());
    }

    template<typename ValueType>
    inline void WriteInteger(const ValueType & i) {
        HPROSE_STATIC_ASSERT(
            (NonCVType<ValueType>::value == ByteType::value) ||
            (NonCVType<ValueType>::value == CharType::value) ||
            (NonCVType<ValueType>::value == IntegerType::value), "Require [ByteType, CharType, IntegerType]");
        WriteInteger(i, std::is_signed<ValueType>());
    }

    template<typename ValueType>
    inline void WriteLong(const ValueType & l) { // Todo: BigInteger
        HPROSE_STATIC_ASSERT(
            (NonCVType<ValueType>::value == ByteType::value) ||
            (NonCVType<ValueType>::value == CharType::value) ||
            (NonCVType<ValueType>::value == IntegerType::value) ||
            (NonCVType<ValueType>::value == LongType::value), "Require [ByteType, CharType, IntegerType, LongType]");
        WriteLong(l, std::is_signed<ValueType>());
    }

    inline void WriteNaN() {
        stream << HproseTags::TagNaN;
    }

    inline void WriteInfinity(bool positive) {
        stream << HproseTags::TagInfinity << (positive ? HproseTags::TagPos : HproseTags::TagNeg);
    }

    template<typename ValueType>
    void WriteDouble(const ValueType & d) {
        HPROSE_STATIC_ASSERT(NonCVType<ValueType>::value == DoubleType::value, "Require DoubleType");
        if (d != d) {
            WriteNaN();
        } else if (d == std::numeric_limits<ValueType>::infinity()) {
            WriteInfinity(true);
        } else if (d == -std::numeric_limits<ValueType>::infinity()) {
            WriteInfinity(false);
        } else {
            stream.precision(std::numeric_limits<ValueType>::digits10);
            stream << HproseTags::TagDouble << d << HproseTags::TagSemicolon;
        }
    }

    template<typename ValueType>
    void WriteBytes(const ValueType & b, bool checkRef = true) {
        HPROSE_STATIC_ASSERT(NonCVType<ValueType>::value == BytesType::value, "Require BytesType");
        size_t n = b.size();
        if (WriteRef(&b, checkRef)) {
            stream << HproseTags::TagBytes;
            if (n > 0) WriteIntFast(n, stream);
            stream << HproseTags::TagQuote;
#ifdef _MSC_VER
            stream.write(reinterpret_cast<const char *>(b.data()), n);
#else
            for (BOOST_DEDUCED_TYPENAME ValueType::const_iterator itr = b.begin(); itr != b.end(); ++itr) {
                stream << *itr;
            }
#endif
            stream << HproseTags::TagQuote;
        }
    }

    template<typename Element, typename Container>
    void WriteBytes(const std::stack<Element, Container> & s, bool checkRef = true) {
#ifdef _MSC_VER
        WriteBytes(s._Get_container(), checkRef);
#else
        size_t n = s.size();
        if (WriteRef(&s, checkRef)) {
            std::stack<Element, Container> temp = s;
            std::vector<Element> v;
            v.resize(n);
            for (size_t i = 1; i <= n; i++) {
                v[n - i] = temp.top();
                temp.pop();
            }
            WriteBytes(v, false);
        }
#endif
    }

    template<typename Element, typename Container>
    void WriteBytes(const std::queue<Element, Container> & q, bool checkRef = true) {
#ifdef _MSC_VER
        WriteBytes(q._Get_container(), checkRef);
#else
        size_t n = q.size();
        if (WriteRef(&q, checkRef)) {
            std::queue<Element, Container> temp = q;
            std::vector<Element> v;
            v.resize(n);
            for (size_t i = 0; i < n; i++) {
                v[i] = temp.front();
                temp.pop();
            }
            WriteBytes(v, false);
        }
#endif
    }

    template<typename ValueType>
    void WriteBytes(const ValueType * b, size_t n, bool checkRef = true) {
        HPROSE_STATIC_ASSERT(NonCVListType<ValueType>::value == BytesType::value, "Require BytesType");
        if (checkRef) {
            references.push_back(Any());
        }
        stream << HproseTags::TagBytes;
        if (n > 0) WriteIntFast(n, stream);
        stream << HproseTags::TagQuote;
        stream.write(reinterpret_cast<const char *>(b), n);
        stream << HproseTags::TagQuote;
    }

    template<typename ValueType, size_t ArraySize>
    inline void WriteBytes(const ValueType (&value)[ArraySize], bool checkRef = true) {
        WriteBytes(value, ArraySize, checkRef);
    }

#ifndef BOOST_NO_INITIALIZER_LISTS
    template<typename ValueType>
    inline void WriteBytes(const std::initializer_list<ValueType> & l, bool checkRef = true) {
        if (WriteRef(&l, checkRef)) {
            WriteBytes(l.begin(), l.size(), false);
        }
    }
#endif

    template<typename ValueType>
    inline void WriteUTF8Char(const ValueType & c) {
        HPROSE_STATIC_ASSERT(NonCVType<ValueType>::value == CharType::value, "Require CharType");
        size_t len = UTF8Encode(c, buf, CharSize<sizeof(ValueType)>());
        if (len > 0 && len < 4) {
            stream << HproseTags::TagUTF8Char;
            stream.write(buf, len);
        } else {
            HPROSE_THROW_EXCEPTION("Malformed UTF-8 char");
        }
    }

    inline void WriteUTF8Char(const char & c) {
#if defined(BOOST_WINDOWS) && (!defined(HPROSE_CHARSET_UTF8))
        WriteUTF8Char(AnsiToUnicode(c));
#else
        if (is_ascii(c)) {
            stream << HproseTags::TagUTF8Char << c;
        } else {
            WriteInteger(c);
        }
#endif
    }

    template<typename ValueType>
    inline void WriteString(const ValueType & s, bool checkRef = true) {
        typedef BOOST_DEDUCED_TYPENAME NonCVElementType<ValueType>::type element;
        WriteString(std::basic_string<element>(s), checkRef);
    }

    template<typename Element, typename Traits, typename Allocator>
    inline void WriteString(const std::basic_string<Element, Traits, Allocator> & s, bool checkRef = true) {
        if (WriteRef(s, checkRef)) {
            stream << HproseTags::TagString;
            WriteUTF8String(UTF8Encode(s), UTF16Size(s), stream);
        }
    }

    inline void WriteString(const std::string & s, bool checkRef = true) {
#if defined(BOOST_WINDOWS) && (!defined(HPROSE_CHARSET_UTF8))
        WriteString(AnsiToUnicode(s), checkRef);
#else
        size_t len = UTF8Length(s.begin(), s.end(), Char16Type());
        if (len == 1) {
            stream << HproseTags::TagUTF8Char << s;
        } else {
            if (WriteRef(s, checkRef)) {
                stream << HproseTags::TagString;
                WriteUTF8String(s, len, stream);
            }
        }
#endif
    }

#ifdef BOOST_WINDOWS
    inline void WriteGUID(const GUID & g, bool checkRef = true) {
        if (WriteRef(g, checkRef)) {
#ifdef __STDC_WANT_SECURE_LIB__
            sprintf_s(buf, 40,
#else
            sprintf(buf,
#endif
                "g{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2],
                g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
            stream.write(buf, 39);
        }
    }
#else
//   inline void WriteGUID(const uuid_t & u, bool checkRef = true) {
//       if (WriteRef(u, checkRef)) {
//           buf[0] = 'g';
//           uuid_unparse(u, &buf[1]);
// //        sprintf(buf,
// //            "g{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
// //            g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2],
// //            g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
//           stream.write(buf, 39);
//       }
//   }
#endif

    template<typename ValueType>
    void WriteList(const ValueType & a, bool checkRef = true) {
        HPROSE_STATIC_ASSERT(
            (NonCVType<ValueType>::value == BytesType::value) ||
            (NonCVType<ValueType>::value == ListType::value), "Require [BytesType, ListType]");
        size_t n = a.size();
        if (n > 0) {
            if (WriteRef(&a, checkRef)) {
                stream << HproseTags::TagList;
                WriteIntFast(n, stream);
                stream << HproseTags::TagOpenbrace;
                for (BOOST_DEDUCED_TYPENAME ValueType::const_iterator itr = a.begin(); itr != a.end(); ++itr) {
                    Serialize(*itr);
                }
                stream << HproseTags::TagClosebrace;
            }
        } else {
            stream << HproseTags::TagList << HproseTags::TagOpenbrace << HproseTags::TagClosebrace;
        }
    }

    template<typename Element, typename Container>
    void WriteList(const std::stack<Element, Container> & s, bool checkRef = true) {
#ifdef _MSC_VER
        WriteList(s._Get_container(), checkRef);
#else
        size_t n = s.size();
        if (n > 0) {
            if (WriteRef(&s, checkRef)) {
                std::stack<Element, Container> temp = s;
                std::vector<Element> v;
                v.resize(n);
                for (size_t i = 1; i <= n; i++) {
                    v[n - i] = temp.top();
                    temp.pop();
                }
                WriteList(v, false);
            }
        } else {
            stream << HproseTags::TagList << HproseTags::TagOpenbrace << HproseTags::TagClosebrace;
        }
#endif
    }

    template<typename Element, typename Container>
    void WriteList(const std::queue<Element, Container> & q, bool checkRef = true) {
#ifdef _MSC_VER
        WriteList(q._Get_container(), checkRef);
#else
        size_t n = q.size();
        if (n > 0) {
            if (WriteRef(&q, checkRef)) {
                std::queue<Element, Container> temp = q;
                std::vector<Element> v;
                v.resize(n);
                for (size_t i = 0; i < n; i++) {
                    v[i] = temp.front();
                    temp.pop();
                }
                WriteList(v, false);
            }
        } else {
            stream << HproseTags::TagList << HproseTags::TagOpenbrace << HproseTags::TagClosebrace;
        }
#endif
    }

    template<unsigned int Bits>
    void WriteList(const std::bitset<Bits> & b, bool checkRef = true) {
        if (Bits > 0) {
            stream << HproseTags::TagList;
            WriteIntFast(Bits, stream);
            stream << HproseTags::TagOpenbrace;
            for (size_t i = 0; i < Bits; i++) {
                Serialize(b[i]);
            }
            stream << HproseTags::TagClosebrace;
        } else {
            stream << HproseTags::TagList << HproseTags::TagOpenbrace << HproseTags::TagClosebrace;
        }
    }

    template<typename ValueType>
    void WriteList(const ValueType * a, size_t n, bool checkRef = true) {
        if (checkRef) {
            references.push_back(Any());
        }
        if (n > 0) {
            stream << HproseTags::TagList;
            WriteIntFast(n, stream);
            stream << HproseTags::TagOpenbrace;
            for (size_t i = 0; i < n; i++) {
                Serialize(a[i]);
            }
            stream << HproseTags::TagClosebrace;
        } else {
            stream << HproseTags::TagList << HproseTags::TagOpenbrace << HproseTags::TagClosebrace;
        }
    }

    template<typename ValueType, size_t ArraySize>
    inline void WriteList(const ValueType (&value)[ArraySize], bool checkRef = true) {
        WriteList(value, ArraySize, checkRef);
    }

#ifndef BOOST_NO_INITIALIZER_LISTS
    template<typename ValueType>
    inline void WriteList(const std::initializer_list<ValueType> & l, bool checkRef = true) {
        if (WriteRef(&l, checkRef)) {
            WriteList(l.begin(), l.size(), false);
        }
    }
#endif

    template<typename ValueType>
    void WriteMap(const ValueType & m, bool checkRef = true) {
        HPROSE_STATIC_ASSERT(NonCVType<ValueType>::value == MapType::value, "Require MapType");
        size_t n = m.size();
        if (n > 0) {
            if (WriteRef(&m, checkRef)) {
                stream << HproseTags::TagMap;
                WriteIntFast(n, stream);
                stream << HproseTags::TagOpenbrace;
                for (BOOST_DEDUCED_TYPENAME ValueType::const_iterator itr = m.begin(); itr != m.end(); ++itr) {
                    Serialize(itr->first);
                    Serialize(itr->second);
                }
                stream << HproseTags::TagClosebrace;
            }
        } else {
            stream << HproseTags::TagMap << HproseTags::TagOpenbrace << HproseTags::TagClosebrace;
        }
    }

    template<typename ValueType>
    void WriteObject(const ValueType & o, bool checkRef = true) {
        if (WriteRef(&o, checkRef)) {
            const std::type_info * type = &typeid(ValueType);
            long cr;
            std::vector<const std::type_info *>::iterator itr = std::find(classRef.begin(), classRef.end(), type);
            if (itr != classRef.end()) {
                cr = itr - classRef.begin();
            } else {
                cr = WriteClass(o);
            }
            stream << HproseTags::TagObject;
            WriteIntFast((int)cr, stream);
            stream << HproseTags::TagOpenbrace;
            ClassManager::ClassProperty * m = ClassManager::SharedInstance()->GetClassProperty(type);
            if (m) {
                for (ClassManager::ClassProperty::const_iterator itr = m->begin(); itr != m->end(); ++itr) {
                    itr->second.Serialize(*this, o);
                }
            }
            stream << HproseTags::TagClosebrace;
        }
    }

private:

    struct SerializeCache {
        int refCount;
        std::string data;
    };

    typedef std::tr1::unordered_map<const std::type_info *, SerializeCache> ClassCaches;

private:

    template<typename ValueType>
    inline void Serialize(const ValueType &, UnknownType) {
        HPROSE_THROW_EXCEPTION(std::string(typeid(ValueType).name()) + " is not a serializable type");
    }

#ifndef BOOST_NO_CXX11_NULLPTR
    inline void Serialize(const std::nullptr_t &, NullPtrType) {
        WriteNull();
    }
#endif

    template<typename ValueType>
    inline void Serialize(const ValueType & p, AutoPtrType) {
        Serialize(p.get()); // Pointer
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & p, PointerType) {
        if (p) {
            Serialize(*p);
        } else {
            WriteNull();
        }
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & a, AnyType) {
        a.Serialize(*this);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & b, BoolType) {
        WriteBool(b);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & b, ByteType) {
        WriteInteger(b);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & c, CharType) {
        WriteUTF8Char(c);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & e, EnumType) {
        WriteEnum(e);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & i, IntegerType) {
        WriteInteger(i);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & l, LongType) {
        WriteLong(l);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & d, DoubleType) {
        WriteDouble(d);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & t, TimeType) {
        WriteDateTime(t);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & b, BytesType) {
        WriteBytes(b);
    }

    template<typename ValueType, size_t ArraySize>
    inline void Serialize(const ValueType (&value)[ArraySize], BytesType) {
        WriteBytes(value);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & s, StringType) {
        typedef BOOST_DEDUCED_TYPENAME NonCVElementType<ValueType>::type element;
        Serialize(std::basic_string<element>(s));
    }

    template<typename Element, typename Traits, typename Allocator>
    inline void Serialize(const std::basic_string<Element, Traits, Allocator> & s, StringType) {
        switch (s.size()) {
            case 0:
                WriteEmpty();
                break;
            case 1:
                WriteUTF8Char(s[0]);
                break;
            default:
                WriteString(s);
        }
    }

    template<typename ValueType, size_t ArraySize>
    inline void Serialize(const ValueType (&value)[ArraySize], StringType) {
        size_t len = value[ArraySize - 1] == 0 ? (ArraySize - 1) : ArraySize;
        switch (len) {
            case 0:
                WriteEmpty();
                break;
            case 1:
                WriteUTF8Char(value[0]);
                break;
            default:
                WriteString(std::basic_string<ValueType>(value, len));
        }
    }

    template<typename Element, typename Traits, typename Allocator>
    inline void Serialize(const std::basic_ostringstream<Element, Traits, Allocator> & oss, StringType) {
        Serialize(oss.str());
    }

    template<typename Element, typename Traits, typename Allocator>
    inline void Serialize(const std::basic_stringbuf<Element, Traits, Allocator> & sb, StringType) {
        Serialize(sb.str());
    }

    template<typename Element, typename Traits, typename Allocator>
    inline void Serialize(const std::basic_stringstream<Element, Traits, Allocator> & ss, StringType) {
        Serialize(ss.str());
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & g, GuidType) {
        WriteGUID(g);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & a, ListType) {
        WriteList(a);
    }

    template<typename ValueType, size_t ArraySize>
    inline void Serialize(const ValueType (&value)[ArraySize], ListType) {
        WriteList(value);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & m, MapType) {
        WriteMap(m);
    }

    template<typename ValueType>
    inline void Serialize(const ValueType & o, ObjectType) {
        WriteObject(o);
    }

private:

    template<typename ValueType>
    inline void WriteInteger(ValueType i, const SignedType & s) {
        if (i >= 0 && i <= 9) {
            stream << static_cast<char>('0' + i);
        } else {
            stream << HproseTags::TagInteger;
            WriteIntFast(i, stream, s);
            stream << HproseTags::TagSemicolon;
        }
    }

    template<typename ValueType>
    inline void WriteInteger(ValueType i, const UnignedType & u) {
        if (i <= 9) {
            stream << static_cast<char>('0' + i);
        } else {
            stream << HproseTags::TagInteger;
            WriteIntFast(i, stream, u);
            stream << HproseTags::TagSemicolon;
        }
    }

    template<typename ValueType>
    inline void WriteLong(ValueType l, const SignedType & s) {
        if (l >= 0 && l <= 9) {
            stream << static_cast<char>('0' + l);
        } else {
            stream << HproseTags::TagLong;
            WriteIntFast(l, stream, s);
            stream << HproseTags::TagSemicolon;
        }
    }

    template<typename ValueType>
    inline void WriteLong(ValueType l, const UnignedType & u) {
        if (l <= 9) {
            stream << static_cast<char>('0' + l);
        } else {
            stream << HproseTags::TagLong;
            WriteIntFast(l, stream, u);
            stream << HproseTags::TagSemicolon;
        }
    }

    void WriteDateTime(const tm & t) {
        int year = t.tm_year + 1900;
        int month = t.tm_mon + 1;
        int day = t.tm_mday;
        int hour = t.tm_hour;
        int minute = t.tm_min;
        int second = t.tm_sec;
        if ((hour == 0) && (minute == 0) && (second == 0)) {
            WriteDate(year, month, day);
        } else if ((year == 1970) && (month == 1) && (day == 1)) {
            WriteTime(hour, minute, second);
        } else {
            WriteDate(year, month, day);
            WriteTime(hour, minute, second);
        }
        stream << HproseTags::TagUTC;
    }

    void WriteDate(int year, int month, int day) {
        stream << HproseTags::TagDate;
        stream << static_cast<char>('0' + (year / 1000 % 10));
        stream << static_cast<char>('0' + (year / 100 % 10));
        stream << static_cast<char>('0' + (year / 10 % 10));
        stream << static_cast<char>('0' + (year % 10));
        stream << static_cast<char>('0' + (month / 10 % 10));
        stream << static_cast<char>('0' + (month % 10));
        stream << static_cast<char>('0' + (day / 10 % 10));
        stream << static_cast<char>('0' + (day % 10));
    }

    void WriteTime(int hour, int minute, int second, int milliseconds = 0) {
        stream << HproseTags::TagTime;
        stream << static_cast<char>('0' + (hour / 10 % 10));
        stream << static_cast<char>('0' + (hour % 10));
        stream << static_cast<char>('0' + (minute / 10 % 10));
        stream << static_cast<char>('0' + (minute % 10));
        stream << static_cast<char>('0' + (second / 10 % 10));
        stream << static_cast<char>('0' + (second % 10));
        if (milliseconds > 0) {
            stream << HproseTags::TagPoint;
            stream << static_cast<char>('0' + (milliseconds / 100 % 10));
            stream << static_cast<char>('0' + (milliseconds / 10 % 10));
            stream << static_cast<char>('0' + (milliseconds % 10));
        }
    }

    inline void WriteUTF8String(const std::string & s, size_t len, std::ostream & os) {
        if (len) {
            WriteIntFast(len, os);
        }
        os << HproseTags::TagQuote << s << HproseTags::TagQuote;
    }

    inline void WriteUTF8String(const std::string & s, std::ostream & os) {
#if defined(BOOST_WINDOWS) && (!defined(HPROSE_CHARSET_UTF8))
        std::wstring w = AnsiToUnicode(s);
        WriteUTF8String(UTF8Encode(w), w.size(), os);
#else
        WriteUTF8String(s, UTF8Length(s.begin(), s.end(), Char16Type()), os);
#endif
    }

    template<typename ValueType>
    long WriteClass(const ValueType & o) {
        const std::type_info * type = &typeid(ValueType);
        SerializeCache cache = GetCache(type);
        if (cache.data.empty()) {
            cache.refCount = 0;
            std::string alias = ClassManager::SharedInstance()->GetClassAlias(type);
            if (!alias.empty()) {
                std::ostringstream os;
                os << HproseTags::TagClass;
                WriteUTF8String(alias, os);
                ClassManager::ClassProperty * m = ClassManager::SharedInstance()->GetClassProperty(alias);
                if (m) {
                    size_t n = m->size();
                    if (n > 0) WriteIntFast(n, os);
                }
                os << HproseTags::TagOpenbrace;
                if (m) {
                    for (ClassManager::ClassProperty::const_iterator itr = m->begin(); itr != m->end(); ++itr, ++cache.refCount) {
                        os << HproseTags::TagString;
                        WriteUTF8String(itr->first, os);
                    }
                }
                os << HproseTags::TagClosebrace;
                cache.data = os.str();
                SetCache(type, cache);
            } else {
                Serialize(o, UnknownType());
            }
        }
        stream << cache.data;
        references.resize(references.size() + cache.refCount);
        classRef.push_back(type);
        return classRef.size() - 1;
    }

    template<typename ValueType>
    bool WriteRef(const ValueType & value, bool checkRef) {
        if (simple) return true;
        if (checkRef) {
            int ref = vector_index_of(references, value);
            if (ref > -1) {
                WriteRef(ref);
                return false;
            }
        }
        references.push_back(value);
        return true;
    }

    inline void WriteRef(int ref) {
        stream << HproseTags::TagRef;
        WriteIntFast(ref, stream);
        stream << HproseTags::TagSemicolon;
    }

private:

    HPROSE_STATIC_MEMBER(ClassCaches, Caches);

    HPROSE_STATIC_MEMBER(boost::shared_mutex, Mutex);

    SerializeCache & GetCache(const std::type_info * type) {
        boost::shared_mutex & mutex = GetMutex();
        boost::shared_lock<boost::shared_mutex> lock(mutex);
        ClassCaches & caches = GetCaches();
        return caches[type];
    }

    void SetCache(const std::type_info * type, const SerializeCache & cache) {
        boost::shared_mutex & mutex = GetMutex();
        boost::unique_lock<boost::shared_mutex> lock(mutex);
        ClassCaches & caches = GetCaches();
        caches[type] = cache;
    }

private:

    char buf[40];

    std::ostream & stream;
    bool simple;

    std::vector<Any> references;
    std::vector<const std::type_info *> classRef;

}; // class HproseWriter

} // namespace hprose

#endif // HPROSE_IO_HPROSE_WRITER_HPP
