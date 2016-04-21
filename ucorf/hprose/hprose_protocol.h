#pragma once

#include <type_traits>
#include <string>
#include <stdlib.h>
#include <limits>
#include <time.h>
#include <chrono>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>

namespace ucorf {
namespace hprose {

/* Serialize Tags */
const char TagInteger     = 'i';
const char TagLong        = 'l';
const char TagDouble      = 'd';
const char TagNull        = 'n';
const char TagEmpty       = 'e';
const char TagTrue        = 't';
const char TagFalse       = 'f';
const char TagNaN         = 'N';
const char TagInfinity    = 'I';
const char TagDate        = 'D';
const char TagTime        = 'T';
const char TagUTC         = 'Z';
const char TagBytes       = 'b';
const char TagUTF8Char    = 'u';
const char TagString      = 's';
const char TagGuid        = 'g';  
const char TagList        = 'a';
const char TagMap         = 'm';
const char TagClass       = 'c';
const char TagObject      = 'o';
const char TagRef         = 'r';
/* Serialize Marks */
const char TagPos         = '+';
const char TagNeg         = '-';
const char TagSemicolon   = ';';
const char TagOpenbrace   = '{';
const char TagClosebrace  = '}';
const char TagQuote       = '"';
const char TagPoint       = '.';
/* Protocol Tags */
const char TagFunctions   = 'F';
const char TagCall        = 'C';
const char TagResult      = 'R';
const char TagArgument    = 'A';
const char TagError       = 'E';
const char TagEnd         = 'z';

template <typename Container>
struct has_mapped_type
{
    template <typename T>
    static void test(T *, typename T::mapped_type* = nullptr) noexcept(true);

    template <typename T>
    static void test(void *, ...) noexcept(false);

    static const bool value = noexcept(test<Container>((Container*)nullptr));
};

struct Buffer
{
    std::string s_;
    size_t read_pos_ = 0;

    struct rb_sentry
    {
        Buffer *buf_ = nullptr;
        size_t pos_ = 0;
        bool commit_ = false;

        explicit rb_sentry(Buffer * b) : buf_(b) {
            pos_ = buf_->read_pos_;
        }
        size_t begin() { return pos_; }
        void commit() { commit_ = true; }
        ~rb_sentry() {
            if (!commit_)
                buf_->read_pos_ = pos_;
        }
    };

    Buffer() {}
    explicit Buffer(std::string const& s) : s_(s) {}
    Buffer(const void* b, size_t len) : s_((const char*)b, len) {}

    std::string const& str()
    {
        return s_;
    }

    int get()
    {
        if (read_pos_ >= s_.size()) return -1;
        return (int)(unsigned char)s_[read_pos_++];
    }

    void rollback(size_t n = 1)
    {
        read_pos_ -= n;
    }

    // 结果不包含字符c
    std::string ReadUntil(char c)
    {
        rb_sentry rb(this);
        for (;;) {
            int v = get();
            if (v == -1)
                return "";

            if (v == c) {
                rb.commit();
                return s_.substr(rb.begin(), read_pos_ - rb.begin());
            }

            ++v;
        }
    }

    // 取N个bytes
    std::string ReadN(size_t n)
    {
        if (read_pos_ + n < s_.size()) {
            read_pos_ += n;
            return s_.substr(read_pos_ - n, n);
        }

        return "";
    }

    // integer and long
    template <typename Integer>
    typename std::enable_if<std::is_integral<Integer>::value && !std::is_same<Integer, bool>::value,
             bool>::type
    Read(Integer & i)
    {
        rb_sentry rb(this);
        bool b = __Read(i);
        if (b) rb.commit();
        return b;
    }

