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
 * HproseReader.hpp                                       *
 *                                                        *
 * hprose reader unit for cpp.                            *
 *                                                        *
 * LastModified: Jul 20, 2015                             *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_IO_HPROSE_READER_HPP
#define HPROSE_IO_HPROSE_READER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <istream>

#include "ClassManager.hpp"
#include "HproseTags.hpp"
#include "HproseTypes.hpp"
#include "ext/CTime.hpp"

namespace hprose {

class HproseReader {
public:

    HproseReader(std::istream & stream, bool simple = false)
        : stream(stream), simple(simple) {
    }

public:

    template<typename ReturnType>
    inline ReturnType Unserialize() {
        return Unserialize<ReturnType>(NonCVType<ReturnType>()); // Don't need remove_cv.
    }

    inline Any Unserialize() {
        return Unserialize<Any>();
    }

public:
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value>::type Read()
    {
        return ReadInt();
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value>::type Read()
    {
        return ReadString();
    }

    bool ReadBool() {
        switch (char tag = stream.get()) {
            case '0': return false;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': return true;
            case HproseTags::TagInteger: return ReadIntWithoutTag() != 0;
            case HproseTags::TagLong: return ReadUntil(HproseTags::TagSemicolon) != "0";
            case HproseTags::TagDouble: return ReadDoubleWithoutTag() != 0.0;
            case HproseTags::TagNull:
            case HproseTags::TagEmpty: return false;
            case HproseTags::TagTrue: return true;
            case HproseTags::TagFalse: return false;
            case HproseTags::TagNaN: return true;
            case HproseTags::TagInfinity: stream.get(); return true;
            case HproseTags::TagUTF8Char: return std::wstring(L"\00").find(ReadUTF8CharAsChar<wchar_t>()) == std::wstring::npos;
            case HproseTags::TagString: return ReadStringWithoutTag() == "true";
            case HproseTags::TagRef: return ReadStringRef() == "true";
            default: HPROSE_THROW_EXCEPTION(CastError<bool>(tag));
        }
    }

    template<typename ReturnType>
    ReturnType ReadChar() {
        HPROSE_STATIC_ASSERT(NonCVType<ReturnType>::value == CharType::value, "Require CharType");
        switch (char tag = stream.get()) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': return tag;
            case HproseTags::TagInteger:
            case HproseTags::TagLong: return ReadIntWithoutTag<ReturnType>();
            case HproseTags::TagDouble: return static_cast<ReturnType>(ReadDoubleWithoutTag());
            case HproseTags::TagNull: return 0;
            case HproseTags::TagUTF8Char: return ReadUTF8CharAsChar<ReturnType>();
            case HproseTags::TagString: return ReadStringWithoutTag<std::basic_string<ReturnType> >().at(0);
            case HproseTags::TagRef: return ReadStringRef<std::basic_string<ReturnType> >().at(0);
            default: HPROSE_THROW_EXCEPTION(CastError<ReturnType>(tag));
        }
    }

    inline char_type ReadChar() {
        return ReadChar<char_type>();
    }

    template<typename ReturnType>
    ReturnType ReadInt() {
        switch (char tag = stream.get()) {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case HproseTags::TagInteger:
            case HproseTags::TagLong: return ReadIntWithoutTag<ReturnType>();
            case HproseTags::TagDouble: return static_cast<ReturnType>(ReadDoubleWithoutTag());
            case HproseTags::TagNull:
            case HproseTags::TagEmpty: return 0;
            case HproseTags::TagTrue: return 1;
            case HproseTags::TagFalse: return 0;
            case HproseTags::TagDate: return static_cast<ReturnType>(ReadDateWithoutTag());
            case HproseTags::TagTime: return static_cast<ReturnType>(ReadTimeWithoutTag());
            case HproseTags::TagUTF8Char: return ReadUTF8CharAsChar<ReturnType>();
            case HproseTags::TagString: return ParseNumeric<ReturnType>(ReadStringWithoutTag());
            case HproseTags::TagRef: return ParseNumeric<ReturnType>(ReadStringRef());
            default: HPROSE_THROW_EXCEPTION(CastError<ReturnType>(tag));
        }
    }

    inline int ReadInt() {
        return ReadInt<int>();
    }

    inline long_type ReadLong() {
        return ReadInt<long_type>();
    }

