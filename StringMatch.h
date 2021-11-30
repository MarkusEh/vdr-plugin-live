#pragma once
#ifdef HAVE_PCRE2
#include <string>

class StringMatch {
private:
  void* re;
  void* match_data;
public:
  StringMatch(std::string Pattern);
  ~StringMatch();
  bool Matches(std::string s);
};
#endif
