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
  void create_order(const std::string &account_id,
                    std::unordered_map<std::string, std::string> attrs);
  void create_symbol(rapidxml::xml_node<> *root, std::string &symbol,
                     rapidxml::xml_node<> *resultroot);
  void create_handler(rapidxml::xml_node<> *root,
                      rapidxml::xml_node<> *resultroot);
  void transaction_handler(rapidxml::xml_node<> *root,
                           const std::string &account_id);

  int match(const std::string &account_id, const std::string &symbol,
            const std::string &amount, const std::string &price,
            const bool &sell);

public:
  void xml_handler(std::vector<char> &xml);
};

#endif
