#include "xmlparser.h"
#include <iostream>
using namespace std;
void visit(rapidxml::xml_node<> *node) {
  if (node == nullptr)
    return;
  for (rapidxml::xml_node<> *curr = node; curr; curr = curr->next_sibling()) {
    std::string tmp = curr->value();
    for (rapidxml::xml_attribute<> *attr = curr->first_attribute(); attr;
         attr = attr->next_attribute()) {
    }

    if (tmp == "")
      visit(curr->first_node());
  }
}
std::unordered_map<std::string, std::string>
getAttrs(rapidxml::xml_node<> *curr) {
  std::unordered_map<std::string, std::string> hmap;
  for (rapidxml::xml_attribute<> *attr = curr->first_attribute(); attr;
       attr = attr->next_attribute()) {
    hmap[attr->name()] = attr->value();
  }
  return hmap;
}
rapidxml::xml_node<> *XMLPARSER::getNode() { return node; }
XMLPARSER::XMLPARSER(std::vector<char> &xml) {
  rapidxml::xml_document<> doc;
  doc.parse<0>(&xml[0]);
  node = doc.first_node();
}
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
