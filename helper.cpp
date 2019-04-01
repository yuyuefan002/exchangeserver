#include "helper.h"
/*
 * trimleadingspace
 * This function can delete all leading space in a string
 *
 * warning: this function will motify original string
 * Test Status: pass unit test
 */
std::string Helper::trimLeadingSpace(std::string &msg) {
  if (msg.empty())
    return "";
  size_t target = msg.find_first_not_of(' ');
  if (target == std::string::npos)
    return msg;
  return msg.substr(target);
}
/*
 * fetchnextseg
 * This function can fetch next segment, split by c.
 *
 * warning: this function will motify original string
 * Test Status: pass unit test
 */
std::string Helper::fetchNextSeg(std::string &msg, char c, size_t substrlen) {
  msg = trimLeadingSpace(msg);
  size_t target = msg.find(c);
  std::string res = msg.substr(0, target);
  if (target != std::string::npos)
    msg = msg.substr(target + substrlen);
  else
    msg = "";

  return res;
}
/*
 * tolower
 * This function can let all char in a string be lower char.
 *
 * Test Status: pass unit test
 */
std::string Helper::tolower(const std::string &msg) {
  std::string res;
  for (auto c : msg) {
    res += std::tolower(c);
  }
  return res;
}
/*
 * wdayTable
 * This function maps all weekdays with number
 *
 * Test Status: pass unit test
 */
int Helper::wdayTable(std::string wday) {
  std::map<std::string, int> w = {{"Sun", 0}, {"Mon", 1}, {"Tue", 2},
                                  {"Wed", 3}, {"Thu", 4}, {"Fri", 5},
                                  {"Sat", 6}};
  return w[wday];
}

/*
 * monTable
 * This function maps all month with number
 *
 * Test Status: pass unit test
 */
int Helper::monTable(std::string mon) {
  std::map<std::string, int> m = {
      {"Jan", 0}, {"Feb", 1}, {"Mar", 2}, {"Apr", 3}, {"May", 4},  {"Jun", 5},
      {"Jul", 6}, {"Aug", 7}, {"Sep", 8}, {"Oct", 9}, {"Nov", 10}, {"Dec", 11}};
  return m[mon];
}
/*
 * HTTPTimerange2num
 * This function can calculate the time range between two HTTP TIME
 *
 * past problem: after difftime, end_tm.tm_hour will increase 1, cannot resolve
 * solution: set tm_isdst
 * Test Status:pass unit test
 */
double Helper::HTTPTimeRange2Num(std::string end, std::string start) {
  double seconds;

  struct tm end_tm, start_tm;
  end_tm.tm_wday = wdayTable(fetchNextSeg(end, ','));
  end_tm.tm_mday = stoi(fetchNextSeg(end));
  end_tm.tm_mon = monTable(fetchNextSeg(end));
  end_tm.tm_year = stoi(fetchNextSeg(end)) - 1900;
  end_tm.tm_hour = stoi(fetchNextSeg(end, ':'));
  end_tm.tm_min = stoi(fetchNextSeg(end, ':'));
  end_tm.tm_sec = stoi(fetchNextSeg(end));
  end_tm.tm_isdst =
      0; // this is important, missing this will cause ambigous time
  std::string zone = fetchNextSeg(end);
  end_tm.tm_zone = zone.c_str();

  start_tm.tm_wday = wdayTable(fetchNextSeg(start, ','));
  start_tm.tm_mday = stoi(fetchNextSeg(start));
  start_tm.tm_mon = monTable(fetchNextSeg(start));
  start_tm.tm_year = stoi(fetchNextSeg(start)) - 1900;
  start_tm.tm_hour = stoi(fetchNextSeg(start, ':'));
  start_tm.tm_min = stoi(fetchNextSeg(start, ':'));
  start_tm.tm_sec = stoi(fetchNextSeg(start));
  start_tm.tm_isdst = 0;
  std::string zone2 = fetchNextSeg(start);
  start_tm.tm_zone = zone2.c_str();

  seconds = difftime(mktime(&end_tm), mktime(&start_tm));
  return seconds;
}
/*
 * HTTPAge
 * This function can calculate the diff between now and HTTP time data
 *
 * warning: something unsure may happen, currently usring gmt time,
 *          i don't know how difftime works, will it change
 *          timezone automatically?
 */
size_t Helper::HTTPAge(std::string date) {
  size_t seconds;
  struct tm date_tm;
  date_tm.tm_wday = wdayTable(fetchNextSeg(date, ','));
  date_tm.tm_mday = stoi(fetchNextSeg(date));
  date_tm.tm_mon = monTable(fetchNextSeg(date));
  date_tm.tm_year = stoi(fetchNextSeg(date)) - 1900;
  date_tm.tm_hour = stoi(fetchNextSeg(date, ':'));
  date_tm.tm_min = stoi(fetchNextSeg(date, ':'));
  date_tm.tm_sec = stoi(fetchNextSeg(date));
  date_tm.tm_isdst = 0;
  std::string zone2 = fetchNextSeg(date);
  date_tm.tm_zone = zone2.c_str();
  time_t now;
  time(&now);
  struct tm *ptm = gmtime(&now);
  seconds = difftime(mktime(ptm), mktime(&date_tm));
  return seconds;
}
std::vector<char> Helper::deleteALine(std::vector<char> &msg,
                                      std::vector<char>::iterator begin) {
  std::vector<char>::iterator it = begin;
  while (*it != '\n') {
    it++;
  }
  auto end = it + 1;
  msg.erase(begin, end);
  return msg;
}

struct tm Helper::strtotm(std::string date) {
  struct tm time;
  time.tm_wday = wdayTable(fetchNextSeg(date, ','));
  time.tm_mday = stoi(fetchNextSeg(date));
  time.tm_mon = monTable(fetchNextSeg(date));
  time.tm_year = stoi(fetchNextSeg(date)) - 1900;
  time.tm_hour = stoi(fetchNextSeg(date, ':'));
  time.tm_min = stoi(fetchNextSeg(date, ':'));
  time.tm_sec = stoi(fetchNextSeg(date));
  time.tm_isdst = 0;
  std::string zone = fetchNextSeg(date);
  time.tm_zone = zone.c_str();
  return time;
}

bool Helper::containNewLine(const std::vector<char> &str) {
  std::vector<char> pattern{'\r', '\n', '\r', '\n'};
  if (std::search(str.begin(), str.end(), pattern.begin(), pattern.end()) ==
      str.end()) {
    return false;
  }
  return true;
}

/*
int main() {
  Helper helper;
  //  std::cout << helper.HTTPTimeRange2Num("Fri, 08 Feb 2019 18:44:41 GMT",
  //                                     "Fri, 08 Feb 2019 18:43:41 GMT")
  //<< std::endl;
  // std::cout << helper.HTTPAge("Date: Mon, 18 Feb 2019 20:55:11 GMT");
  std::vector<char> R = {'a',  's',  'd', '\r', '\n', 'a',  's', 'd',
                         '\r', '\n', 'a', 's',  'd',  '\r', '\n'};
  std::vector<char>::iterator it = R.begin();
  R = helper.deleteALine(R, it);
  std::cout << R.data();
}
*/
