#include "exchangeserver.h"
#include <iostream>

void EXCHANGESERVER::errorTag(rapidxml::xml_node<> *resultroot,
                              std::string info) {
  XMLPARSER XMLParser;
  char *name = doc.allocate_string("error");
  char *msg = doc.allocate_string(info.c_str());
  XMLParser.append_node(doc, resultroot, name, {}, msg);
}

/*
 * appendAttrs
 *
 * append attributes into map
 */
void appendAttrs(
    rapidxml::xml_document<> &doc,
    std::unordered_map<std::string, std::pair<const char *, const char *>>
        &attrs,
    std::string name, std::string value) {
  char *nname = doc.allocate_string(name.c_str());
  char *nvalue = doc.allocate_string(value.c_str());
  attrs[name] = std::make_pair(nname, nvalue);
}

/*
 * match
 *
 * use the given order to match existing orders until no more can be matched
 *
 */
std::mutex mtx;
int EXCHANGESERVER::match(const std::string &account_id,
                          const std::string &symbol, const std::string &amount,
                          const std::string &price, const bool &sell) {
  double amount_n = stod(amount);
  int id = DBInterface.create_order(account_id, symbol, amount, price, sell);
  if (id == -1)
    return -1;
  // loop until no more can be matched
  while (1) {
    // look up one matching order
    std::lock_guard<std::mutex> lck(mtx);
    order_info_t newstatus = DBInterface.query_order_status(std::to_string(id));
    amount_n = newstatus.rest;
    if (amount_n <= 0)
      break;
    order_info_t order = DBInterface.match(price, symbol, sell);
    if (order.order_id == -1)
      break;
    std::string seller_id = account_id;
    std::string buyer_id = std::to_string(order.account_id);
    std::string sell_oid = std::to_string(id);
    std::string buy_oid = std::to_string(order.order_id);
    if (!sell) {
      std::swap(seller_id, buyer_id);
      std::swap(sell_oid, buy_oid);
    }
    std::string final_amount = amount_n > order.rest
                                   ? std::to_string(order.rest)
                                   : std::to_string(amount_n);
    // match two orders
    DBInterface.execute_order(seller_id, buyer_id, symbol,
                              std::to_string(order.price), final_amount,
                              sell_oid, buy_oid);
  }
  return id;
}
/*
 * are digits
 *
 * are all chars digits?
 */
bool are_digits(const std::string &str) {
  for (auto c : str) {
    if (!isdigit(c))
      return false;
  }
  return true;
}

/*
 * xml_handler
 *
 * main entrance, handle the given xml
 * throw exception
 */
void EXCHANGESERVER::xml_handler(std::vector<char> &xml) {
  XMLPARSER XMLParser(xml);
  rapidxml::xml_node<> *resultroot =
      doc.allocate_node(rapidxml::node_element, "results");
  doc.append_node(resultroot);
  rapidxml::xml_node<> *root = XMLParser.getNode();
  for (rapidxml::xml_node<> *curr = root; curr != nullptr;
       curr = curr->next_sibling()) {
    std::string name = curr->name();
    // create order or position
    if (name == "create") {
      create_handler(curr->first_node(), resultroot);
    }
    // query or order or cancel
    else if (name == "transactions") {
      std::unordered_map<std::string, std::pair<const char *, const char *>>
          attrs = XMLParser.getAttrs(curr);
      if (!are_digits(attrs["id"].second))
        errorTag(resultroot, "Invalid accound id");
      transaction_handler(curr->first_node(), attrs["id"].second, resultroot);
    }

    // other tags are invalid
    else {
      errorTag(resultroot, "Invalid tag");
    }
  }
}

/*
 * create_order
 *
 * place an order
 */
void EXCHANGESERVER::create_order(
    const std::string &account_id,
    std::unordered_map<std::string, std::pair<const char *, const char *>>
        attrs,
    rapidxml::xml_node<> *resultroot) {
  double amount_p = stod(std::string(attrs["amount"].second));
  // if sell a position, limit will be negative
  std::string amount =
      amount_p <= 0 ? std::to_string(-amount_p) : attrs["amount"].second;
  bool sell = amount_p <= 0 ? true : false;
  if (!are_digits(attrs["limit"].second))
    errorTag(resultroot, "Invalid limit");
  // match orders
  int status = match(account_id, attrs["sym"].second, amount,
                     attrs["limit"].second, sell);
  XMLPARSER XMLParser;
  if (status == -1) {
    char *name = doc.allocate_string("error");
    char *msg = doc.allocate_string(DBInterface.errmsg.c_str());
    XMLParser.append_node(doc, resultroot, name, attrs, msg);
  } else {
    char *name = doc.allocate_string("opened");
    appendAttrs(doc, attrs, "opened", std::to_string(status));
    XMLParser.append_node(doc, resultroot, name, attrs, nullptr);
  }
}

