#pragma once

#include "preheader.h"
#include "transport.h"

namespace ucorf
{
    struct Session
    {
        SessId sess;
        ITransportServer *transport;
    };

    class IService;
    class Server
    {
    public:
        bool Listen(ITransportServer *transport);

        Server& RegisterTo(std::string const& url);

        bool RegisterService(IService* service);

        void RemoveService(std::string const& service_name);

    private:
        void OnMsg(Session sess, IMessage *request);

    private:
        typedef std::map<std::string, IService*> ServiceMap;
        typedef std::list<ITransportServer*> TransportList;

        ServiceMap services_;
        TransportList transports_;
    };

} //namespace ucorf
