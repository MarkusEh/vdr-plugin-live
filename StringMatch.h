#pragma once
#ifdef HAVE_PCRE2
#include <string>
#if TNTVERSION >= 30000
  #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif
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
