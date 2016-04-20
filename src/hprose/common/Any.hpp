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
 * Any.hpp                                                *
 *                                                        *
 * hprose any unit for cpp.                               *
 *                                                        *
 * LastModified: Jun 15, 2014                             *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_COMMON_ANY_HPP
#define HPROSE_COMMON_ANY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "HproseException.hpp"

#if (defined(__GNUC__) && __GNUC__ >= 3) \
 || defined(_AIX) \
 || (defined(__sgi) && defined(__host_mips)) \
 || (defined(__hpux) && defined(__HP_aCC)) \
 || (defined(linux) && defined(__INTEL_COMPILER) && defined(__ICC))
#define BOOST_AUX_ANY_TYPE_ID_NAME
#include <cstring>
#endif

namespace hprose {

template<typename ValueType>
struct is_const
  : public std::is_const<ValueType> {
};

template<typename ValueType>
struct is_const<ValueType *>
  : public is_const<ValueType> {
};

template<typename Reader, typename Writer>
class BasicAny {
public: // structors

    BasicAny()
      : content(0) {
    }

    template<typename ValueType>
    BasicAny(const ValueType & value)
      : content(new holder<ValueType, void>(value)) {
    }

    template<typename ValueType, typename ClassType>
    BasicAny(ValueType ClassType::*value)
      : content(new holder<ValueType ClassType::*, ClassType>(value)) {
    }

    BasicAny(const BasicAny & other)
      : content(other.content ? other.content->clone() : 0) {
    }

    ~BasicAny() {
        delete content;
    }

public: // modifies

    BasicAny & swap(BasicAny & rhs) {
        std::swap(content, rhs.content);
        return *this;
    }

    template<typename T>
    BasicAny & operator=(const T & rhs) {
        BasicAny<Reader, Writer>(rhs).swap(*this);
        return *this;
    }

    BasicAny & operator=(BasicAny rhs) {
        rhs.swap(*this);
        return *this;
    }

public: // queries

    bool empty() const {
        return !content;
    }

    const std::type_info & type() const {
        return content ? content->type() : typeid(void);
    }

public: // Serialize

    inline void Unserialize(Reader & r) {
        if (content) {
            content->Unserialize(r);
        } else {
            r.Unserialize(*this);
        }
    }

    template<typename Object>
    inline void Unserialize(Reader & r, Object & object) {
        if (content) {
            content->Unserialize(r, &object);
        } else {
            r.Unserialize(*this);
        }
    }

    inline void Serialize(Writer & w) const {
        if (content) {
            content->Serialize(w);
        } else {
            w.WriteNull();
        }
    }

    template<typename Object>
    inline void Serialize(Writer & w, const Object & object) const {
        if (content) {
            content->Serialize(w, &object);
        } else {
            w.WriteNull();
        }
    }

private: // types

    class placeholder {
    public: // structors

        virtual ~placeholder() {
        }

    public: // queries

        virtual const std::type_info & type() const = 0;

        virtual placeholder * clone() const = 0;

        virtual void Unserialize(Reader & r) = 0;

        virtual void Unserialize(Reader & r, void * o) = 0;

        virtual void Serialize(Writer & w) const = 0;

        virtual void Serialize(Writer & w, const void * o) const = 0;

    };

    template<typename ValueType, typename ClassType>
    class holder : public placeholder {
    public: // structors

        holder(const ValueType & value)
          : held(value) {
        }

    public: // queries

        virtual const std::type_info & type() const {
            return typeid(ValueType);
        }

        virtual placeholder * clone() const {
            return new holder(held);
        }

        virtual void Unserialize(Reader & r) {
            Unserialize(r, is_const<ValueType>());
        }

        virtual void Unserialize(Reader & r, void * o) {
            Unserialize(r, o, std::is_member_pointer<ValueType>());
        }

        virtual void Serialize(Writer & w) const {
            w.Serialize(held);
        }

        virtual void Serialize(Writer & w, const void * o) const {
            Serialize(w, o, std::is_member_pointer<ValueType>());
        }

    private:

