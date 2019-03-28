#ifndef __EXCHANGESERVER_H__
#define __EXCHANGESERVER_H__
#include "dbinterface.h"
#include <string>
class EXCHANGESERVER {
public:
  int transaction(const std::string &account_id, const std::string &symbol,
                  const std::string &amount, const std::string &price,
                  const bool &sell);
};

#endif