/*
 * query_order_first
 *
 * generate query info
 *
 */
void EXCHANGESERVER::query_order_first(
    rapidxml::xml_document<> &doc, rapidxml::xml_node<> *resultroot,
    std::string tagname, std::string id, bool checkopen,
    std::unordered_map<std::string, std::pair<const char *, const char *>>
        attrs) {
  XMLPARSER XMLParser;
  std::unordered_map<std::string, std::pair<const char *, const char *>>
      returnattrs;
  appendAttrs(doc, returnattrs, "id", id);
  order_info_t status = DBInterface.query_order_status(id);
  if (status.order_id == -1) {
    char *tag = doc.allocate_string("error");
    char *msg = doc.allocate_string("No order can be found");
    XMLParser.append_node(doc, resultroot, tag, returnattrs, msg);
    return;
  }
  char *tag = doc.allocate_string(tagname.c_str());

  rapidxml::xml_node<> *child =
      XMLParser.append_node(doc, resultroot, tag, returnattrs, nullptr);
  query_order_second(attrs, child, checkopen);
}

/*
 * query_order_second
 *
 * interact with database, get order info
 * add into node
 */
void EXCHANGESERVER::query_order_second(
    std::unordered_map<std::string, std::pair<const char *, const char *>>
        &attrs,
    rapidxml::xml_node<> *resultroot, bool checkopen) {
  XMLPARSER XMLParser;
  std::string id = attrs["id"].second;
  order_info_t status = DBInterface.query_order_status(id);
  if (status.order_id == -1) {
    char *tag = doc.allocate_string("error");
    char *msg = doc.allocate_string("No order can be found");
    XMLParser.append_node(doc, resultroot, tag, {}, msg);
    return;
  }
  std::vector<order_info_t> executes =
      DBInterface.query_order_execution(attrs["id"].second);
  char *tag = doc.allocate_string("open");
  std::unordered_map<std::string, std::pair<const char *, const char *>>
      returnattrs;
  // print open info
  if (checkopen) {
    appendAttrs(doc, returnattrs, "shares", std::to_string(status.total));
    XMLParser.append_node(doc, resultroot, tag, returnattrs, nullptr);
  }
  // print cancel info
  if (status.status == "cc") {
    tag = doc.allocate_string("cancelled");
    appendAttrs(doc, returnattrs, "shares", std::to_string(status.rest));
    appendAttrs(doc, returnattrs, "time", std::to_string(status.time));
    XMLParser.append_node(doc, resultroot, tag, returnattrs, nullptr);
  }
  tag = doc.allocate_string("executed");
  // print execution history
  for (auto e : executes) {
    appendAttrs(doc, returnattrs, "shares", std::to_string(e.rest));
    appendAttrs(doc, returnattrs, "price", std::to_string(e.price));
    appendAttrs(doc, returnattrs, "time", std::to_string(e.time));
    XMLParser.append_node(doc, resultroot, tag, returnattrs, nullptr);
  }
}

/*
 * transaction_handler
 *
 * handle all transactions, query or cancel or order
 *
 *
 */
void EXCHANGESERVER::transaction_handler(rapidxml::xml_node<> *root,
                                         const std::string &account_id,
                                         rapidxml::xml_node<> *resultroot) {
  for (rapidxml::xml_node<> *curr = root; curr; curr = curr->next_sibling()) {
    std::string name = curr->name();
    XMLPARSER XMLParser;
    std::unordered_map<std::string, std::pair<const char *, const char *>>
        attrs = XMLParser.getAttrs(curr);

    // place an order
    if (name == "order") {
      create_order(account_id, attrs, resultroot);
    }
    // query an order
    else if (name == "query") {
      if (!are_digits(attrs["id"].second))
        errorTag(resultroot, "Invalid ID");
      query_order_first(doc, resultroot, "status", attrs["id"].second, true,
                        attrs);
    }
    // cancel an order
    else if (name == "cancel") {
      if (!are_digits(attrs["id"].second))
        errorTag(resultroot, "Invalid id");
      int status = DBInterface.cancel_order(attrs["id"].second, account_id);
      if (status == -1) {
        char *errmsg = doc.allocate_string(DBInterface.errmsg.c_str());
        XMLParser.append_node(doc, resultroot, "error", attrs, errmsg);
      } else
        query_order_first(doc, resultroot, "canceled", attrs["id"].second,
                          false, attrs);
    }
    // invalid things
    else {
      errorTag(resultroot, "Invalid tag");
    }
  }
}

