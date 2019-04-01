#include "server.h"

Server::Server(const char *p) : port(p) {
  signal(SIGPIPE, SIG_IGN);
  const char *hostname = NULL;
  addrinfo *host_info_list;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;
  try {
    if (getaddrinfo(hostname, port, &host_info, &host_info_list) != 0)
      throw std::string("getaddrinfo");
    listener = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                      host_info_list->ai_protocol);
    if (listener == -1)
      throw std::string("socket");

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    // set recv timeout
    if (bind(listener, host_info_list->ai_addr, host_info_list->ai_addrlen) ==
        -1)
      throw std::string("bind");
    if (listen(listener, 100) == -1)
      throw std::string("listen");
  } catch (std::string e) {
    freeaddrinfo(host_info_list);
    throw std::string("server init");
  }
  freeaddrinfo(host_info_list);
}

int Server::acceptNewConn() {
  sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof socket_addr;
  int newfd = accept(listener, (sockaddr *)&socket_addr, &socket_addr_len);
  if (newfd == -1) {
    throw std::string("accept failed");
  }
  return newfd;
}

std::vector<char> Server::recvall(int fd) {
  std::vector<char> msg;
  size_t index = 0;
  int contentlen = 0;
  // Helper helper;
  if (msg.size() < index + MAXDATASIZE)
    msg.resize(index + MAXDATASIZE);
  int nbytes;
  if ((nbytes = recv(fd, &msg.data()[index], MAXDATASIZE - 1, 0)) <= 0) {
    return std::vector<char>();
  } else {
    index += nbytes;
  }

  std::vector<char> pattern{'\n'};
  auto it = std::search(msg.begin(), msg.end(), pattern.begin(), pattern.end());
  std::string len_str(msg.begin(), it);
  contentlen = stoi(len_str);
  msg.erase(msg.begin(), it + 1);
  index -= (it + 1 - msg.begin());
  msg.resize(index);
  for (it = msg.begin(); it != msg.end(); it++) {
    contentlen--;
  }
  while (contentlen) {
    if (msg.size() < index + MAXDATASIZE)
      msg.resize(index + MAXDATASIZE);
    int nbytes;
    if ((nbytes = recv(fd, &msg.data()[index], MAXDATASIZE - 1, 0)) <= 0) {
      break;
    } else {
      index += nbytes;
      contentlen -= nbytes;
    }
  }
  msg.resize(index);
  return msg;
}

std::vector<char> Server::basicRecv(int fd) {
  int index = 0;
  std::vector<char> msg;
  msg.resize(MAXDATASIZE);
  index = recv(fd, &msg.data()[index], MAXDATASIZE - 1, 0);
  if (index == -1)
    throw std::string("recv failed");
  msg.resize(index);
  return msg;
}

int Server::sendall(int fd, const char *buf, size_t *len) {
  size_t total = 0;     // how many bytes we've sent
  int bytesleft = *len; // how many we have left to send
  int n;

  while (total < *len) {
    if ((n = send(fd, buf + total, bytesleft, 0)) == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  *len = total; // return number actually sent here

  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}
std::vector<char> Server::receiveData(int fd) { return recvall(fd); }
void Server::sendData(int fd, const std::vector<char> &msg) {
  size_t sent = 0;
  size_t len = msg.size();
  size_t max = msg.size();
  while (sent < len) {
    sent = len - sent;
    len = sent;
    if (sendall(fd, &msg.data()[max - len], &sent) == -1) {
      throw std::string("send failed");
    }
  }
}
Server::Server() {
  signal(SIGPIPE, SIG_IGN);
  listener = -1;
}
Server::~Server() {
  if (listener != -1)
    close(listener);
}
