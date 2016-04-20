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
 * ClassManager.hpp                                       *
 *                                                        *
 * hprose class manager unit for cpp.                     *
 *                                                        *
 * LastModified: Jun 15, 2014                             *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_IO_CLASS_MANAGER_HPP
#define HPROSE_IO_CLASS_MANAGER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "common.hpp"

#include <boost/tr1/unordered_map.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#define HPROSE_REG_CLASS(Class) \
    hprose::ClassManager::SharedInstance()->RegisterClass(&typeid(Class), #Class)
#define HPROSE_REG_CLASS_EX(Class, Alias) \
    hprose::ClassManager::SharedInstance()->RegisterClass(&typeid(Class), Alias)
#define HPROSE_REG_PROPERTY(Class, Property) \
    hprose::ClassManager::SharedInstance()->RegisterProperty(&typeid(Class), #Property, &Class::Property)
#define HPROSE_REG_PROPERTY_EX(Class, Property, Alias) \
    hprose::ClassManager::SharedInstance()->RegisterProperty(&typeid(Class), Alias, &Class::Property)

#define HproseRegClass      HPROSE_REG_CLASS
#define HproseRegClassEx    HPROSE_REG_CLASS_EX
#define HproseRegProperty   HPROSE_REG_PROPERTY
#define HproseRegPropertyEx HPROSE_REG_PROPERTY_EX

#ifndef BOOST_NO_CXX11_VARIADIC_MACROS
#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
#define HPROSE_REGISTER(Class, ...) \
    hprose::ClassManager::SharedInstance()->RegisterClass(&typeid(Class), #Class, __VA_ARGS__)

#define HproseRegister  HPROSE_REGISTER
#endif  
#endif

namespace hprose {

class ClassManager {
public:

    typedef std::tr1::unordered_map<std::string, Any> ClassProperty;

public:

    inline static ClassManager * SharedInstance() {
        static ClassManager classManager;
        return &classManager;
    }

public:

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template<typename... ValueTypes>
    inline void RegisterClass(const std::type_info * type, const std::string & alias, const ValueTypes &... values) {
        if (type && (!alias.empty())) {
            RegisterClass(type, alias);
            RegisterProperty(alias, values...);
        }
    }
#endif

    inline void RegisterClass(const std::type_info * type, const std::string & alias) {   
        if (type && (!alias.empty())) {
#ifdef HPROSE_THREAD_SAFE_REG
            boost::unique_lock<boost::shared_mutex> lock(mutex);
#endif
            classAliases[type] = alias;
            classes[alias] = type;
        }
    }

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template<typename ValueType, typename... ValueTypes>
    inline void RegisterProperty(const std::string & alias, const std::string & name, const ValueType & value, const ValueTypes &... values) {
        if ((!alias.empty()) && (!name.empty())) {
#ifdef HPROSE_THREAD_SAFE_REG
            boost::unique_lock<boost::shared_mutex> lock(propertyMutex);
#endif
            properties[alias][name] = value;
        }
        RegisterProperty(alias, values...);
    }
#endif

    template<typename ValueType>
    inline void RegisterProperty(const std::string & alias, const std::string & name, ValueType value) {
        if ((!alias.empty()) && (!name.empty())) {
#ifdef HPROSE_THREAD_SAFE_REG
            boost::unique_lock<boost::shared_mutex> lock(propertyMutex);
#endif
            properties[alias][name] = value;
        }
    }

    template<typename ValueType>
    inline void RegisterProperty(const std::type_info * type, const std::string & name, ValueType value) {
        if (type) {
            std::string alias = GetClassAlias(type);
            RegisterProperty(alias, name, value);
        }
    }

    inline std::string GetClassAlias(const std::type_info * type) {
        if (type) {
#ifdef HPROSE_THREAD_SAFE_REG
            boost::shared_lock<boost::shared_mutex> lock(mutex);
#endif
            return classAliases[type];
        } else {
            return "";
        }
    }

    inline const std::type_info * GetClass(const std::string & alias) {
        if (!alias.empty()) {
#ifdef HPROSE_THREAD_SAFE_REG
            boost::shared_lock<boost::shared_mutex> lock(mutex);
#endif
            Classes::const_iterator itr = classes.find(alias);
            return (itr != classes.end()) ? itr->second : NULL;
        } else {
            return NULL;
        }
    }

    inline ClassProperty * GetClassProperty(const std::string & alias) {
        if (!alias.empty()) {
#ifdef HPROSE_THREAD_SAFE_REG
            boost::shared_lock<boost::shared_mutex> lock(propertyMutex);
#endif
            ClassProperties::iterator itr = properties.find(alias);
            return (itr != properties.end()) ? &itr->second : NULL;
        } else {
            return NULL;
        }
    }

    inline ClassProperty * GetClassProperty(const std::type_info * type) {
        if (type) {
            std::string alias = GetClassAlias(type);
            return GetClassProperty(alias);
        } else {
            return NULL;
        }
    }

private:

    typedef std::tr1::unordered_map<const std::type_info *, std::string> ClassAliases;
    typedef std::tr1::unordered_map<std::string, const std::type_info *> Classes;
    typedef std::tr1::unordered_map<std::string, ClassProperty> ClassProperties;

private:

    ClassAliases classAliases;
    Classes classes;
    ClassProperties properties;

#ifdef HPROSE_THREAD_SAFE_REG
    boost::shared_mutex mutex; // read-write lock for classAliases and classes;
    boost::shared_mutex propertyMutex; // read-write lock for properties;
#endif

}; // class ClassManager

} // namespace hprose

#endif // HPROSE_IO_CLASS_MANAGER_HPP
