#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <string>
#include <unordered_map>
#include <vector>
class XmlNode {
public:
  std::string name;
  std::string value;
  std::vector<std::pair<std::string, std::string>> attr;
  XmlNode *nextSibling;
  XmlNode *child;
  XmlNode(std::string n) : name(n) {}
};

class XMLPARSER {
private:
  rapidxml::xml_node<> *node;

public:
  XMLPARSER(std::vector<char> &xml);
  XMLPARSER(){}
  rapidxml::xml_node<> *getNode();
  std::unordered_map<std::string, std::string>
  getAttrs(rapidxml::xml_node<> *curr);
};
#endif
