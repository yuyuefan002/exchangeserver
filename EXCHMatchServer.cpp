#include "exchangeserver.h"
#include <thread>
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
  while (1) {
    int newfd = Server.accNewRequest();
    std::thread t = std::thread(server_func, newfd);
    t.detach();
    // Server.handler(newfd);
  }
  return EXIT_SUCCESS;
}
