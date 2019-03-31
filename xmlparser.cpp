#include "xmlparser.h"
#include <iostream>

void XMLPARSER::append_node(
    rapidxml::xml_node<> *root, std::string &tag,
    std::unordered_map<std::string, std::pair<const char *, const char *>>
        attrs,
    std::string &msg) {
  rapidxml::xml_document<> doc;
  rapidxml::xml_node<> *child =
      doc.allocate_node(rapidxml::node_element, tag.c_str());
  for (auto attr : attrs) {
    child->append_attribute(
        doc.allocate_attribute(attr.second.first, attr.second.second));
  }
  if (msg != "")
    child->value(msg.c_str());
  root->append_node(child);
}

void XMLPARSER::visit(rapidxml::xml_node<> *node) {
  if (node == nullptr)
    return;
  for (rapidxml::xml_node<> *curr = node; curr; curr = curr->next_sibling()) {
    std::string tmp = curr->value();
    std::cout << curr->name() << " " << tmp << std::endl;
    for (rapidxml::xml_attribute<> *attr = curr->first_attribute(); attr;
         attr = attr->next_attribute()) {
      std::cout << "attr:" << attr->name() << ":" << attr->value() << std::endl;
    }

    if (tmp == "")
      visit(curr->first_node());
  }
}

std::unordered_map<std::string, std::pair<const char *, const char *>>
XMLPARSER::getAttrs(rapidxml::xml_node<> *curr) {
  std::unordered_map<std::string, std::pair<const char *, const char *>> hmap;
  for (rapidxml::xml_attribute<> *attr = curr->first_attribute(); attr;
       attr = attr->next_attribute()) {
    hmap[attr->name()] = std::make_pair(attr->name(), attr->value());
  }
  return hmap;
}
rapidxml::xml_node<> *XMLPARSER::getNode() { return node; }
XMLPARSER::XMLPARSER(std::vector<char> &xml) {
  rapidxml::xml_document<> doc;
  doc.parse<0>(&xml[0]);
  node = doc.first_node();
}
/*
#include <string>
int main() {
  std::string xml_str =
      "<?xml version=\"1.0\" "
      "encoding=\"UTF-8\"?>\n<create>\n<account "
      "id=\"123456\" balance=\"1000\"/>\n<symbol sym=\"SPY\">\n<account "
      "id=\"123456\">100000</account>\n</symbol>\n</create>\n";
  std::vector<char> xml(xml_str.begin(), xml_str.end());
  XMLPARSER XMLParser(xml);
}
*/