    template<typename ReturnType>
    ReturnType ReadDouble() {
        switch (char tag = stream.get()) {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case HproseTags::TagInteger:
            case HproseTags::TagLong: return ReadIntWithoutTag<ReturnType>();
            case HproseTags::TagDouble: return ReadDoubleWithoutTag<ReturnType>();
            case HproseTags::TagNull:
            case HproseTags::TagEmpty: return 0;
            case HproseTags::TagTrue: return 1;
            case HproseTags::TagFalse: return 0;
            case HproseTags::TagNaN: return std::numeric_limits<ReturnType>::quiet_NaN();
            case HproseTags::TagInfinity: return ReadInfinityWithoutTag<ReturnType>();
            case HproseTags::TagUTF8Char: return static_cast<ReturnType>(ReadUTF8CharAsChar());
            case HproseTags::TagString: return ParseNumeric<ReturnType>(ReadStringWithoutTag());
            case HproseTags::TagRef: return ParseNumeric<ReturnType>(ReadStringRef());
            default: HPROSE_THROW_EXCEPTION(CastError<ReturnType>(tag));
        }
    }

    inline double ReadDouble() {
        return ReadDouble<double>();
    }

    time_t ReadDateTime() {
        switch (char tag = stream.get()) {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case HproseTags::TagInteger:
            case HproseTags::TagLong: return ReadIntWithoutTag<time_t>();
            case HproseTags::TagDouble: return static_cast<time_t>(ReadDoubleWithoutTag());
            case HproseTags::TagNull:
            case HproseTags::TagEmpty: return 0;
            case HproseTags::TagDate: return ReadDateWithoutTag();
            case HproseTags::TagTime: return ReadTimeWithoutTag();
            case HproseTags::TagRef: return ReadRef<time_t>();
            default: HPROSE_THROW_EXCEPTION(CastError<time_t>(tag));
        }
    }

    template<typename ReturnType>
    ReturnType ReadBytes() {
        typedef BOOST_DEDUCED_TYPENAME NonCVElementType<ReturnType>::type element;
        switch (char tag = stream.get()) {
            case HproseTags::TagNull:
            case HproseTags::TagEmpty: return ReturnType();
            case HproseTags::TagUTF8Char: {
                std::string s = ReadUTF8CharAsUTF8String();
                return ReturnType(s.begin(), s.end());
            }
            case HproseTags::TagString: {
                std::string s = ReadCharsAsUTF8String();
                return ReturnType(s.begin(), s.end());
            }
            case HproseTags::TagGuid: {
#ifdef BOOST_WINDOWS
                GUID g = ReadGUIDWithoutTag();
                element *first = reinterpret_cast<element *>(&g);
                return ReturnType(first, first + sizeof(GUID));
#else
    //Todo: uuid_t
#endif
            }
            case HproseTags::TagBytes: return ReadBytesWithoutTag();
            case HproseTags::TagList: {
                int count = ReadInt(HproseTags::TagOpenbrace);
                ReturnType a;
                a.resize(count);
                for (int i = 0; i < count; i++) {
                    a[i] = ReadInt<element>();
                }
                stream.get();
                references.push_back(a);
                return a;
            }
            //case HproseTags::TagRef: {
            //    object obj = ReadRef();
            //    if (obj is byte[]) return (byte[])obj;
            //    if (obj is Guid) return ((Guid)obj).ToByteArray();
            //    if (obj is MemoryStream) return ((MemoryStream)obj).ToArray();
            //    throw CastError(obj, HproseHelper.typeofByteArray);
            //}
            default: HPROSE_THROW_EXCEPTION(CastError<ReturnType>(tag));
        }
    }

