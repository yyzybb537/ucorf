#pragma once

#include "service.h"
#include "hprose_message.h"
#include "hprose_header.h"
#include "client.h"
#include "error.h"
#include "logger.h"
#include "io.hpp"

namespace ucorf
{
    struct CalleeBase
    {
        virtual ~CalleeBase() {}
        virtual std::string Call(hprose::HproseReader & reader) = 0;

        std::string R2Hprose(void)
        {
            std::stringstream ss;
            hprose::HproseWriter writer(ss);
            ss << hprose::HproseTags::TagResult;
            writer.WriteEmpty();
            ss << hprose::HproseTags::TagEnd;
            return ss.str();
        }

        template <typename R>
        std::string R2Hprose(R const& r)
        {
            std::stringstream ss;
            hprose::HproseWriter writer(ss);
            ss << hprose::HproseTags::TagResult;
            writer.Write(r);
            ss << hprose::HproseTags::TagEnd;
            return ss.str();
        }
    };

    template <typename F>
    struct Callee;

    template <typename R, typename Arg>
    struct Callee<R(Arg)> : public CalleeBase
    {
        typedef boost::function<R(Arg)> func_t;
        explicit Callee(func_t const& fn) : fn_(fn) {}

        virtual std::string Call(hprose::HproseReader & reader) override
        {
            Arg arg = reader.Read<Arg>();
            R result = fn_(arg);
            return R2Hprose(result);
        }

        func_t fn_;
    };

    template <typename R, typename Arg1, typename Arg2>
    struct Callee<R(Arg1, Arg2)> : public CalleeBase
    {
        typedef boost::function<R(Arg1, Arg2)> func_t;
        explicit Callee(func_t const& fn) : fn_(fn) {}

        virtual std::string Call(hprose::HproseReader & reader) override
        {
            Arg1 arg1 = reader.Read<Arg1>();
            Arg2 arg2 = reader.Read<Arg2>();
            R result = fn_(arg1, arg2);
            return R2Hprose(result);
        }

        func_t fn_;
    };

    class Hprose_Service : public IService
    {
    public:
        std::string name() override { return "hprose"; }

        template <typename R, typename ... Args>
        bool RegisterFunction(std::string const& method, boost::function<R(Args)...> const& fn)
        {
            std::unique_lock<co_mutex> lock(func_mutex_);
            return functions_.insert(std::make_pair(method,
                        boost::static_pointer_cast<CalleeBase>(boost::make_shared<Callee<R(Args)...>>(fn))
                        )).second;
        }

        std::unique_ptr<IMessage> CallMethod(std::string const&,
                const char *request_data, size_t request_bytes) override;

    private:
        std::string GetFunctionList();

        std::string Call(std::string const& method, hprose::HproseReader & reader);

        co_mutex func_mutex_;
        std::map<std::string, boost::shared_ptr<CalleeBase>> functions_;
    };

    class Hprose_ServiceStub : public IServiceStub
    {
    public:
        using IServiceStub::IServiceStub;

        std::string name() override { return "hprose"; }

        template <typename R, typename ... Args>
        R CallMethod(std::string const& method, boost_ec & ec, Args && ... args)
        {
            std::ostringstream ss;
            hprose::HproseWriter writer(ss);
            ss << hprose::HproseTags::TagCall;
            writer.WriteString(method);
            RecursiveWrite(writer, std::forward<Args>(args)...);
            ss << hprose::HproseTags::TagEnd;
            Hprose_Message request(ss.str());
            Hprose_Message response;
            ec = c_->Call("", "", &request, &response);
            if (ec) return R();

            std::istringstream rs(response.body_);
            hprose::HproseReader reader(rs);
            char flag = rs.get();
            if (flag == hprose::HproseTags::TagResult)
                return reader.Read<R>();

            if (flag == hprose::HproseTags::TagError) {
                std::string err = reader.ReadString();
                ucorf_log_error("returns error:%s", err.c_str());
                ec = MakeUcorfErrorCode(eUcorfErrorCode::ec_logic_error);
                return R();
            }

            ucorf_log_error("request type error: 0x%x", (int)flag);
            ec = MakeUcorfErrorCode(eUcorfErrorCode::ec_call_error);
            return R();
        }

        template <typename R, typename ... Args>
        R CallMethodNE(std::string const& method, Args && ... args)
        {
            boost_ec ec;
            return CallMethod(method, ec, std::forward<Args>(args)...);
        }

        template <typename A, typename ... Args>
        void RecursiveWrite(hprose::HproseWriter & writer, A && a)
        {
            writer.Write(a);
        }

        template <typename A, typename ... Args>
        void RecursiveWrite(hprose::HproseWriter & writer, A && a, Args && ... args)
        {
            writer.Write(std::forward<A>(a));
            RecursiveWrite(writer, std::forward<Args>(args)...);
        }
    };

} //namespace ucorf
