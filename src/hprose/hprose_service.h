#pragma once

#include "service.h"
#include "hprose_message.h"
#include "client.h"
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
        std::string name() override { return "hprose service"; }

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

        std::string name() override { return "hprose service stub"; }

        template <typename R, typename ... Args>
        R CallMethod(std::string const& method, boost_ec & ec, Args && ... args)
        {
            std::stringstream ss;
            hprose::HproseWriter writer(ss);
            ss << hprose::HproseTags::TagCall;
            writer.WriteString(method);
            RecursiveWrite(writer, std::forward(args)...);
            ss << hprose::HproseTags::TagEnd;
            Hprose_Message request(ss.str());
            Hprose_Message response;
            ec = c_->Call("", "", &request, &response);
            if (ec) return R();

            std::stringstream rs;
            rs.rdbuf()->sputn(response.body_.data(), response.body_.size());
            hprose::HproseReader reader(rs);
            return reader.Read<R>();
        }

        template <typename R, typename ... Args>
        R CallMethodNE(std::string const& method, Args && ... args)
        {
            boost_ec ec;
            return CallMethod(method, ec, std::forward(args)...);
        }

        template <typename A, typename ... Args>
        void RecursiveWrite(hprose::HproseWriter & writer, A && a, Args && ... args)
        {
            writer.Write(a);
            if (sizeof...(Args)) {
                RecursiveWrite(writer, std::forward(args)...);
            }
        }
    };

} //namespace ucorf
