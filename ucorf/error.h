#pragma once
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <string>

namespace ucorf
{

    using boost_ec = boost::system::error_code;

    enum class eUcorfErrorCode : int
    {
        ec_ok           = 0,
        ec_snd_timeout  = 1,
        ec_rcv_timeout  = 2,
        ec_call_error   = 3,
        ec_parse_error  = 4,
        ec_no_estab     = 5,
        ec_unsupport_protocol   = 6,
        ec_req_wnd_full = 7,
    };

    class ucorf_error_category
        : public boost::system::error_category
    {
    public:
        virtual const char* name() const BOOST_SYSTEM_NOEXCEPT;

        virtual std::string message(int) const;
    };

    const boost::system::error_category& GetUcorfErrorCategory();

    boost_ec MakeUcorfErrorCode(eUcorfErrorCode code);

    void ThrowError(eUcorfErrorCode code, const char* what = "");

} //namespace ucorf
