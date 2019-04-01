#ifndef __GENERATEXML_H__
#define __GENERATEXML_H__
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <string.h>
#define MAXDATASIZE 65536
typedef struct addrinfo addrinfo;
typedef struct timeval timeval;
class GENERATEXML {
 private:
  const char *hostname;
  const char *port;
  int error;
  int sockfd;
  const char *getHost(const char *hostname);
  std::vector<char> recvall(int fd);
  int sendall(int fd, const char *buf, size_t *len);
 public:
  GENERATEXML(const char *h, const char *p);
  ~GENERATEXML();
  std::vector<char> recvServeResponse();
  std::vector<char> basicRecv();
  void Send(const std::vector<char> &msg);
  int getError();
  int getFD();
  
};

#endif
