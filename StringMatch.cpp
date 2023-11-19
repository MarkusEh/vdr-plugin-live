#include "StringMatch.h"
#ifdef HAVE_PCRE2

/* we use utf8, therefore we use code unit of 8 bit width.
 * define PCRE2_CODE_UNIT_WIDTH before #include <pcre2.h>
 */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>


StringMatch::StringMatch(cSv Pattern) : re(nullptr), match_data(nullptr) {
  PCRE2_SIZE erroroffset;
  int errorcode;

  if (not Pattern.empty()) {
     re = pcre2_compile(
            (PCRE2_SPTR) Pattern.data(),
            (PCRE2_SIZE) Pattern.length(),
             0
             | PCRE2_CASELESS           // Do caseless matching
             | PCRE2_DUPNAMES           // Allow duplicate names for subpatterns
             | PCRE2_NEVER_BACKSLASH_C  // Lock out the use of \C in patterns
             | PCRE2_NEVER_UCP          // Lock out PCRE2_UCP, e.g. via (*UCP)
             | PCRE2_NO_UTF_CHECK       // Do not check the pattern for UTF validity (only relevant if PCRE2_UTF is set)
             | PCRE2_UTF                // Treat pattern and subjects as UTF strings
            ,
            &errorcode,
            &erroroffset,
            nullptr);
     }

  if (re != nullptr)
     match_data = pcre2_match_data_create_from_pattern((const pcre2_code*)re, nullptr);
}


StringMatch::~StringMatch() {
  pcre2_match_data_free((pcre2_match_data*)match_data);
  pcre2_code_free((pcre2_code*)re);
}

bool StringMatch::Matches(cSv s) {
  if ((re == nullptr) or (match_data == nullptr))
     return false;

  int rc = pcre2_match(
              (pcre2_code*)re,               // the compiled pattern
              (PCRE2_SPTR) s.data(),         // the subject string
              (PCRE2_SIZE) s.length(),       // the length of the subject
              0,                             // start at offset 0 in the subject
              0,                             // default options
              (pcre2_match_data*)match_data, // block for storing the result
              nullptr);                      // use default match context

  return rc >= 0;
}

#endif
