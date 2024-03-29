#include "generatexml.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdio.h>  /* printf, scanf, puts, NULL */
#include <stdlib.h> /* srand, rand */
#include <string>
#include <thread>
#include <time.h>
#include <vector>
#define THREAD_NUM 50 // thread number
#define REQUEST_NUM 1 // request number of pre thread
using namespace std::chrono;

int GENERATEXML::sendall(int fd, const char *buf, size_t *len) {
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
  *len = total;            // return number actually sent here
  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

const char *GENERATEXML::getHost(const char *hostname) {
  struct hostent *he = gethostbyname(hostname);
  if (he == nullptr)
    throw std::string("no host");
  struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
  return inet_ntoa(*addr_list[0]);
}

void GENERATEXML::Send(const std::vector<char> &msg) {
  size_t sent = 0;
  size_t len = msg.size();
  size_t max = msg.size();
  while (sent < len) {
    sent = len - sent;
    len = sent;
    if (sendall(sockfd, &msg.data()[max - len], &sent) == -1) {
      throw std::string("send failed");
    }
  }
}

std::vector<char> GENERATEXML::recvall(int fd) {
  // set recv timeout
  timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv))
    throw std::string("setsockopt");

  std::vector<char> msg;
  size_t index = 0;
  int nbytes;
  while (1) {
    if (msg.size() < index + MAXDATASIZE)
      msg.resize(index + MAXDATASIZE);
    nbytes = recv(fd, &msg.data()[index], MAXDATASIZE - 1, 0);
    if (nbytes == -1 && msg.empty()) {
      break;
    } else if (nbytes <= 0) {
      break;

    } else {
      index += nbytes;
    }
  }
  msg.resize(index);
  return msg;
}

std::vector<char> GENERATEXML::basicRecv() {
  int index = 0;
  std::vector<char> msg;
  msg.resize(MAXDATASIZE);
  index = recv(sockfd, &msg.data()[index], MAXDATASIZE - 1, 0);
  if (index == -1)
    throw std::string("recv failed");
  msg.resize(index);
  return msg;
}
std::vector<char> GENERATEXML::recvServeResponse() { return recvall(sockfd); }

int GENERATEXML::getError() { return error; }

int GENERATEXML::getFD() { return sockfd; }

/*
 * initializer
 * This function initialize the socket and connect to server
 * status: uncomplete, exception
 *
 */
GENERATEXML::GENERATEXML(const char *h, const char *p) : port(p) {
  hostname = getHost(h);
  error = 0;
  addrinfo host_info;
  addrinfo *host_info_list;

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  try {
    if (getaddrinfo(hostname, port, &host_info, &host_info_list) != 0)
      throw std::string("getaddrinfo");

    sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                    host_info_list->ai_protocol);
    if (sockfd == -1)
      throw std::string("socket");

    if ((connect(sockfd, host_info_list->ai_addr,
                 host_info_list->ai_addrlen)) == -1)
      throw std::string("connect");
  } catch (std::string e) {
    error = 1;
  }
  freeaddrinfo(host_info_list);
}
GENERATEXML::~GENERATEXML() { close(sockfd); }

void client_func(const char *hostname, const char *port_num,
                 std::ofstream &myfile) {
  // std::cout<<"-------------------"<<std::endl;
  // auto start = high_resolution_clock::now();
  int i = REQUEST_NUM;
  while (i--) {
    GENERATEXML generatexml(hostname, port_num);

    srand((unsigned int)time(NULL) + 0);
    /* generate secret number between 1 and 10: */
    int iSecret = rand() % 5 + 1;
    std::string str;
    // iSecret = 4;
    switch (iSecret) {
    case 1:
      str = "165\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<create>\n"
            "<account id=\"123457\" balance=\"10000\"/>\n"
            "<symbol sym=\"SPY\">\n"
            "<account id=\"123456\">1000000</account>\n"
            "</symbol>\n"
            "</create>\n";
      break;

    case 2:
      str = "159\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<create>\n<account id=\"6789\" balance=\"3000\"/>\n"
            "<symbol sym=\"USD\">\n"
            "<account id=\"6789\">100000</account>\n"
            "</symbol>\n"
            "</create>\n";
      break;

    case 3:
      str = "137\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<transactions id=\"2\">\n"
            "<order sym=\"BIT\" amount=\"100\" limit=\"100\"/>\n"
            "<query id=\"1\"/>\n"
            "</transactions>\n";
      break;

    case 4:
      str = "138\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<transactions id=\"1\">\n"
            "<order sym=\"BIT\" amount=\"-100\" limit=\"100\"/>\n"
            "<query id=\"1\"/>\n"
            "</transactions>\n";
      break;

    default:
      str = "154\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<transactions id=\"2\">\n"
            "<order sym=\"BIT\" amount=\"100\" limit=\"100\"/>\n"
            "<query id=\"1\"/>\n"
            "<cancel id=\"1\"/>\n"
            "</transactions>\n";
    }
    // std::cout<<iSecret<<std::endl;
    std::vector<char> s(str.begin(), str.end());
    // for(int i = 0;i<200;i++){
    generatexml.Send(s);
    //}

    // for(int i = 0;i<200;i++){
    std::vector<char> test = generatexml.recvServeResponse();
    // std::cout<<test.data()<<std::endl;
    //}
  }
  // auto stop = high_resolution_clock::now();
  // auto duration = duration_cast<microseconds>(stop - start);

  // myfile
  //<< "Time taken by function: "
  //<< duration.count()<<std::endl;
  //<< " microseconds" << std::endl;
}

void create_account_helper(std::string str, const char *hostname,
                           const char *port_num) {
  GENERATEXML generatexml(hostname, port_num);
  std::vector<char> s(str.begin(), str.end());
  generatexml.Send(s);
  std::vector<char> test = generatexml.recvServeResponse();
}

void client_create_account(const char *hostname, const char *port_num) {
  std::string str = "155\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<create>\n"
                    "<account id=\"2\" balance=\"10000\"/>\n"
                    "<symbol sym=\"BIT\">\n"
                    "<account id=\"2\">1000000</account>\n"
                    "</symbol>\n"
                    "</create>\n";
  create_account_helper(str, hostname, port_num);
  str = "155\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<create>\n"
        "<account id=\"1\" balance=\"10000\"/>\n"
        "<symbol sym=\"BIT\">\n"
        "<account id=\"2\">1000000</account>\n"
        "</symbol>\n"
        "</create>\n";
  create_account_helper(str, hostname, port_num);
}
int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Syntax: client <hostname>\n" << std::endl;
    return 1;
  }
  std::ofstream myfile;
  myfile.open("output.txt", std::ios::out);
  int i = THREAD_NUM;

  std::vector<std::thread> pool;

  client_create_account(argv[1], argv[2]);
  auto start = high_resolution_clock::now();

  for (int j = 0; j < i; j++) {
    std::thread t =
        std::thread(client_func, argv[1], argv[2], std::ref(myfile));
    pool.push_back(std::move(t));
  }

  for (std::thread &t : pool) {
    t.join();
  }

  /*
  client_create_account(argv[1],argv[2]);
  auto start = high_resolution_clock::now();


  while(i!=0){
    std::thread t = std::thread(client_func,argv[1],argv[2], std::ref(myfile));
    t.join();
    --i;
  }
  */
  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(stop - start);
  myfile
      //<< "Total function: "
      << duration.count() << std::endl;
  //<< " microseconds" << std::endl;
  myfile.close();
  return EXIT_SUCCESS;
}
