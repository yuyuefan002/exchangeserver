#include "dbinterface.h"
#include <iostream>
long int unix_timestamp() {
  time_t t = std::time(0);
  long int now = static_cast<long int>(t);
  return now;
}
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
    throw std::string("Database error");
    return -1;
  }
  return 0;
}
/*
 * execute and return
 *
 * execute sql and return required value in one transaction
 *
 *
 */
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
    throw std::string("Database error");
    return -1;
  }
}
std::string
genSQLKeyPair(std::vector<std::pair<std::string, std::string>> pairs) {
  std::string res;
  for (auto pair : pairs) {
    res += pair.first + "=" + pair.second + " AND ";
  }
  auto pos = res.find_last_of("AND");
  res.erase(res.begin() + pos - 2, res.end());
  return res;
}

/*
 * is_exist
 *
 */
bool is_exist(pqxx::connection *C, const std::string &sql) {
  pqxx::nontransaction N(*C);
  pqxx::result R(N.exec(sql));
  auto c = R.begin();
  return c[0].as<bool>();
}

/*
 * account_is_exist
 * Look up DB, return true if given account_id exist, else false;
 *
 */
bool order_is_valid(pqxx::connection *C, const std::string &account_id,
                    const std::string &order_id) {
  std::string sql =
      "SELECT EXISTS (SELECT 1 FROM ORDERS WHERE STATUS='op' AND ORDER_ID=" +
      order_id + "AND ACCOUNT_ID=" + account_id + ");";
  return is_exist(C, sql);
}
/*
 * account_is_exist
 * Look up DB, return true if given account_id exist, else false;
 *
 */
bool account_is_exist(pqxx::connection *C, const std::string &account_id) {
  std::string sql =
      "SELECT EXISTS (SELECT 1 FROM ACCOUNT WHERE ACCOUNT_ID=" + account_id +
      ");";
  return is_exist(C, sql);
}

bool order_is_sell(pqxx::connection *C, const std::string &order_id) {
  std::string sql = "SELECT SELL FROM ORDER WHERE ORDER_ID=" + order_id + ");";
  return is_exist(C, sql);
}

/*
 * has_the_position
 * Look up DB, return true if given position exist, else false;
 *
 */
bool has_the_position(pqxx::connection *C, const std::string &account_id,
                      const std::string &symbol) {
  std::string sql =
      "SELECT EXISTS (SELECT 1 FROM POSITION WHERE OWNER_ID=" + account_id +
      " AND SYM='" + symbol + "');";
  return is_exist(C, sql);
}

/*
 * create_account
 *
 * return 0 if success, else -1
 */
int DBINTERFACE::create_account(const std::string &account_id,
                                const std::string &balance) {
  try {
    if (account_is_exist(C.get(), account_id))
      return -1;
    std::string sql = "INSERT INTO ACCOUNT(ACCOUNT_ID, BALANCE) VALUES(" +
                      account_id + "," + balance + ");";
    return execute(sql);
  } catch (std::string e) {
    errmsg = e;
    return -1;
  }
}

/*
 * amount_is_enough
 *
 * check whether seller has enough shares
 *
 */
bool amount_is_enough(pqxx::connection *C, const std::string &account_id,
                      const std::string &symbol, const std::string &number) {
  std::string sql = "SELECT SHARE FROM POSITION WHERE OWNER_ID=" + account_id +
                    " AND SYM='" + symbol + "';";
  pqxx::nontransaction N(*C);
  pqxx::result R(N.exec(sql));
  if (R.empty())
    return false;
  auto c = R.begin();
  int share = c["SHARE"].as<int>();
  return share >= stoi(number);
}

