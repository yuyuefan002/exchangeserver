#include "exchangeserver.h"
#include <thread>
#include <vector>
void server_func(int newfd) {
  EXCHANGESERVER Server;
  Server.handler(newfd);
  close(newfd);
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
  std::thread t[100];
  int i = 0;
  while (1) {
    int newfd = Server.accNewRequest();
    // server_func(newfd);
    t[i++] = std::thread(server_func, newfd);
    if (i == 100) {
      for (int j = 0; j < 100; j++)
        // t[j].detach();
        t[j].join();
      i = 0;
    }
    // t.detach();
  }
  return EXIT_SUCCESS;
}
// pre-create thread, contend for same resource
// it is slow

/*#include "exchangeserver.h"
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#define THREAD_NUM 1000
std::mutex q_mtx;
std::queue<int> clients;
void server_func_pre_create() {
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
    t[i] = std::thread(server_func_pre_create);

  while (1) {
    int newfd = Server.accNewRequest();
    clients.push(newfd);
  }
  for (int j = 0; j < THREAD_NUM; j++)
    t[j].join();
  return EXIT_SUCCESS;
}
*/