    template <typename Integer>
    typename std::enable_if<std::is_integral<Integer>::value && !std::is_same<Integer, bool>::value,
             bool>::type
    __Read(Integer & i)
    {
        int v = get();
        if (v == -1) return false;

        if (v >= '0' && v <= '9') {
            // 个位数
            i = (int)(v - '0');
            return true;
        }

        if (v == TagInteger) {
            // Integer
            std::string const& s = ReadUntil(TagSemicolon);
            i = (int)atoi(s.c_str());
            return true;
        }

        if (v == TagLong) {
            // Integer
            std::string const& s = ReadUntil(TagSemicolon);
            i = (long long)atoll(s.c_str());
            return true;
        }

        return false;
    }

    // double (float)
    template <typename Double>
    typename std::enable_if<std::is_floating_point<Double>::value,
             bool>::type
    Read(Double & f)
    {
        rb_sentry rb(this);
        bool b = __Read(f);
        if (b) rb.commit();
        return b;
    }

    template <typename Double>
    typename std::enable_if<std::is_floating_point<Double>::value,
             bool>::type
    __Read(Double & f)
    {
        int v = get();
        if (v == -1) return false;
        if (v == TagNaN) {
            f = std::numeric_limits<Double>::signaling_NaN();
            return true;
        }
        if (v == TagInfinity) {
            int signal = get();
            if (signal == TagPos) {
                f = std::numeric_limits<Double>::infinity();
                return true;
            } else if (signal == TagNeg) {
                f = - std::numeric_limits<Double>::infinity();
                return true;
            }

            return false;
        }
        if (v != TagDouble) return false;
        std::string const& s = ReadUntil(TagSemicolon);
        f = (double)atof(s.c_str());
        return true;
    }

    // bool
    bool Read(bool & b)
    {
        rb_sentry rb(this);
        bool r = __Read(b);
        if (r) rb.commit();
        return r;
    }

    bool __Read(bool & b)
    {
        int v = get();
        if (v == TagTrue) {
            b = true;
            return true;
        } else if (v == TagFalse) {
            b = false;
            return true;
        }
        return false;
    }

    // utf-8 char
    bool ReadUTF8(std::string & c)
    {
        rb_sentry rb(this);
        bool r = __Read(c);
        if (r) rb.commit();
        return r;
    }
    bool __ReadUTF8(std::string & c)
    {
        int v = get();
        if (v != TagUTF8Char) return false;
        int leader = get();
        if (leader == -1) return false;

        int follow_bytes = 0;
        for (; follow_bytes <= 6; ++follow_bytes)
            if ((leader & (1 << follow_bytes)) == 0)
                break;

        std::string r;
        r += leader;
        if (follow_bytes) {
            std::string s = ReadN(follow_bytes);
            if (s.empty()) return false;
            r += s;
            r.swap(c);
            return true;
        }
        return true;
    }

    // Null
    bool Read(nullptr_t null)
    {
        rb_sentry rb(this);
        bool r = __Read(null);
        if (r) rb.commit();
        return r;
    }
    bool __Read(nullptr_t null)
    {
        return (get() == TagNull);
    }

    // DateTime
    // @returns: local time.
    bool Read(time_t &t, long long &nano)
    {
        rb_sentry rb(this);
        bool r = __Read(t, nano);
        if (r) rb.commit();
        return r;
    }
    bool __Read(time_t &t, long long &nano)
    {
        int v = get();
        if (v == -1) return false;
        if (v != TagDate && v != TagTime) return false;

        tm r;
        memset(&r, 0, sizeof(tm));

        bool is_utc = false;
        std::string s = ReadUntil(TagSemicolon);
        if (s.empty()) {
            is_utc = true;
            s = ReadUntil(TagUTC);
        }
        if (s.empty()) return false;

        if (v == TagDate) {
            // Date
            if (s.size() < 8) return false;
            r.tm_year = atoi(s.substr(0, 4).c_str()) - 1900;
            r.tm_mon = atoi(s.substr(4, 2).c_str());
            r.tm_mday = atoi(s.substr(6, 2).c_str());
            s = s.substr(9);
        } else {
            r.tm_year = 70;
            r.tm_mon = 0;
            r.tm_mday = 1;
        }

        if (s.size() >= 6) {
            // Time
            r.tm_hour = atoi(s.substr(0, 2).c_str());
            r.tm_min = atoi(s.substr(2, 4).c_str());
            r.tm_sec = atoi(s.substr(4, 6).c_str());

            if (is_utc) {
                // convert to local time.
                time_t t = timegm(&r);
                localtime_r(&t, &r);
            }
            t = mktime(&r);

            if (s.size() > 6) {
                if (s[6] != TagPoint) return false;
                std::string nanos = s.substr(7);
                if (nanos.size() > 9) return false;
                nanos.resize(9, '0');
                nano = atol(nanos.c_str());
            } else {
                nano = 0;
            }
        }

        return true;
    }

