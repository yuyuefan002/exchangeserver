#ifndef __EXCHANGESERVER_H__
#define __EXCHANGESERVER_H__
#include "dbinterface.h"
#include "rapidxml_print.hpp"
#include "xmlparser.h"
#include <sstream>
#include <string>
class EXCHANGESERVER {
private:
  DBINTERFACE DBInterface;
  rapidxml::xml_document<> doc;
  void create_order(
      const std::string &account_id,
      std::unordered_map<std::string, std::pair<const char *, const char *>>
          attrs,
      rapidxml::xml_node<> *resultroot, std::vector<std::string> &tags,
      std::vector<std::string> &failinfo);
  void create_symbol(rapidxml::xml_node<> *root,
                     std::pair<const char *, const char *> symbol,
                     rapidxml::xml_node<> *resultroot,
                     std::vector<std::string> &tags,
                     std::vector<std::string> &failinfo);
  void create_handler(rapidxml::xml_node<> *root,
                      rapidxml::xml_node<> *resultroot,
                      std::vector<std::string> &tags,
                      std::vector<std::string> &failinfo);
  void transaction_handler(rapidxml::xml_node<> *root,
                           const std::string &account_id,
                           rapidxml::xml_node<> *resultroot,
                           std::vector<std::string> &tags,
                           std::vector<std::string> &failinfo);

  int match(const std::string &account_id, const std::string &symbol,
            const std::string &amount, const std::string &price,
            const bool &sell);

public:
  void xml_handler(std::vector<char> &xml);
};

#endif