    template<typename ReturnType>
    ReturnType ReadString() {
        switch (char tag = stream.get()) {
            case '0': return Stringify<ReturnType>("0");
            case '1': return Stringify<ReturnType>("1");
            case '2': return Stringify<ReturnType>("2");
            case '3': return Stringify<ReturnType>("3");
            case '4': return Stringify<ReturnType>("4");
            case '5': return Stringify<ReturnType>("5");
            case '6': return Stringify<ReturnType>("6");
            case '7': return Stringify<ReturnType>("7");
            case '8': return Stringify<ReturnType>("8");
            case '9': return Stringify<ReturnType>("9");
            case HproseTags::TagInteger:
            case HproseTags::TagLong:
            case HproseTags::TagDouble: return Stringify<ReturnType>(ReadUntil(HproseTags::TagSemicolon));
            case HproseTags::TagNull:
            case HproseTags::TagEmpty: return ReturnType();
            case HproseTags::TagTrue: return Stringify<ReturnType>("true");
            case HproseTags::TagFalse: return Stringify<ReturnType>("false");
            case HproseTags::TagNaN: return Stringify<ReturnType>("NaN");
            case HproseTags::TagInfinity: return (stream.get() == HproseTags::TagPos) ? Stringify<ReturnType>("Infinity") : Stringify<ReturnType>("-Infinity");
//            case HproseTags::TagDate: return ReadDateWithoutTag().ToString();
//            case HproseTags::TagTime: return ReadTimeWithoutTag().ToString();
            case HproseTags::TagUTF8Char: return ReadUTF8CharWithoutTag<ReturnType>();
            case HproseTags::TagString: return ReadStringWithoutTag<ReturnType>();
//            case HproseTags::TagGuid: // return readUUIDWithoutTag().toString();
//            case HproseTags::TagList: // return readListWithoutTag().toString();
//            case HproseTags::TagMap: // return readMapWithoutTag().toString();
//            case HproseTags::TagClass: // readClass(); return readObject(null).toString();
//            case HproseTags::TagObject: // return readObjectWithoutTag(null).toString();
            case HproseTags::TagRef: return ReadRef<ReturnType>();
            default: HPROSE_THROW_EXCEPTION(CastError<std::string>(tag));
        }
    }

    inline std::string ReadString() {
        return ReadString<std::string>();
    }

#ifdef BOOST_WINDOWS
    GUID ReadGUID()  {
        switch (char tag = stream.get()) {
//            case HproseTags::TagBytes: return new Guid(ReadBytesWithoutTag());
            case HproseTags::TagGuid: return ReadGUIDWithoutTag();
//            case HproseTags::TagString: return new Guid(ReadStringWithoutTag());
            //case HproseTags::TagRef: {
            //    object obj = ReadRef();
            //    if (obj is Guid) return (Guid)obj;
            //    if (obj is byte[]) return new Guid((byte[])obj);
            //    if (obj is string) return new Guid((string)obj);
            //    if (obj is char[]) return new Guid(new string((char[])obj));
            //    throw CastError(obj, HproseHelper.typeofGuid);
            //}
            default: HPROSE_THROW_EXCEPTION(CastError<GUID>(tag));
        }
    }
#endif

    template<typename ReturnType>
    ReturnType ReadList() {
        switch (char tag = stream.get()) {
            case HproseTags::TagNull: return ReturnType();
            case HproseTags::TagList: return ReadListWithoutTag<ReturnType>();
//            case HproseTags.TagRef: return (List<ReturnType>)ReadRef();
            default: HPROSE_THROW_EXCEPTION(CastError<ReturnType>(tag));
        }
    }

    template<typename ReturnType>
    ReturnType ReadMap() {
        switch (char tag = stream.get()) {
            case HproseTags::TagNull: return ReturnType();
           // case HproseTags::TagList: return ReadListAsMap();
            case HproseTags::TagMap: return ReadMapWithoutTag<ReturnType>();
           // case HproseTags::TagClass: readClass(); return readMap(cls, type);
           // case HproseTags::TagObject: return readObjectAsMap((Map)HproseHelper.newInstance(cls));
            case HproseTags::TagRef: return ReadRef<ReturnType>();
            default: HPROSE_THROW_EXCEPTION(CastError<ReturnType>(tag));
        }
    }

    template<typename ReturnType>
    ReturnType ReadObject() {
        switch (char tag = stream.get()) {
//        case HproseTags::TagNull: return null;
//        case HproseTags::TagMap: return readMapAsObject(type);
//        case HproseTags::TagClass: readClass(); return readObject(type);
//        case HproseTags::TagObject: return readObjectWithoutTag(type);
//        case HproseTags::TagRef: return readRef(type);
        default: HPROSE_THROW_EXCEPTION(CastError<ReturnType>(tag));
        }
    }

public:

    inline void CheckTag(char expected, char found) {
        if (found != expected) {
            HPROSE_THROW_EXCEPTION(std::string("HproseTags::Tag '") + expected + "' expected, but '" + found + "' found in stream");
        }
    }

    inline void CheckTag(char expected) {
        CheckTag(expected, stream.get());
    }

