#ifndef __HELPER_H__
#define __HELPER_H__
#include <algorithm>
#include <ctime>
#include <map>
#include <string>
#include <vector>
class Helper {
public:
  std::string trimLeadingSpace(std::string &msg);
  std::string fetchNextSeg(std::string &msg, char c = ' ',
                           size_t substrlen = 1);
  std::string tolower(const std::string &msg);
  int wdayTable(std::string wday);
  int monTable(std::string mon);
  double HTTPTimeRange2Num(std::string end, std::string start);
  std::vector<char> deleteALine(std::vector<char> &msg,
                                std::vector<char>::iterator begin);
  size_t HTTPAge(std::string date);
  struct tm strtotm(std::string date);
  bool containNewLine(const std::vector<char> &str);
};
#endif
