<%pre>
// do not add to Makefile
// and do not write an ecpp comment into this file. It must produce no
// HTML output not even an empty line.
#include "stringhelpers.h"
</%pre>
<%session scope="global">
std::string effectiveTheme;
std::string last_page;
bool logged_in(false);
</%session>
<%cpp>
if (effectiveTheme.empty() ) {
  std::string browserLocalTheme = request.getCookie(liveNamePrefix cookieNameLocalTheme).getValue();
  if (browserLocalTheme.empty() ) {
    effectiveTheme = LiveSetup().GetTheme();
  } else {
// update the expiration date
    reply.setCookie(liveNamePrefix cookieNameLocalTheme, browserLocalTheme, 366*24*3600);
    effectiveTheme = browserLocalTheme;
  }
}

// unfortunately there is no way to use functions as session-specific
// variables are not accessible outside of the function that builds the page
// -> macros to get themed links
std::string themedLinkPrefix = concat("/themes/", effectiveTheme, "/");
#define GetEffectiveTheme() (effectiveTheme)
#define GetThemedLinkPrefix() (themedLinkPrefix)
#define GetThemedLinkPrefixImg()  (cToSvConcat("/themes/", effectiveTheme, "/img/"))
#define GetThemedLink(type, name) (cToSvConcat("/themes/", effectiveTheme, "/", (type), "/", (name)))
</%cpp>
