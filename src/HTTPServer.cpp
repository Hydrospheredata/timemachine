#include "HTTPServer.h"
#include "spdlog/spdlog.h"
#include "handlers/HandlerFactory.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocketImpl.h>
#include <iostream>


namespace timemachine {

    class ServerSocketImpl : public Poco::Net::ServerSocketImpl {
    public:
        using Poco::Net::SocketImpl::init;
    };

    class Socket : public Poco::Net::Socket {
    public:
        Socket(const std::string &address)
                : Poco::Net::Socket(new ServerSocketImpl()) {
            const Poco::Net::SocketAddress socket_address(address);
            ServerSocketImpl *socket = static_cast<ServerSocketImpl *>(impl());
            socket->init(socket_address.af());
            socket->setReuseAddress(true);
            socket->setReusePort(false);
            socket->bind(socket_address, false);
            socket->listen();
        }
    };

    void HTTPServer::start(std::shared_ptr<timemachine::DbClient> client) {

        Poco::Net::HTTPServerParams::Ptr parameters = new Poco::Net::HTTPServerParams();
        parameters->setTimeout(client->cfg->http_timeout);
        parameters->setMaxQueued(client->cfg->http_max_queued);
        parameters->setMaxThreads(client->cfg->http_max_threads);
        std::string port(client->cfg->http_port);
        //TODO: move to config
        const Poco::Net::ServerSocket socket(Socket("0.0.0.0:" + port));

        Poco::Net::HTTPServer http_server(new handlers::HandlerFactory(client), socket, parameters);

        http_server.start();
        spdlog::info("http server started on port {}", client->cfg->http_port);
        waitForTerminationRequest();
        spdlog::info("http server stopped on port {}", client->cfg->http_port);
        http_server.stopAll();

    }

}