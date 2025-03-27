#include <thread>

#include "app.h"

int main() {
  const auto port = 8080;
  const auto thread_count = std::thread::hardware_concurrency();

  reversi::server::RunApp(port, thread_count);
}
