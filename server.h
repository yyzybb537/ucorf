#pragma once

#include "preheader.h"
#include "transport.h"
#include "message.h"

namespace ucorf
{
    struct Session
    {
        SessId sess;
        ITransportServer *transport;
        IHeaderPtr header;
    };

    class IService;
    class Server
    {
    public:
        typedef boost::function<IMessage*(std::string const&, std::string const&)> MessageFactory;

        Server& Listen(ITransportServer *transport);

        Server& SetHeaderFactory(HeaderFactory const& head_factory);

        Server& SetMessageFactory(MessageFactory const& msg_factory);

        Server& RegisterTo(std::string const& url);

        bool RegisterService(IService* service);

        void RemoveService(std::string const& service_name);

    private:
        size_t OnReceiveData(ITransportServer *tp, SessId sess_id, const char* data, size_t bytes);

        bool DispatchMsg(Session sess, const char* data, size_t bytes);

    private:
        typedef std::map<std::string, IService*> ServiceMap;
        typedef std::list<ITransportServer*> TransportList;

        ServiceMap services_;
        TransportList transports_;
        HeaderFactory head_factory_;
        MessageFactory msg_factory_;
    };

} //namespace ucorf
