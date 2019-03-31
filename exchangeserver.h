#ifndef __EXCHANGESERVER_H__
#define __EXCHANGESERVER_H__
#include "dbinterface.h"
#include "rapidxml_print.hpp"
#include "server.h"
#include "xmlparser.h"
#include <sstream>
#include <string>
class EXCHANGESERVER {
private:
  Server server;
  DBINTERFACE DBInterface;
  rapidxml::xml_document<> doc;
  void errorTag(rapidxml::xml_node<> *resultroot, std::string info);
  void create_order(
      const std::string &account_id,
      std::unordered_map<std::string, std::pair<const char *, const char *>>
          attrs,
      rapidxml::xml_node<> *resultroot);
  void create_symbol(rapidxml::xml_node<> *root,
                     std::pair<const char *, const char *> symbol,
                     rapidxml::xml_node<> *resultroot);
  void create_handler(rapidxml::xml_node<> *root,
                      rapidxml::xml_node<> *resultroot);
  void transaction_handler(rapidxml::xml_node<> *root,
                           const std::string &account_id,
                           rapidxml::xml_node<> *resultroot);

  int match(const std::string &account_id, const std::string &symbol,
            const std::string &amount, const std::string &price,
            const bool &sell);
  void query_order_second(
      std::unordered_map<std::string, std::pair<const char *, const char *>>
          &attrs,
      rapidxml::xml_node<> *resultroot, bool checkopen);
  void query_order_first(
      rapidxml::xml_document<> &doc, rapidxml::xml_node<> *resultroot,
      std::string tagname, std::string id, bool checkopen,
      std::unordered_map<std::string, std::pair<const char *, const char *>>
          attrs);
  std::vector<char> genResponse();
  void xml_handler(std::vector<char> &xml);

public:
  EXCHANGESERVER(const char *port);
  EXCHANGESERVER() {}
  int DBinitializer();
  ~EXCHANGESERVER();
  int accNewRequest();
  void handler(int newfd);
};

#endif
