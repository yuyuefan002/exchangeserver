#include "exchangeserver.h"

int EXCHANGESERVER::transaction(const std::string &account_id,
                                const std::string &symbol,
                                const std::string &amount,
                                const std::string &price, const bool &sell) {
  double amount_n = stod(amount);
  DBINTERFACE DBInterface;
  int id = DBInterface.create_order(account_id, symbol, amount, price, sell);
  if (id == -1)
    return -1;
  while (amount_n) {
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

    DBInterface.execute_order(seller_id, buyer_id, symbol,
                              std::to_string(order.price), final_amount,
                              sell_oid, buy_oid);
    amount_n -= stod(final_amount);
  }
  return 0;
}

int main() {
  EXCHANGESERVER exchangeserver;
  exchangeserver.transaction("2", "BIT", "100", "100", false);
}
