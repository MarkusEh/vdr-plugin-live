#pragma once
#ifdef HAVE_PCRE2
#include <string>
#include <vdr/channels.h>
#include "stringhelpers.h"

class StringMatch {
private:
  void* re;
  void* match_data;
public:
  StringMatch(cSv Pattern);
  ~StringMatch();
  bool Matches(cSv s);
};
#endif
