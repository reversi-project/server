#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdint>

#include "reversi/server/handler.h"

namespace reversi::server {

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace system = boost::system;
namespace beast = boost::beast;
namespace ws = beast::websocket;

using PortT = uint16_t;
using WebSocket = ws::stream<beast::tcp_stream>;
using IoContext = asio::io_context;
using Acceptor = ip::tcp::acceptor;
using Endpoint = ip::tcp::endpoint;
using ThreadPool = asio::thread_pool;
using SystemErrorCode = const system::error_code&;
using FlatBuffer = beast::flat_buffer;
using FlatBufferPtr = std::shared_ptr<FlatBuffer>;
using HandlerWs = Handler<WebSocket*>;
using HandlerPtr = std::shared_ptr<HandlerWs>;
using Actions = ActionList<WebSocket*>;

void RunApp(PortT port, uint16_t thread_count);

}  // namespace reversi::server