        inline void Serialize(Writer & w, const void * o, const std::true_type &) const {
            const ClassType * p = static_cast<const ClassType *>(o);
            w.Serialize(p->*held);
        }

        inline void Serialize(Writer & w, const void * o, const std::false_type &) const {
            HPROSE_THROW_EXCEPTION("Can't Serialize none member pointer");
        }

        inline void Unserialize(Reader & r, const std::true_type &) {
            HPROSE_THROW_EXCEPTION("Constant value can't be Unserialized");
        }

        inline void Unserialize(Reader & r, const std::false_type &) {
            held = r.template Unserialize<ValueType>();
        }

        inline void Unserialize(Reader & r, void * o, const std::true_type &) {
            ClassType * p = static_cast<ClassType *>(o);
            r.Unserialize(p->*held);
        }

        inline void Unserialize(Reader & w, void * o, const std::false_type &) {
            HPROSE_THROW_EXCEPTION("Can't Unserialize none member pointer");
        }

    public: // representation

        ValueType held;

    private: // intentionally left unimplemented

        holder & operator=(const holder &);

    };

public:

    template<typename ValueType>
    static ValueType * cast(BasicAny<Reader, Writer> * operand) {
        return operand &&
        #ifdef BOOST_AUX_ANY_TYPE_ID_NAME
            std::strcmp(operand->type().name(), typeid(ValueType).name()) == 0
        #else
            operand->type() == typeid(ValueType)
        #endif
            ? &static_cast<BOOST_DEDUCED_TYPENAME BasicAny<Reader, Writer>::BOOST_NESTED_TEMPLATE holder<ValueType, void> *>(operand->content)->held
            : 0;
    }

    template<typename ValueType>
    inline static const ValueType * cast(const BasicAny<Reader, Writer> * operand) {
        return cast<ValueType>(const_cast<BasicAny<Reader, Writer> *>(operand));
    }

    template<typename ValueType>
    static ValueType cast(BasicAny<Reader, Writer> & operand) {
        typedef BOOST_DEDUCED_TYPENAME std::remove_reference<ValueType>::type nonref;
    #ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
        BOOST_STATIC_ASSERT(!is_reference<nonref>::value);
    #endif
        nonref * result = cast<nonref>(&operand);
        if (!result) {
            HPROSE_THROW_EXCEPTION("Failed conversion using hprose::Any::cast");
        }
        return *result;
    }

    template<typename ValueType>
    inline static ValueType cast(const BasicAny<Reader, Writer> & operand) {
        typedef BOOST_DEDUCED_TYPENAME std::remove_reference<ValueType>::type nonref;
    #ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
        BOOST_STATIC_ASSERT(!is_reference<nonref>::value);
    #endif
        return cast<const nonref &>(const_cast<BasicAny<Reader, Writer> &>(operand));
    }

    template<typename ValueType>
    inline static ValueType * unsafe_cast(BasicAny<Reader, Writer> * operand) {
        return &static_cast<BOOST_DEDUCED_TYPENAME BasicAny<Reader, Writer>::BOOST_NESTED_TEMPLATE holder<ValueType, void> *>(operand->content)->held;
    }

    template<typename ValueType>
    inline static const ValueType * unsafe_cast(const BasicAny<Reader, Writer> * operand) {
        return unsafe_cast<ValueType>(const_cast<BasicAny<Reader, Writer> *>(operand));
    }

private: // representation

    placeholder * content;

};

class HproseReader;
class HproseWriter;

typedef BasicAny<HproseReader, HproseWriter> Any;

template<typename ValueType>
int vector_index_of(const std::vector<Any> & container, ValueType & value) {
    std::vector<Any>::const_iterator iter = container.begin();
    while (iter != container.end()) {
        if ((*iter).type() == typeid(ValueType)) {
            if (Any::cast<ValueType>(*iter) == value) {
                return (int)(iter - container.begin());
            }
        }
        iter++;
    }
    return -1;
}

struct type_info_less {
    bool operator()(const std::type_info * left, const std::type_info * right) const {
        return left->before(*right) != 0;
    }
};

} // namespace hprose

#endif // HPROSE_COMMON_ANY_HPP
