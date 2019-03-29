#include "exchangeserver.h"
#include <iostream>
/*
 * match
 *
 * use the given order to match existing orders until no more can be matched
 *
 */
int EXCHANGESERVER::match(const std::string &account_id,
                          const std::string &symbol, const std::string &amount,
                          const std::string &price, const bool &sell) {
  double amount_n = stod(amount);
  int id = DBInterface.create_order(account_id, symbol, amount, price, sell);
  if (id == -1)
    return -1;
  // loop until no more can be matched
  while (amount_n) {
    // look up one matching order
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
    amount_n -= stod(final_amount);
  }
  return 0;
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
  for (rapidxml::xml_node<> *curr = root; curr; curr = curr->next_sibling()) {
    std::string name = curr->name();
    // create order or position
    if (name == "create") {
      create_handler(curr->first_node(), resultroot);
    }
    // query or order or cancel
    else if (name == "transactions") {
      std::unordered_map<std::string, std::string> attrs =
          XMLParser.getAttrs(curr);
      if (!are_digits(attrs["id"]))
        throw std::string("invalid account_id");
      transaction_handler(curr->first_node(), attrs["id"]);
    }

    // other tags are invalid
    else {
      throw std::string("invalid tag");
    }
  }
  std::cout << doc;
  XMLParser.visit(doc.first_node());
}

/*
 * create_order
 *
 * place an order
 */
void EXCHANGESERVER::create_order(
    const std::string &account_id,
    std::unordered_map<std::string, std::string> attrs) {
  std::string price;
  bool sell;
  double number_p = stod(attrs["limit"]);
  // if sell a position, limit will be negative
  if (number_p <= 0) {
    price = std::to_string(-number_p);
    sell = true;
  } else {
    price = attrs["limit"];
    sell = false;
  }
  if (!are_digits(attrs["amount"]))
    throw std::string("invalid amount");
  // match orders
  match(account_id, attrs["sym"], attrs["amount"], price, sell);
}

/*
 * transaction_handler
 *
 * handle all transactions, query or cancel or order
 *
 *
 */
void EXCHANGESERVER::transaction_handler(rapidxml::xml_node<> *root,
                                         const std::string &account_id) {
  for (rapidxml::xml_node<> *curr = root; curr; curr = curr->next_sibling()) {
    std::string name = curr->name();
    XMLPARSER XMLParser;
    std::unordered_map<std::string, std::string> attrs =
        XMLParser.getAttrs(curr);

    // place an order
    if (name == "order") {
      create_order(account_id, attrs);
    }
    // query an order
    else if (name == "query") {
      if (!are_digits(attrs["id"]))
        throw std::string("invalid id");
      order_info_t status = DBInterface.query_order_status(attrs["id"]);
      std::vector<order_info_t> executes =
          DBInterface.query_order_execution(attrs["id"]);
    }
    // cancel an order
    else if (name == "cancel") {
      if (!are_digits(attrs["id"]))
        throw std::string("invalid id");
      DBInterface.cancel_order(attrs["id"], account_id);
    }
    // invalid things
    else {
      throw std::string("invalid tag");
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
    std::unordered_map<std::string, std::string> attrs =
        XMLParser.getAttrs(curr);

    // create account
    if (name == "account") {

      if (!are_digits(attrs["id"]) || !are_digits(attrs["balance"]))
        throw std::string("invalid id or balance");
      std::unordered_map<std::string, std::string> returnattrs;
      returnattrs["id"] = attrs["id"];
      if (DBInterface.create_account(attrs["id"], attrs["balance"]) == -1) {
        XMLParser.append_node(resultroot, "error", returnattrs,
                              "Failed to create account");
      } else {
        XMLParser.append_node(resultroot, "created", returnattrs);
      }
    }
    // create postion
    else if (name == "symbol")
      create_symbol(curr->first_node(), attrs["sym"], resultroot);
    // invalid tags
    else
      throw std::string("invalid tag");
  }
}

/*
 * craete_symbol
 *
 * create positions
 */
void EXCHANGESERVER::create_symbol(rapidxml::xml_node<> *root,
                                   std::string &symbol,
                                   rapidxml::xml_node<> *resultroot) {
  for (rapidxml::xml_node<> *curr = root; curr; curr = curr->next_sibling()) {
    std::string name = curr->name();
    std::string amount = curr->value();
    XMLPARSER XMLParser;
    std::unordered_map<std::string, std::string> attrs =
        XMLParser.getAttrs(curr);
    attrs["sym"] = symbol;
    if (name == "account") {
      if (!are_digits(attrs["id"]))
        throw std::string("invalid account_id");
      if (DBInterface.create_position(attrs["id"], symbol, amount) == -1) {
        XMLParser.append_node(resultroot, "error", attrs,
                              "fail to create position");
      } else
        XMLParser.append_node(resultroot, "created", attrs);
    }

    else
      throw std::string("invalid tag");
  }
}

int main() {
  EXCHANGESERVER exchangeserver;
  std::string str =
      "<?xml version=\"1.0\" "
      "encoding=\"UTF-8\"?>\n<create>\n<account "
      "id=\"123456\" balance=\"1000\"/>\n<symbol sym=\"SPY\">\n<account "
      "id=\"123456\">100000</account>\n</symbol>\n</create>\n";
  /*  std::string str = "<?xml version=\"1.0\" "
                    "encoding=\"UTF-8\"?>\n"
                    "<transactions id=\"2\">\n<order sym=\"BIT\" "
                    "amount=\"100\" limit=\"-100\"/>\n<query "
                    "id=\"1\"/>\n<cancel id=\"12\"/>\n</transactions>\n";*/
  std::vector<char> s(str.begin(), str.end());
  exchangeserver.xml_handler(s);
}
