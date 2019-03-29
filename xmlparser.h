#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <sstream>
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
  XMLPARSER() {}
  rapidxml::xml_node<> *getNode();
  std::unordered_map<std::string, std::string>
  getAttrs(rapidxml::xml_node<> *curr);
  void append_node(rapidxml::xml_node<> *root, std::string tag,
                   std::unordered_map<std::string, std::string> attrs,
                   std::string msg = "");
  void visit(rapidxml::xml_node<> *node);
};
#endif