/*
 * create_handler
 *
 * create account or position
 *
 */
void EXCHANGESERVER::create_handler(rapidxml::xml_node<> *root,
                                    rapidxml::xml_node<> *resultroot) {
  for (rapidxml::xml_node<> *curr = root; curr; curr = curr->next_sibling()) {
    std::string name = curr->name();
    XMLPARSER XMLParser;
    std::unordered_map<std::string, std::pair<const char *, const char *>>
        attrs = XMLParser.getAttrs(curr);

    // create account
    if (name == "account") {

      if (!are_digits(attrs["id"].second) ||
          !are_digits(attrs["balance"].second))
        errorTag(resultroot, "Invalid id or balance");
      std::unordered_map<std::string, std::pair<const char *, const char *>>
          returnattrs;
      returnattrs["id"] = attrs["id"];
      if (DBInterface.create_account(attrs["id"].second,
                                     attrs["balance"].second) == -1) {
        char *name = doc.allocate_string("error");
        char *msg = doc.allocate_string(DBInterface.errmsg.c_str());
        XMLParser.append_node(doc, resultroot, name, returnattrs, msg);
      } else {
        char *name = doc.allocate_string("created");
        XMLParser.append_node(doc, resultroot, name, returnattrs, nullptr);
      }
    }
    // create postion
    else if (name == "symbol") {
      create_symbol(curr->first_node(), attrs["sym"], resultroot);
    }
    // invalid tags
    else {
      char *name = doc.allocate_string("error");
      char *msg = doc.allocate_string("Invalid tag");
      XMLParser.append_node(doc, resultroot, name, {}, msg);
    }
  }
}

/*
 * craete_symbol
 *
 * create positions
 */
void EXCHANGESERVER::create_symbol(rapidxml::xml_node<> *root,
                                   std::pair<const char *, const char *> symbol,
                                   rapidxml::xml_node<> *resultroot) {
  for (rapidxml::xml_node<> *curr = root; curr; curr = curr->next_sibling()) {
    std::string name = curr->name();
    std::string amount = curr->value();
    XMLPARSER XMLParser;
    std::unordered_map<std::string, std::pair<const char *, const char *>>
        attrs = XMLParser.getAttrs(curr);
    attrs["sym"] = symbol;
    if (name == "account") {
      if (!are_digits(attrs["id"].second))
        errorTag(resultroot, "Invalid account id");

      if (DBInterface.create_position(attrs["id"].second, symbol.second,
                                      amount) == -1) {
        char *name = doc.allocate_string("error");
        char *msg = doc.allocate_string(DBInterface.errmsg.c_str());
        XMLParser.append_node(doc, resultroot, name, attrs, msg);
      } else {
        char *name = doc.allocate_string("created");
        XMLParser.append_node(doc, resultroot, name, attrs, nullptr);
      }
    }

    else
      errorTag(resultroot, "Invalid tag");
  }
}

int EXCHANGESERVER::accNewRequest() {
  int newfd = server.acceptNewConn();
  return newfd;
}
std::vector<char> EXCHANGESERVER::genResponse() {
  std::stringstream ss;
  ss << doc;
  std::string str = ss.str();
  std::vector<char> response(str.begin(), str.end());
  return response;
}
void EXCHANGESERVER::handler(int newfd) {
  std::vector<char> xml = server.receiveData(newfd);
  xml_handler(xml);
  std::vector<char> response = genResponse();
  server.sendData(newfd, response);
}
EXCHANGESERVER::EXCHANGESERVER(const char *port) : server(port) {}
EXCHANGESERVER::~EXCHANGESERVER() {}
int EXCHANGESERVER::DBinitializer() { return DBInterface.initializer(); }
/*
int main() {
  EXCHANGESERVER exchangeserver;
  std::string str =
      "<?xml version=\"1.0\" "
      "encoding=\"UTF-8\"?>\n<create>\n<account "
      "id=\"123456\" balance=\"1000\"/>\n<symbol sym=\"SPY\">\n<account "
      "id=\"123456\">100000</account>\n</symbol>\n</create>\n";
    std::string str = "<?xml version=\"1.0\" "
                    "encoding=\"UTF-8\"?>\n"
                    "<transactions id=\"2\">\n<order sym=\"BIT\" "
                    "amount=\"100\" limit=\"100\"/>\n<query "
                    "id=\"1\"/>\n<cancel id=\"12\"/>\n</transactions>\n";
std::vector<char> s(str.begin(), str.end());
s.push_back('\0');
exchangeserver.xml_handler(s);
}
*/
