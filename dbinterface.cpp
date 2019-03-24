#include "dbinterface.h"

DBINTERFACE::DBINTERFACE() {
  // Allocate & initialize a Postgres connection object

  // Establish a connection to the database
  // Parameters: database name, user name, user password
  C = std::unique_ptr<pqxx::connection>(
      new pqxx::connection("dbname=EXCHANGE user=postgres password=passw0rd"));
  if (C->is_open()) {
    //  cout << "Opened database successfully: " << C->dbname() << endl;
  } else {
    throw std::string("fail to open db");
  }
}
DBINTERFACE::~DBINTERFACE() { C->disconnect(); }
/*
 * execute sql
 *
 * return 0 if success, else -1
 */
int DBINTERFACE::execute(const std::string &sql) {
  pqxx::work W(*C);
  try {
    W.exec(sql);
    W.commit();
  } catch (const std::exception &e) {
    W.abort();
    return -1;
  }
  return 0;
}
int DBINTERFACE::execute_and_return(const std::string &sql) {
  pqxx::work W(*C);
  try {
    auto v = W.exec(sql);
    W.commit();
    pqxx::result R(v);
    auto c = R.begin();
    return c["ORDER_ID"].as<int>();
  } catch (const std::exception &e) {
    W.abort();
    return -1;
  }
}

bool account_is_exist(pqxx::connection *C, const std::string &account_id) {
  std::string sql =
      "SELECT * FROM ACCOUNT WHERE ACCOUNT_ID=" + account_id + ";";
  pqxx::nontransaction N(*C);
  pqxx::result R(N.exec(sql));
  if (R.empty())
    return false;
  return true;
}

/*
 * create_account
 *
 * return 0 if success, else -1
 */
int DBINTERFACE::create_account(const std::string &account_id,
                                const std::string &balance) {
  if (account_is_exist(C.get(), account_id))
    return -1;
  std::string sql = "INSERT INTO ACCOUNT(ACCOUNT_ID, BALANCE) VALUES(" +
                    account_id + "," + balance + ");";
  return execute(sql);
}
bool amount_is_enough(pqxx::connection *C, const std::string &account_id,
                      const std::string &symbol, const std::string &number) {
  std::string sql = "SELECT SHARE FROM POSITION WHERE OWNER_ID=" + account_id +
                    " AND SYM=" + symbol + ";";
  pqxx::nontransaction N(*C);
  pqxx::result R(N.exec(sql));
  if (R.empty())
    return false;
  auto c = R.begin();
  int share = c["SHARE"].as<int>();
  return share >= stoi(number);
}

bool balance_is_enough(pqxx::connection *C, const std::string &account_id,
                       const std::string &number, const std::string &price) {
  std::string sql =
      "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID=" + account_id + ";";
  pqxx::nontransaction N(*C);
  pqxx::result R(N.exec(sql));
  if (R.empty())
    return false;
  auto c = R.begin();
  int balance = c["BALANCE"].as<int>();
  return balance >= stoi(number) * stoi(price);
}
/*
 * create_order
 *
 * This function will first check if there is enough share to sell or balance to
 * buy then create the order return the order id if success, else -1
 */
int DBINTERFACE::create_order(const std::string &account_id,
                              const std::string &symbol,
                              const std::string &number,
                              const std::string &price, const bool &sell) {
  try {
    if (sell) {
      // check if amount is enough
      if (!amount_is_enough(C.get(), account_id, symbol, number))
        throw std::string("amount is not enough");
    } else {
      // check if balance is enough
      if (!balance_is_enough(C.get(), account_id, number, price))
        throw std::string("balance is not enough");
    }
    std::string sql =
        "INSERT INTO ORDERS(ACCOUNT_ID, SYM,SELL,PRICE,TOTAL,REST) VALUES(" +
        account_id + ",'" + symbol + "',";
    if (sell)
      sql += "TRUE";
    else
      sql += "FALSE";
    sql += "," + price + "," + number + "," + number + ") RETURNING ORDER_ID;";

    int order_id = execute_and_return(sql);
    return order_id;
  } catch (std::string s) {
    return -1;
  }
}
#include <iostream>
int main() {
  DBINTERFACE DBInterface;
  std::cout << DBInterface.create_account("1", "10000") << std::endl;
  std::cout << DBInterface.create_order("1", "BIT", "100", "100", false)
            << std::endl;
}
