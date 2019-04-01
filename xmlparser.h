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
  std::unordered_map<std::string, std::pair<const char *, const char *>>
  getAttrs(rapidxml::xml_node<> *curr);
  rapidxml::xml_node<> *append_node(
      rapidxml::xml_document<> &doc, rapidxml::xml_node<> *root,
      const char *tag,
      std::unordered_map<std::string, std::pair<const char *, const char *>>
          attrs,
      const char *msg);
  void visit(rapidxml::xml_node<> *node);
};
#endif
