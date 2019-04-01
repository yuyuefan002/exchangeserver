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
        t[j].join();
      i = 0;
    }
    // t.detach();
  }
  return EXIT_SUCCESS;
}
