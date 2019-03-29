#include "xmlparser.h"
#include <iostream>
using namespace std;
void visit(rapidxml::xml_node<> *node) {
  if (node == nullptr)
    return;
  for (rapidxml::xml_node<> *curr = node; curr; curr = curr->next_sibling()) {
    cout << curr->name() << " " << curr->value() << " ";
    for (rapidxml::xml_attribute<> *attr = curr->first_attribute(); attr;
         attr = attr->next_attribute()) {
      cout << attr->name() << ":" << attr->value() << " ";
    }
    cout << "\n";
    std::string tmp = curr->value();
    if (tmp == "")
      visit(curr->first_node());
  }
}
void XMLPARSER::parse(std::vector<char> xml) {
  rapidxml::xml_document<> doc;
  doc.parse<0>(&xml[0]);
  rapidxml::xml_node<> *node = doc.first_node();
  visit(node);
}
#include <string>
int main() {
  std::string xml_str =
      "<?xml version=\"1.0\" "
      "encoding=\"UTF-8\"?>\n<create>\n<account "
      "id=\"123456\" balance=\"1000\"/>\n<symbol sym=\"SPY\">\n<account "
      "id=\"123456\">100000</account>\n</symbol>\n</create>\n";
  std::vector<char> xml(xml_str.begin(), xml_str.end());
  XMLPARSER XMLParser;
  XMLParser.parse(xml);
}