    // Bytes & String
    bool Read(std::string & str, bool utf8 = false)
    {
        rb_sentry rb(this);
        bool r = __Read(str);
        if (r) rb.commit();
        return r;
    }
    bool __Read(std::string & str, bool utf8 = false)
    {
        int v = get();
        if (v == TagEmpty) {
            // empty string
            str.clear();
            return true;
        }

        if (v != TagString) return false;

        std::string len_s = ReadUntil(TagQuote);
        if (len_s.empty()) {
            if (get() == TagQuote) {
                // empty string
                str.clear();
                return true;
            }

            return false;
        }

        std::string result;
        int len = atoi(len_s.c_str());
        if (len <= 0) return false;
        if (utf8) {
            for (int i = 0; i < len; ++i)
            {
                std::string utf8_char;
                if (!ReadUTF8(utf8_char))
                    return false;

                result += utf8_char;
            }
        } else {
            std::string bin = ReadN(len);
            if (bin.length() != (size_t)len) return false;
            result.swap(bin);
        }

        if (get() == TagQuote) {
            str.swap(result);
            return true;
        }

        return false;
    }

    bool Read(boost::uuids::uuid & uuid)
    {
        rb_sentry rb(this);
        bool r = __Read(uuid);
        if (r) rb.commit();
        return r;
    }
    bool __Read(boost::uuids::uuid & uuid)
    {
        if (get() != TagGuid) return false;
        if (get() != TagOpenbrace) return false;
        std::string uuid_s = ReadUntil(TagClosebrace);
        if (uuid_s.empty()) return false;
        try {
            boost::uuids::uuid n_uuid;
            n_uuid = boost::uuids::string_generator()(uuid_s);
            n_uuid.swap(uuid);
            return true;
        } catch (std::exception &e) {
            return false;
        }
    }

    // std::map & std::unordered_map
    // @remarks: use container by iterator.
    template <typename Container>
    typename std::enable_if<has_mapped_type<Container>::value, bool>::type
    Read(Container & container)
    {
        rb_sentry rb(this);
        bool r = __Read(container);
        if (r) rb.commit();
        return r;
    }
    template <typename Container>
    typename std::enable_if<has_mapped_type<Container>::value, bool>::type
    __Read(Container & container)
    {
//        static_assert(false, "No support map.");
        return false;
    }

    // std::vector & std::list & std::array & std::deque & std::set
    // @remarks: use container by iterator.
    template <typename Container>
    typename std::enable_if<!has_mapped_type<Container>::value, bool>::type
    Read(Container & container)
    {
        rb_sentry rb(this);
        bool r = __Read(container);
        if (r) rb.commit();
        return r;
    }

    template <typename Container>
    typename std::enable_if<!has_mapped_type<Container>::value, bool>::type
    __Read(Container & container)
    {
        if (get() != TagList) return false;
        std::string len_s = ReadUntil(TagOpenbrace);
        if (len_s.empty()) {
            if (get() == TagClosebrace) {
                container.clear();
                return true;
            }

            return false;
        }

        int len = atoi(len_s.c_str());
        Container c;
        c.resize(len);
        for (auto & elem : c)
            if (!this->Read(elem))
                return false;

        if (get() == TagClosebrace) {
            container.swap(c);
            return true;
        }

        return false;
    }

    template <typename T>
    void Write(T const& t)
    {
    }
};

} // namespace hprose
} // namespace ucorf

