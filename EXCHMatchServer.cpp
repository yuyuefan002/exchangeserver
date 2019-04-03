#include "exchangeserver.h"
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#define THREAD_NUM 1000
std::mutex q_mtx;
std::queue<int> clients;
void server_func() {
  while (1) {
    while (clients.empty()) {
    }
    q_mtx.lock();
    if (clients.empty()) {
      q_mtx.unlock();
      continue;
    }
    int newfd = clients.front();
    clients.pop();
    q_mtx.unlock();
    EXCHANGESERVER Server;
    Server.handler(newfd);
    close(newfd);
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage:%s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  EXCHANGESERVER Server(argv[1]);
  if (Server.DBinitializer() == -1) {
    fprintf(stderr, "failed to initialize database\n");
  }
  std::thread t[THREAD_NUM];
  for (int i = 0; i < THREAD_NUM; i++)
    t[i] = std::thread(server_func);

  while (1) {
    int newfd = Server.accNewRequest();
    // server_func(newfd);
    clients.push(newfd);
    // t.detach();
  }
  for (int j = 0; j < THREAD_NUM; j++)
    t[j].join();
  return EXIT_SUCCESS;
}
