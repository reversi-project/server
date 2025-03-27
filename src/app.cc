#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdint>
#include <memory>

#include "app.h"
#include "server.h"

namespace reversi::server {

namespace {

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace system = boost::system;
namespace beast = boost::beast;
namespace ws = beast::websocket;

using IoContext = asio::io_context;
using Acceptor = ip::tcp::acceptor;
using Endpoint = ip::tcp::endpoint;
using ThreadPool = asio::thread_pool;
using SystemErrorCode = const system::error_code&;
using FlatBuffer = beast::flat_buffer;
using FlatBufferPtr = std::shared_ptr<FlatBuffer>;

void StartWork(ThreadPool& tp, IoContext& context, uint16_t thread_count) {
  for (auto i = 0; i < thread_count; ++i) {
    asio::post(tp, [&context]() {
      const auto guard = asio::make_work_guard(context);
      context.run();
    });
  }
}

void TuneAcceptor(Acceptor& acceptor, const Endpoint& endpoint) {
  acceptor.open(endpoint.protocol());
  acceptor.set_option(Acceptor::reuse_address(true));
  acceptor.bind(endpoint);
}

struct Session {
  WebSocket ws;
  ServerPtr server;

  auto& GetSocket() { return beast::get_lowest_layer(ws).socket(); }
};

using SessionPtr = std::shared_ptr<Session>;

struct Scope {
  IoContext context;
  Acceptor acceptor;
  ServerPtr server;

  Scope()
      : context{IoContext{}},
        acceptor{context},
        server(std::make_shared<Server>()) {}

  [[nodiscard]] SessionPtr MakeSession() {
    auto ws = WebSocket{context};
    ws.set_option(
        ws::stream_base::timeout::suggested(beast::role_type::server));

    return std::make_shared<Session>(std::move(ws), server);
  }
};

void OnReadAsync(SessionPtr session, FlatBufferPtr buf, BeastErrorCode ec);

void OnAcceptAsync(Scope& sc, SessionPtr session, SystemErrorCode ec);

void OnWriteAsync(SessionPtr session, BeastErrorCode ec);

void ReadAsync(SessionPtr session) {
  auto buf = std::make_shared<FlatBuffer>();
  session->ws.async_read(
      *buf, [session, buf](BeastErrorCode ec, std::size_t /*bytes*/) {
        OnReadAsync(session, buf, ec);
      });
}

void WriteAsyncSilentCallback(WebSocket* ws, const std::string& message) {
  ws->async_write(asio::buffer(message),
                  [](BeastErrorCode /*ec*/, std::size_t /*bytes*/) {});
}

void WriteAsyncCallback(ServerPtr server, WebSocket* ws,
                        const std::string& message) {
  ws->async_write(asio::buffer(message), [server, ws](BeastErrorCode ec,
                                                      std::size_t /*bytes*/) {
    if (server->HandleError(ec, ws, &WriteAsyncSilentCallback)) {
      return;
    }
  });
}

void AcceptAsync(Scope& sc) {
  auto session = sc.MakeSession();
  sc.acceptor.async_accept(
      session->GetSocket(),
      [&sc, session](SystemErrorCode ec) { OnAcceptAsync(sc, session, ec); });
}

void OnReadAsync(SessionPtr session, FlatBufferPtr buf, BeastErrorCode ec) {
  if (session->server->HandleError(ec, &session->ws,
                                   &WriteAsyncSilentCallback)) {
    return;
  }

  auto data = buf->data();
  auto req = std::string{asio::buffers_begin(data), asio::buffers_end(data)};
  auto& server = session->server;

  server->HandleRequest(req, &session->ws,
                        [server](WebSocket* ws, const std::string& message) {
                          return WriteAsyncCallback(server, ws, message);
                        });
  ReadAsync(session);
}

void OnAcceptAsync(Scope& sc, SessionPtr session, SystemErrorCode ec) {
  if (session->server->HandleError(ec, &session->ws,
                                   &WriteAsyncSilentCallback)) {
    return;
  }

  session->ws.async_accept([session](BeastErrorCode ec) {
    if (session->server->HandleError(ec, &session->ws,
                                     &WriteAsyncSilentCallback)) {
      return;
    }
    ReadAsync(session);
  });

  AcceptAsync(sc);
}

}  // namespace

void RunApp(uint16_t port, uint16_t thread_count) {
  auto sc = Scope{};
  auto tp = ThreadPool{thread_count};
  const auto endpoint = Endpoint{ip::tcp::v4(), port};

  StartWork(tp, sc.context, thread_count);
  TuneAcceptor(sc.acceptor, endpoint);
  sc.acceptor.listen();

  for (auto i = 0; i < thread_count; ++i) {
    AcceptAsync(sc);
  }

  tp.join();
  tp.stop();
}

}  // namespace reversi::server
