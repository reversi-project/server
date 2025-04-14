#include <cstdint>
#include <memory>

#include "reversi/server/app.h"

namespace reversi::server {

namespace {

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

struct Connection {
  WebSocket ws;
  HandlerPtr handler;

  auto& GetSocket() { return beast::get_lowest_layer(ws).socket(); }
};

using ConnectionPtr = std::shared_ptr<Connection>;

struct Scope {
  IoContext context;
  Acceptor acceptor;
  HandlerPtr handler;

  Scope()
      : context{IoContext{}},
        acceptor{context},
        handler(std::make_shared<HandlerWs>()) {}

  [[nodiscard]] ConnectionPtr MakeConnection() {
    auto ws = WebSocket{context};
    ws.set_option(
        ws::stream_base::timeout::suggested(beast::role_type::server));

    return std::make_shared<Connection>(std::move(ws), handler);
  }
};

bool HandleError(ConnectionPtr conn, BeastErrorCode ec) {
  const auto actions_opt = conn->handler->HandleError(&conn->ws, ec);
  if (!actions_opt) {
    return false;
  }
  for (const auto& action : actions_opt.value()) {
    const auto& ws = action.user_id;
    const auto raw = contract::ResponseToRaw(action.res);
    ws->async_write(asio::buffer(raw),
                    [](BeastErrorCode ec, std::size_t /*bytes*/) {});
  }
  return true;
}

void DoActions(ConnectionPtr conn, const Actions& actions) {
  for (const auto& action : actions) {
    const auto& ws = action.user_id;
    const auto raw = contract::ResponseToRaw(action.res);
    ws->async_write(asio::buffer(raw),
                    [conn](BeastErrorCode ec, std::size_t /*bytes*/) {
                      HandleError(conn, ec);
                    });
  }
}

void OnReadAsync(ConnectionPtr conn, FlatBufferPtr buf, BeastErrorCode ec);

void OnAcceptAsync(Scope& sc, ConnectionPtr conn, SystemErrorCode ec);

void OnWriteAsync(ConnectionPtr conn, BeastErrorCode ec);

void ReadAsync(ConnectionPtr conn) {
  auto buf = std::make_shared<FlatBuffer>();
  conn->ws.async_read(*buf,
                      [conn, buf](BeastErrorCode ec, std::size_t /*bytes*/) {
                        OnReadAsync(conn, buf, ec);
                      });
}

void AcceptAsync(Scope& sc) {
  auto conn = sc.MakeConnection();
  sc.acceptor.async_accept(conn->GetSocket(), [&sc, conn](SystemErrorCode ec) {
    OnAcceptAsync(sc, conn, ec);
  });
}

void OnReadAsync(ConnectionPtr conn, FlatBufferPtr buf, BeastErrorCode ec) {
  if (HandleError(conn, ec)) {
    return;
  }

  auto data = buf->data();
  auto req = std::string{asio::buffers_begin(data), asio::buffers_end(data)};
  std::cout << "[log] req is " << req << "\n";

  const auto actions = conn->handler->HandleRequest(&conn->ws, req);
  DoActions(conn, actions);
  ReadAsync(conn);
}

void OnAcceptAsync(Scope& sc, ConnectionPtr conn, SystemErrorCode ec) {
  if (HandleError(conn, ec)) {
    return;
  }

  conn->ws.async_accept([conn](BeastErrorCode ec) {
    if (HandleError(conn, ec)) {
      return;
    }
    ReadAsync(conn);
  });

  AcceptAsync(sc);
}

}  // namespace

void RunApp(PortT port, uint16_t thread_count) {
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
