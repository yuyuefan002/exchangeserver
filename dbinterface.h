#ifndef __DBINTERFACE_H__
#define __DBINTERFACE_H__
#include <ctime>
#include <pqxx/pqxx>
class DBINTERFACE {
private:
  std::unique_ptr<pqxx::connection> C;
  int execute(const std::string &sql);
  int execute_and_return(const std::string &sql);
  int add_to_position(const std::string &account_id, const std::string &symbol,
                      const std::string &amount);
  std::string create_position_sql(const std::string &account_id,
                                  const std::string &symbol,
                                  const std::string &amount);
  int update_order_status(const std::string &order_id,
                          const std::string &status);

public:
  DBINTERFACE();
  ~DBINTERFACE();
  int create_account(const std::string &account_id, const std::string &balance);
  int create_order(const std::string &account_id, const std::string &symbol,
                   const std::string &number, const std::string &price,
                   const bool &sell);
  int create_position(const std::string &account_id, const std::string &symbol,
                      const std::string &amount);
  int execute_order(const std::string &seller_id, const std::string &buyer_id,
                    const std::string &symbol, const std::string &final_price,
                    const std::string &final_amount,
                    const std::string &sell_oid, const std::string &buy_oid);
  int cancel_order(const std::string &order_id, const std::string &account_id);
};

struct _order_info_t {
  std::string symbol;
  int price;
  bool sell;
  double rest;
};

typedef struct _order_info_t order_info_t;
#endif