/*
 * balance_is_enough
 *
 * check whether buyer has enough money
 *
 */
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
 * This function will first check if there is enough share
 * to sell or balance to buy
 * then create the order return the order id if success, else -1
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
        sell
            ? "UPDATE POSITION SET SHARE=SHARE-" + number +
                  " WHERE OWNER_ID=" + account_id + " AND SYM='" + symbol + "';"
            : "UPDATE ACCOUNT SET BALANCE=BALANCE-" + price + "*" + number +
                  " WHERE ACCOUNT_ID=" + account_id + ";";
    sql +=
        "INSERT INTO ORDERS(ACCOUNT_ID, SYM,SELL,PRICE,TOTAL,REST,TM) VALUES(" +
        account_id + ",'" + symbol + "',";
    sql += sell ? "TRUE" : "FALSE";
    sql += "," + price + "," + number + "," + number + "," +
           std::to_string(unix_timestamp()) + ") RETURNING ORDER_ID;";

    int order_id = execute_and_return(sql);
    return order_id;
  } catch (std::string s) {
    errmsg = s;
    return -1;
  }
}
int DBINTERFACE::add_to_position(const std::string &account_id,
                                 const std::string &symbol,
                                 const std::string &amount) {
  try {
    std::string sql = "UPDATE POSITION SET SHARE=SHARE+" + amount +
                      " WHERE OWNER_ID=" + account_id + " AND SYM='" + symbol +
                      "';";
    return execute(sql);
  } catch (std::string e) {
    errmsg = e;
    return -1;
  }
}
std::string DBINTERFACE::create_position_sql(const std::string &account_id,
                                             const std::string &symbol,
                                             const std::string &amount) {
  // check if account exist
  if (!account_is_exist(C.get(), account_id))
    throw std::string("account not exist");
  std::string sql;
  // check if there is the position
  if (has_the_position(C.get(), account_id, symbol)) {
    // add amount to existing symbol;
    sql = "UPDATE POSITION SET SHARE=SHARE+" + amount +
          " WHERE OWNER_ID=" + account_id + " AND SYM='" + symbol + "';";
  } else {
    // create new symbol with amount
    sql = "INSERT INTO POSITION(OWNER_ID,SYM,SHARE) VALUES(" + account_id +
          ",'" + symbol + "'," + amount + ");";
  }
  return sql;
}
/*
 * create postion
 *
 * update share info for a given account, return 0 if success, else -1
 */
int DBINTERFACE::create_position(const std::string &account_id,
                                 const std::string &symbol,
                                 const std::string &amount) {
  try {
    std::string sql = create_position_sql(account_id, symbol, amount);
    return execute(sql);
  } catch (std::string s) {
    errmsg = s;
    return -1;
  }
}
/*
 * execute_order
 *
 * warning: this function will not check the correctness of given nums
 *
 */
int DBINTERFACE::execute_order(const std::string &seller_id,
                               const std::string &buyer_id,
                               const std::string &symbol,
                               const std::string &final_price,
                               const std::string &final_amount,
                               const std::string &sell_oid,
                               const std::string &buy_oid) {
  try {
    std::string sql = "UPDATE ACCOUNT SET BALANCE=BALANCE+" + final_price +
                      "*" + final_amount + " WHERE ACCOUNT_ID=" + seller_id +
                      ";";
    sql += create_position_sql(buyer_id, symbol, final_amount);
    sql += "UPDATE ORDERS SET REST=REST-" + final_amount +
           " WHERE ORDER_ID=" + buy_oid + ";";
    sql += "UPDATE ORDERS SET REST=REST-" + final_amount +
           " WHERE ORDER_ID=" + sell_oid + ";";
    sql += "INSERT INTO EXECUTE(ORDER_ID,SHARE,PRICE,TIME) VALUES(" + sell_oid +
           "," + final_amount + "," + final_price + "," +
           std::to_string(unix_timestamp()) + ");";
    sql += "INSERT INTO EXECUTE(ORDER_ID,SHARE,PRICE,TIME) VALUES(" + buy_oid +
           "," + final_amount + "," + final_price + "," +
           std::to_string(unix_timestamp()) + ");";
    sql += "UPDATE ORDERS SET STATUS='cl', TM=" +
           std::to_string(unix_timestamp()) +
           " WHERE REST=0 AND ORDER_ID=" + sell_oid + ";";
    sql += "UPDATE ORDERS SET STATUS='cl', TM=" +
           std::to_string(unix_timestamp()) +
           " WHERE REST=0 AND ORDER_ID=" + buy_oid + ";";
    return execute(sql);
  } catch (std::string e) {
    errmsg = e;
    return -1;
  }
}

