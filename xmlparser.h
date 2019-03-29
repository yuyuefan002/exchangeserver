#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <vector>
class XMLPARSER {
public:
  void parse(std::vector<char> xml);
};
#endif
