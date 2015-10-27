#pragma once

#include <string>
#include <map>
#include <list>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>
#include <boost/any.hpp>
#include <coroutine/coroutine.h>

// linux
#include <arpa/inet.h>

namespace ucorf
{
    using boost_ec = ::boost::system::error_code;

} //namespace ucorf