/*
 * update order status
 *
 * set status to cc-cancel or cl-close
 */
int DBINTERFACE::update_order_status(const std::string &order_id,
                                     const std::string &status) {
  try {
    std::string sql = "UPDATE ORDERS SET STATUS='" + status +
                      "', tm=" + std::to_string(unix_timestamp()) +
                      " WHERE ORDER_ID=" + order_id + ";";
    return execute(sql);
  } catch (std::string e) {
    errmsg = e;
    return -1;
  }
}

/*
 * look_up_order
 *
 * return order info
 *
 */
order_info_t look_up_order(pqxx::connection *C, const std::string &order_id) {
  std::string sql = "SELECT *FROM ORDERS WHERE ORDER_ID=" + order_id + ";";
  pqxx::nontransaction N(*C);
  pqxx::result R(N.exec(sql));
  order_info_t order;
  if (R.empty()) {
    order.order_id = -1;
    return order;
  }
  auto c = R.begin();

  order.symbol = c["SYM"].as<std::string>();
  order.price = c["PRICE"].as<double>();
  order.sell = c["SELL"].as<bool>();
  order.rest = c["REST"].as<double>();
  order.status = c["STATUS"].as<std::string>();
  order.time = c["TM"].as<int>();
  order.total = c["TOTAL"].as<double>();
  return order;
}

/*
 * cancel a order
 *
 * return share back to seller
 * return money back to buyer
 */
int DBINTERFACE::cancel_order(const std::string &order_id,
                              const std::string &account_id) {
  try {
    if (!order_is_valid(C.get(), account_id, order_id))
      return -1;
    int status = update_order_status(order_id, "cc");
    if (status == -1)
      return -1;
    order_info_t order = look_up_order(C.get(), order_id);
    if (order.sell == true) {
      return add_to_position(account_id, order.symbol,
                             std::to_string(order.rest));
    }
    // return balance to account
    std::string sql = "UPDATE ACCOUNT SET BALANCE=BALANCE+" +
                      std::to_string(order.price * order.rest) +
                      "WHERE ACCOUNT_ID=" + account_id + ";";
    return execute(sql);
  } catch (std::string e) {
    errmsg = e;
    return -1;
  }
}

order_info_t DBINTERFACE::query_order_status(const std::string &order_id) {
  return look_up_order(C.get(), order_id);
}

/*
 * query_order_execution
 *
 * return all the executions this order have
 *
 */
