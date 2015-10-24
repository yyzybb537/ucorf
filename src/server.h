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

        Server& BindTransport(std::unique_ptr<ITransportServer> && transport);

        Server& SetHeaderFactory(HeaderFactory const& head_factory);

        Server& RegisterTo(std::string const& url);

        bool RegisterService(std::shared_ptr<IService> service);

        void RemoveService(std::string const& service_name);

    private:
        size_t OnReceiveData(ITransportServer *tp, SessId sess_id, const char* data, size_t bytes);

        bool DispatchMsg(Session sess, const char* data, size_t bytes);

    private:
        typedef std::map<std::string, std::shared_ptr<IService>> ServiceMap;
        typedef std::list<std::unique_ptr<ITransportServer>> TransportList;

        ServiceMap services_;
        TransportList transports_;
        HeaderFactory head_factory_;
    };

} //namespace ucorf