    template<size_t ArraySize>
    inline char CheckTags(const char (&expected)[ArraySize], char found) {
        bool ok = false;
        for (size_t i = 0; i < ArraySize; i++) {
            if (expected[i] == found) {
                ok = true;
                break;
            }
        }
        if (!ok) {
            HPROSE_THROW_EXCEPTION(std::string("HproseTags::Tag '") + std::string(expected, expected + ArraySize) + "' expected, but '" + found + "' found in stream");
        }
        return found;
    }

    template<size_t ArraySize>
    inline char CheckTags(const char (&expected)[ArraySize]) {
        return CheckTags(expected, stream.get());
    }

private:

    template<typename ReturnType>
    inline ReturnType Unserialize(UnknownType) {
        HPROSE_THROW_EXCEPTION(CastError<ReturnType>(stream.get()));
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(AutoPtrType) {
        return Unserialize<ReturnType>(UnknownType());
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(PointerType) {
        return Unserialize<ReturnType>(UnknownType());
    }

    template<typename ReturnType>
    ReturnType Unserialize(AnyType) {
        switch (char tag = stream.get()) {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case HproseTags::TagInteger: return ReadIntWithoutTag();
            case HproseTags::TagLong: return ReadLongWithoutTag(); // Todo: BigInteger
            case HproseTags::TagDouble: return ReadDoubleWithoutTag();
            case HproseTags::TagNaN: return std::numeric_limits<double>::quiet_NaN();
            case HproseTags::TagInfinity: return ReadInfinityWithoutTag();
            case HproseTags::TagNull: return Any();
            case HproseTags::TagTrue: return true;
            case HproseTags::TagFalse: return false;
            case HproseTags::TagDate: return ReadDateWithoutTag();
            case HproseTags::TagTime: return ReadTimeWithoutTag();
            case HproseTags::TagBytes: return ReadBytesWithoutTag();
            case HproseTags::TagUTF8Char: return ReadUTF8CharWithoutTag();
            case HproseTags::TagString: return ReadStringWithoutTag();
            case HproseTags::TagList: return ReadListWithoutTag();
            case HproseTags::TagMap: return ReadMapWithoutTag();
            case HproseTags::TagClass: ReadClass(); return Unserialize();
            //case HproseTags::TagObject: return ReadObjectWithoutTag();
            case HproseTags::TagRef: return references[ReadInt(HproseTags::TagSemicolon)];
            case HproseTags::TagError: HPROSE_THROW_EXCEPTION(ReadString());
            default: HPROSE_THROW_EXCEPTION(UnexpectedTag(tag));
        }
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(BoolType) {
        return ReadBool();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(ByteType) {
        return ReadInt<ReturnType>();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(CharType) {
        return ReadChar<ReturnType>();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(EnumType) {
        return static_cast<ReturnType>(ReadInt());
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(IntegerType) {
        return ReadInt<ReturnType>();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(LongType) {
        return ReadLong(); // Todo: BigInteger
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(DoubleType) {
        return ReadDouble<ReturnType>();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(TimeType) {
        return ReadDateTime();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(BytesType) {
        return ReadBytes<ReturnType>();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(StringType) {
        return ReadString<ReturnType>();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(GuidType) {
#ifdef BOOST_WINDOWS
        return ReadGUID();
#else
    // Todo: uuid_t
#endif
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(ListType) {
        return ReadList<ReturnType>();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(MapType) {
        return ReadMap<ReturnType>();
    }

    template<typename ReturnType>
    inline ReturnType Unserialize(ObjectType) {
        return ReadObject<ReturnType>();
    }

private:

    std::string ReadUntil(char tag) {
        std::stringstream ss;
        char c = stream.get();
        while ((c != tag) && (c != -1)) {
            ss << c;
            c = stream.get();
        }
        return ss.str();
    }

    template<typename ReturnType>
    ReturnType ReadInt(char tag) {
        ReturnType result = 0;
        char c = stream.get();
        if (c == tag) {
            return result;
        }
        ReturnType sign = 1;
        switch (c) {
            case '-': sign = -1; // no break
            case '+': c = stream.get();
        }
        while (c != tag && c != -1) {
            result *= 10;
            result += (c - '0') * sign;
            c = stream.get();
        }
        return result;
    }

    inline int ReadInt(char tag) {
        return ReadInt<int>(tag);
    }

    template<typename ReturnType>
    inline ReturnType ParseNumeric(std::stringstream && ss) {
        ReturnType result;
        ss >> result;
        return result;
    }

    template<typename ReturnType>
    inline ReturnType ParseNumeric(std::string s) {
        return ParseNumeric<ReturnType>(std::stringstream(s));
    }

    void ReadDate(struct tm * ptm) {
        int year = stream.get() - '0';
        year = year * 10 + stream.get() - '0';
        year = year * 10 + stream.get() - '0';
        year = year * 10 + stream.get() - '0';
        int month = stream.get() - '0';
        month = month * 10 + stream.get() - '0';
        int day = stream.get() - '0';
        day = day * 10 + stream.get() - '0';
        ptm->tm_year = year - 1900;
        ptm->tm_mon = month - 1;
        ptm->tm_mday = day;
    }

    void ReadTime(struct tm * ptm) {
        int hour = stream.get() - '0';
        hour = hour * 10 + stream.get() - '0';
        int min = stream.get() - '0';
        min = min * 10 + stream.get() - '0';
        int sec = stream.get() - '0';
        sec = sec * 10 + stream.get() - '0';
        char tag = stream.get();
        if (tag == HproseTags::TagPoint) { // ignore millisec
            do {
                tag = stream.get();
            } while ((tag >= '0') && (tag <= '9'));
        }
        if (tag == HproseTags::TagUTC) {
            sec -= CTime::TimeZone();
        }
        ptm->tm_hour = hour;
        ptm->tm_min = min;
        ptm->tm_sec = sec;
    }

    template<typename ReturnType>
    inline ReturnType ReadUTF8CharAsChar() {
        return static_cast<ReturnType>(ReadUTF8CharAsChar());
    }

    char_type ReadUTF8CharAsChar() {
        char_type u;
        int c = stream.get();
        switch (c >> 4) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7: { // 0xxx xxxx
                u = (char_type)c;
                break;
            }
            case 12:
            case 13: { // 110x xxxx   10xx xxxx
                int c2 = stream.get();
                u = (char_type)(((c & 0x1f) << 6) | (c2 & 0x3f));
                break;
            }
            case 14: { // 1110 xxxx  10xx xxxx  10xx xxxx
                int c2 = stream.get();
                int c3 = stream.get();
                u = (char_type)(((c & 0x0f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f));
                break;
            }
            default:
                HPROSE_THROW_EXCEPTION("bad utf-8 encoding"); //Todo: at
        }
        return u;
    }

    std::basic_string<char_type> ReadChars() {
        std::basic_string<char_type> buf;
        int count = ReadInt(HproseTags::TagQuote);
        buf.resize(count);
        for (int i = 0; i < count; i++) {
            int c = stream.get();
            switch (c >> 4) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7: { // 0xxx xxxx
                    buf[i] = (char_type)c;
                    break;
                }
                case 12:
                case 13: { // 110x xxxx   10xx xxxx
                    char c2 = stream.get();
                    buf[i] = (char_type)(((c & 0x1f) << 6) | (c2 & 0x3f));
                    break;
                }
                case 14: { // 1110 xxxx  10xx xxxx  10xx xxxx
                    char c2 = stream.get();
                    char c3 = stream.get();
                    buf[i] = (char_type)(((c & 0x0f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f));
                    break;
                }
                case 15: { // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                    if ((c & 0xf) <= 4) {
                        char c2 = stream.get();
                        char c3 = stream.get();
                        char c4 = stream.get();
                        int s = (((c & 0x07) << 18) | ((c2 & 0x3f) << 12) | ((c3 & 0x3f) << 6) | (c4 & 0x3f)) - 0x10000;
                        if (0 <= s && s <= 0xfffff) {
                            buf[i++] = (char_type)(((s >> 10) & 0x03ff) | 0xd800);
                            buf[i] = (char_type)((s & 0x03ff) | 0xdc00);
                            break;
                        }
                    }
                    // no break here!! here need throw exception.
                }
                default:
                    HPROSE_THROW_EXCEPTION("bad utf-8 encoding");//Todo: at
                };
        }
        stream.get();
        return buf;
    }

    template<typename ReturnType>
    ReturnType ReadCharsAsString(const std::basic_string<char_type> & chars, Char8Type) {
#if defined(BOOST_WINDOWS) && (!defined(HPROSE_CHARSET_UTF8))
        return UnicodeToAnsi(chars);
#else
        std::string s;
        s.resize(UTF8Size(chars.begin(), chars.end(), Char16Type()));
        UTF8Encode(chars.begin(), chars.end(), s.begin(), Char16Type());
        return s;
#endif
    }

    template<typename ReturnType>
    inline ReturnType ReadCharsAsString(const std::basic_string<char_type> & chars, Char16Type) {
        return ReturnType(chars.begin(), chars.end());
    }

    template<typename ReturnType>
    inline ReturnType ReadCharsAsString(const std::basic_string<char_type> & chars, Char32Type) {
        return ReturnType(chars.begin(), chars.end());
    }

    std::string ReadUTF8CharAsUTF8String() {
        std::string s;
        int c = stream.get();
        switch (c >> 4) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7: {
                s.push_back(c);
                break;
            }
            case 12:
            case 13: {
                char c2 = stream.get();
                s.push_back(c);
                s.push_back(c2);
                break;
            }
            case 14: {
                char c2 = stream.get();
                char c3 = stream.get();
                s.push_back(c);
                s.push_back(c2);
                s.push_back(c3);
                break;
            }
            default:
                HPROSE_THROW_EXCEPTION("bad utf-8 encoding");//Todo: at
        }
        return s;
    }

    std::string ReadCharsAsUTF8String() {
        int count = ReadInt(HproseTags::TagQuote);
        std::string s;
        s.reserve(count * 3);
        for (int i = 0; i < count; i++) {
            int c = stream.get();
            switch (c >> 4) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7: {
                    s.push_back(c);
                    break;
                }
                case 12:
                case 13: {
                    char c2 = stream.get();
                    s.push_back(c);
                    s.push_back(c2);
                    break;
                }
                case 14: {
                    char c2 = stream.get();
                    char c3 = stream.get();
                    s.push_back(c);
                    s.push_back(c2);
                    s.push_back(c3);
                    break;
                }
                case 15: { // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                        if ((c & 0xf) <= 4) {
                            char c2 = stream.get();
                            char c3 = stream.get();
                            char c4 = stream.get();
                            int c32 = (((c & 0x07) << 18) | ((c2 & 0x3f) << 12) | ((c3 & 0x3f) << 6) | (c4 & 0x3f)) - 0x10000;
                            if (0 <= c32 && c32 <= 0xfffff) {
                                s.push_back(c);
                                s.push_back(c2);
                                s.push_back(c3);
                                s.push_back(c4);
                                break;
                            }
                        }
                        // no break here!! here need throw exception.
                    }
                default:
                    HPROSE_THROW_EXCEPTION("bad utf-8 encoding");//Todo: at
            }
        }
        stream.get();
        references.push_back(s);
        return s;
    }

    template<typename ReturnType>
    inline ReturnType Stringify(const std::string & s) {
        return ReturnType(s.begin(), s.end());
    }

    template<typename ReturnType, size_t ArraySize>
    inline ReturnType Stringify(const char (&value)[ArraySize]) {
        size_t len = value[ArraySize - 1] == 0 ? (ArraySize - 1) : ArraySize;
        return ReturnType(value, &value[len - 1]);
    }

private:

    template<typename ReturnType>
    inline ReturnType ReadIntWithoutTag() {
        return ReadInt<ReturnType>(HproseTags::TagSemicolon);
    }

    inline int ReadIntWithoutTag() {
        return ReadIntWithoutTag<int>();
    }

    inline long_type ReadLongWithoutTag() {
        return ReadIntWithoutTag<long_type>();
    }

    template<typename ReturnType>
    inline ReturnType ReadDoubleWithoutTag() {
        return ParseNumeric<ReturnType>(ReadUntil(HproseTags::TagSemicolon));
    }

    inline double ReadDoubleWithoutTag() {
        return ReadDoubleWithoutTag<double>();
    }

    template<typename ReturnType>
    inline ReturnType ReadInfinityWithoutTag() {
        return ((stream.get() == HproseTags::TagPos)
                ? std::numeric_limits<ReturnType>::infinity()
                : -std::numeric_limits<ReturnType>::infinity());
    }

    inline double ReadInfinityWithoutTag() {
        return ReadInfinityWithoutTag<double>();
    }

    time_t ReadDateWithoutTag() {
        tm t;
        t.tm_isdst = -1;
        ReadDate(&t);
        char tag = stream.get();
        if (tag == HproseTags::TagTime) {
            ReadTime(&t);
        } else {
            t.tm_hour = 0;
            t.tm_min = 0;
            if (tag == HproseTags::TagUTC) {
                t.tm_sec = static_cast<int>(-CTime::TimeZone());
            } else {
                t.tm_sec = 0;
            }
        }
        time_t result = mktime(&t);
        references.push_back(result);
        return result;
    }

    time_t ReadTimeWithoutTag() {
        tm t;
        t.tm_year = 70; // 1970 - 1900
        t.tm_mon = 0;
        t.tm_mday = 0;
        t.tm_isdst = -1;
        ReadTime(&t);
        time_t result = mktime(&t);
        references.push_back(result);
        return result;
    }

    std::vector<unsigned char> ReadBytesWithoutTag() {
        int count = ReadInt(HproseTags::TagQuote);
        std::vector<unsigned char> b;
        b.resize(count);
        stream.read(reinterpret_cast<char *>(b.data()), count);
        stream.get();
        references.push_back(b);
        return b;
    }

    template<typename ReturnType>
    ReturnType ReadUTF8CharWithoutTag(Char8Type) {
        char_type c = ReadUTF8CharAsChar();
#if defined(BOOST_WINDOWS) && (!defined(HPROSE_CHARSET_UTF8))
        return UnicodeToAnsi(std::wstring(1, c));
#else
        std::string s;
        s.resize(UTF8Size(&c, &c, Char16Type()));
        UTF8Encode(&c, &c, s.begin(), Char16Type());
        return s;
#endif
    }

    template<typename ReturnType>
    inline ReturnType ReadUTF8CharWithoutTag(Char16Type) {
        return ReturnType(1, ReadUTF8CharAsChar());
    }

    template<typename ReturnType>
    inline ReturnType ReadUTF8CharWithoutTag(Char32Type) {
        return ReadUTF8CharWithoutTag<ReturnType>(Char16Type());
    }

    template<typename ReturnType>
    inline ReturnType ReadUTF8CharWithoutTag() {
        typedef BOOST_DEDUCED_TYPENAME NonCVElementType<ReturnType>::type element;
        return ReadUTF8CharWithoutTag<ReturnType>(CharSize<sizeof(element)>());
    }

    inline std::string ReadUTF8CharWithoutTag() {
        return ReadUTF8CharWithoutTag<std::string>();
    }

    template<typename ReturnType>
    ReturnType ReadStringWithoutTag() {
        typedef BOOST_DEDUCED_TYPENAME NonCVElementType<ReturnType>::type element;
        std::basic_string<char_type> chars = ReadChars();
        references.push_back(chars);
        return ReadCharsAsString<ReturnType>(chars, CharSize<sizeof(element)>());
    }

    inline std::string ReadStringWithoutTag() {
        return ReadStringWithoutTag<std::string>();
    }

#ifdef BOOST_WINDOWS
    GUID ReadGUIDWithoutTag() {
        char buf[39];
        for (int i = 0; i < 38; i++) {
            buf[i] = stream.get();
        }
        buf[38] = 0;
        char temp[19]; // 16 + (4 - 1)
        GUID *g = (GUID *)&temp[0];
#ifdef __STDC_WANT_SECURE_LIB__
        sscanf_s(buf,
#else
        sscanf(buf,
#endif
            "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            &g->Data1, &g->Data2, &g->Data3, &g->Data4[0], &g->Data4[1], &g->Data4[2],
            &g->Data4[3], &g->Data4[4], &g->Data4[5], &g->Data4[6], &g->Data4[7]);
        references.push_back(*g);
        return *g;
    }
#endif

    template<typename ReturnType>
    ReturnType ReadListWithoutTag() {
        typedef BOOST_DEDUCED_TYPENAME NonCVElementType<ReturnType>::type element;
        int count = ReadInt(HproseTags::TagOpenbrace);
        ReturnType a;
        a.resize(count);
        for (int i = 0; i < count; i++) {
            a[i] = Unserialize<element>();
        }
        stream.get();
        references.push_back(a);
        return a;
    }

    inline std::vector<Any> ReadListWithoutTag() {
        return ReadListWithoutTag<std::vector<Any> >();
    }

    template<typename ReturnType>
    ReturnType ReadMapWithoutTag() {
        typedef BOOST_DEDUCED_TYPENAME NonCVKeyType<ReturnType>::type key;
        typedef BOOST_DEDUCED_TYPENAME NonCVValueType<ReturnType>::type value;
        int count = ReadInt(HproseTags::TagOpenbrace);
        ReturnType map;
        for (int i = 0; i < count; i++) {
            key k = Unserialize<key>();
            value v = Unserialize<value>();
            map[k] = v;
        }
        stream.get();
        references.push_back(map);
        return map;
    }

    inline std::tr1::unordered_map<std::string, Any> ReadMapWithoutTag() {
        return ReadMapWithoutTag<std::tr1::unordered_map<std::string, Any> >();
    }

    void ReadClass() {
        std::string name = ReadString();
        int count = ReadInt(HproseTags::TagOpenbrace);
        std::vector<std::string> attrs;
        attrs.reserve(count);
        for (int i = 0; i < count; i++) {
            attrs.push_back(ReadStringWithoutTag());
        }
        CheckTag(HproseTags::TagClosebrace);
        const std::type_info * type = ClassManager::SharedInstance()->GetClass(name);
        if (type) {
            classRef.push_back(type);
            attrsRef[type] = attrs;
        }
    }

    template<typename ReturnType>
    ReturnType ReadObjectWithoutTag() {
//        const std::type_info * type = classRef[ReadInt(HproseTags::TagOpenbrace)];
//        std::vector<std::string> & attrs = attrsRef[type];
//        std::string alias = ClassManager::SharedInstance()->GetClassAlias(type);
//        ClassManager::ClassProperty * m  = ClassManager::SharedInstance()->GetClassProperty(alias);
//        references.push_back(&o);
//        for (size_t i = 0; i < attrs.size(); i++) {
//            (*m)[attrs[i]].Unserialize(*this, o);
//        }
//        CheckTag(HproseTags::TagClosebrace);
    }


    template<typename ReturnType>
    ReturnType ReadRef() {
        if (simple) {
            HPROSE_THROW_EXCEPTION("Not supported yet.");
        }
        Any a = references[ReadIntWithoutTag()];
        try {
            return Any::cast<ReturnType>(a);
        } catch (...) {
            HPROSE_THROW_EXCEPTION(CastError(a.type().name(), typeid(ReturnType).name()));
        }
    }

    template<typename ReturnType>
    inline ReturnType ReadStringRef() {
        typedef BOOST_DEDUCED_TYPENAME NonCVElementType<ReturnType>::type element;
        std::basic_string<char_type> chars = ReadRef<std::basic_string<char_type> >();
        return ReadCharsAsString<ReturnType>(chars, CharSize<sizeof(element)>());
    }

    inline std::string ReadStringRef() {
        return ReadStringRef<std::string>();
    }

private:

    inline std::string UnexpectedTag(char tag) {
        if (tag == -1) {
            return "No byte found in stream";
        } else {
            return std::string("Unexpected serialize tag '") + tag + "' in stream";
        }
    }

    inline std::string CastError(const std::string & srctype, const std::string & desttype) {
        return srctype + " can't change to " + desttype;
    }

    template<typename DestType>
    inline std::string CastError(char tag) {
        return CastError(TagToString(tag), typeid(DestType).name());
    }

    std::string TagToString(char tag) {
        switch (tag) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case HproseTags::TagInteger: return "Integer";
            case HproseTags::TagLong: return "Long";
            case HproseTags::TagDouble: return "Double";
            case HproseTags::TagNull: return "Null";
            case HproseTags::TagEmpty: return "Empty String";
            case HproseTags::TagTrue: return "Boolean True";
            case HproseTags::TagFalse: return "Boolean False";
            case HproseTags::TagNaN: return "NaN";
            case HproseTags::TagInfinity: return "Infinity";
            case HproseTags::TagDate: return "DateTime";
            case HproseTags::TagTime: return "DateTime";
            case HproseTags::TagBytes: return "Bytes";
            case HproseTags::TagUTF8Char: return "Char";
            case HproseTags::TagString: return "String";
            case HproseTags::TagGuid: return "Guid";
            case HproseTags::TagList: return "List";
            case HproseTags::TagMap: return "Map";
            case HproseTags::TagClass: return "Class";
            case HproseTags::TagObject: return "Object";
            case HproseTags::TagRef: return "Object Reference";
            case HproseTags::TagError: HPROSE_THROW_EXCEPTION(ReadString());
            default: HPROSE_THROW_EXCEPTION(UnexpectedTag(tag));
        }
    }

private:

    std::istream & stream;
    bool simple;

    std::vector<Any> references;
    std::vector<const std::type_info *> classRef;
    std::tr1::unordered_map<const std::type_info *, std::vector<std::string> > attrsRef;

}; // class HproseReader

} // namespace hprose

#endif // HPROSE_IO_HPROSE_READER_HPP
