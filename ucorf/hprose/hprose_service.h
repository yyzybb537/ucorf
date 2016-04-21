#pragma once

#include "service.h"
#include "hprose_message.h"
#include "hprose_header.h"
#include "client.h"
#include "error.h"
#include "logger.h"
#include "hprose_protocol.h"

namespace ucorf {
namespace hprose {

    struct CalleeBase
    {
        virtual ~CalleeBase() {}
        virtual std::string Call(Buffer & reader) = 0;

        std::string R2Hprose(void)
        {
            Buffer buf;
            buf.Write(hprose::TagResult);
            buf.Write(hprose::TagEmpty);
            buf.Write(hprose::TagEnd);
            return buf.str();
        }

        template <typename R>
        std::string R2Hprose(R const& r)
        {
            Buffer buf;
            buf.Write(hprose::TagResult);
            buf.Write(r);
            buf.Write(hprose::TagEnd);
            return buf.str();
        }
    };

    template <typename F>
    struct Callee;

    template <typename R>
    struct Callee<R()> : public CalleeBase
    {
        typedef boost::function<R()> func_t;
        explicit Callee(func_t const& fn) : fn_(fn) {}

        virtual std::string Call(Buffer & reader) override
        {
            R result = fn_();
            return R2Hprose(result);
        }

        func_t fn_;
    };

    template <typename R, typename Arg>
    struct Callee<R(Arg)> : public CalleeBase
    {
        typedef boost::function<R(Arg)> func_t;
        explicit Callee(func_t const& fn) : fn_(fn) {}

        virtual std::string Call(Buffer & reader) override
        {
            Arg arg;
            if (reader.Read(arg)) {
                R result = fn_(arg);
                return R2Hprose(result);
            }

            return "Es10\"Error Args\"z";
        }

        func_t fn_;
    };

    template <typename R, typename Arg1, typename Arg2>
    struct Callee<R(Arg1, Arg2)> : public CalleeBase
    {
        typedef boost::function<R(Arg1, Arg2)> func_t;
        explicit Callee(func_t const& fn) : fn_(fn) {}

        virtual std::string Call(Buffer & reader) override
        {
            Arg1 arg1;
            Arg2 arg2;
            if (reader.Read(arg1) && reader.Read(arg2)) {
                R result = fn_(arg1, arg2);
                return R2Hprose(result);
            }

            return "Es10\"Error Args\"z";
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

        std::string Call(std::string const& method, Buffer & reader);

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
            Buffer buf;
            buf.Write(hprose::TagCall);
            buf.Write(method);
            RecursiveWrite(buf, std::forward<Args>(args)...);
            buf.Write(hprose::TagEnd);

            Hprose_Message request(buf.str());
            Hprose_Message response;
            ec = c_->Call("", "", &request, &response);
            if (ec) return R();

            Buffer reader(response.body_);
            char flag = reader.get();
            if (flag == hprose::TagResult) {
                R r;
                if (reader.Read(r))
                    return r;
                return R();
            }

            if (flag == hprose::TagError) {
                std::string err;
                if (!reader.Read(err))
                    err = "Parse Response Error";
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
        void RecursiveWrite(Buffer & buf, A && a)
        {
            buf.Write(a);
        }

        template <typename A, typename ... Args>
        void RecursiveWrite(Buffer & buf, A && a, Args && ... args)
        {
            buf.Write(std::forward<A>(a));
            RecursiveWrite(buf, std::forward<Args>(args)...);
        }
    };

} //namespace hprose
} //namespace ucorf