std::vector<order_info_t>
DBINTERFACE::query_order_execution(const std::string &order_id) {
  std::string sql =
      "SELECT SHARE,PRICE,TIME FROM EXECUTE WHERE ORDER_ID=" + order_id + ";";
  pqxx::nontransaction N(*C);
  pqxx::result R(N.exec(sql));
  std::vector<order_info_t> executes;
  for (auto c : R) {
    order_info_t order;
    order.price = c["PRICE"].as<double>();
    order.rest = c["SHARE"].as<double>();
    order.time = c["TIME"].as<int>();
    executes.push_back(order);
  }
  return executes;
}
order_info_t DBINTERFACE::match(const std::string &price,
                                const std::string &symbol, const bool &sell) {
  std::string sql;
  if (sell)
    sql = "SELECT * FROM ORDERS WHERE STATUS='op' AND SELL=false AND SYM='" +
          symbol + "'AND PRICE>=" + price +
          " AND TM<=" + std::to_string(unix_timestamp()) +
          "ORDER BY PRICE DESC LIMIT 1;";
  else
    sql = "SELECT * FROM ORDERS WHERE STATUS='op' AND SELL=true AND SYM='" +
          symbol + "'AND PRICE<=" + price +
          "AND TM<=" + std::to_string(unix_timestamp()) +
          "ORDER BY PRICE ASC LIMIT 1;";
  pqxx::nontransaction N(*C);
  pqxx::result R(N.exec(sql));
  order_info_t order;
  if (R.empty()) {
    order.order_id = -1;
    return order;
  }

  auto c = R.begin();

  order.account_id = c["ACCOUNT_ID"].as<int>();
  order.order_id = c["ORDER_ID"].as<int>();
  order.price = c["PRICE"].as<double>();
  order.rest = c["REST"].as<double>();
  return order;
}
int DBINTERFACE::initializer() {
  pqxx::work W(*C);
  try {
    // drop table if exists
    W.exec("DROP TABLE IF EXISTS EXECUTE;DROP TABLE IF EXISTS ORDERS;DROP "
           "TABLE IF EXISTS POSITION;DROP TABLE IF EXISTS ACCOUNT;");

    // create table ACCOUNT
    std::string sql = "CREATE TABLE ACCOUNT(ACCOUNT_ID INT PRIMARY KEY NOT "
                      "NULL,BALANCE DOUBLE PRECISION NOT NULL);";
    W.exec(sql);

    // create table POSITION
    sql = "CREATE TABLE POSITION("
          "POSITION_ID SERIAL PRIMARY KEY NOT NULL,"
          "OWNER_ID INT REFERENCES ACCOUNT(ACCOUNT_ID),"
          "SYM VARCHAR(20) NOT NULL,"
          "SHARE DOUBLE PRECISION NOT NULL"
          ");";
    W.exec(sql);

    // create table ORDERS
    sql = "CREATE TABLE ORDERS("
          "ORDER_ID SERIAL PRIMARY KEY NOT NULL,"
          "ACCOUNT_ID INT REFERENCES ACCOUNT(ACCOUNT_ID),"
          "SYM VARCHAR(20) NOT NULL,"
          "STATUS char(2) NOT NULL DEFAULT 'op',"
          "PRICE DOUBLE PRECISION NOT NULL,"
          "SELL BOOLEAN NOT NULL,"
          "TOTAL DOUBLE PRECISION NOT NULL,"
          "REST DOUBLE PRECISION NOT NULL,"
          "TM INT"
          ");";
    W.exec(sql);

    // create table EXECUTE
    sql = "CREATE TABLE EXECUTE("
          "EXECUTE_ID SERIAL PRIMARY KEY NOT NULL,"
          "ORDER_ID INT REFERENCES ORDERS(ORDER_ID),"
          "SHARE DOUBLE PRECISION NOT NULL,"
          "PRICE DOUBLE PRECISION NOT NULL,"
          "TIME INT NOT NULL"
          ");";
    W.exec(sql);
    W.commit();
  } catch (const std::exception &e) {
    W.abort();
    return -1;
  }
  return 0;
}
/*
#include <iostream>
int main() {
  DBINTERFACE DBInterface;
  std::cout << DBInterface.create_account("2", "10000") << std::endl;
  std::cout << DBInterface.create_order("2", "BIT", "100", "50", true)
            << std::endl;
  //  << std::endl;
  std::vector<std::pair<std::string, std::string>> test;
  // std::cout << DBInterface.create_position("1", "BIT", "100") << std::endl;
  // std::cout << DBInterface.cancel_order("1", "1");
  // test.push_back({"OWNER_ID", "1"});
  // test.push_back({"SYM", "BIT"});
  // std::cout << genSQLKeyPair(test) << std::endl;
  // std::cout << DBInterface.execute_order("1", "2", "BIT", "100", "100", "5",
  //                                       "6");
  order_info_t order = DBInterface.match(
      "100", "BIT",
      false); std::vector<order_info_t>
                orders = DBInterface.query_order_execution("5");
std::cout << order.order_id << " " << order.price << std::endl;

for (auto order : orders) { std::cout <<
       order.rest
    << " " << order.price << " " << order.time << std::endl;
}

}
*/
