#ifndef __DBINTERFACE_H__
#define __DBINTERFACE_H__
#include <pqxx/pqxx>
class DBINTERFACE {
private:
  std::unique_ptr<pqxx::connection> C;
  int execute(const std::string &sql);
  int execute_and_return(const std::string &sql);

public:
  DBINTERFACE();
  ~DBINTERFACE();
  int create_account(const std::string &account_id, const std::string &balance);
  int create_order(const std::string &account_id, const std::string &symbol,
                   const std::string &number, const std::string &price,
                   const bool &sell);
};
#endif
