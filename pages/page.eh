<%session scope="global">
std::string effectiveTheme;
</%session>
<%pre>
// unfortunately there is no way to use inline functions as session-specific
// variables are not accessible outside of the function that builds the page
#include "stringhelpers.h"
</%pre>
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
std::string themedLinkPrefix = concat("themes/", effectiveTheme, "/");

#define GetEffectiveTheme() (effectiveTheme)
#define GetThemedLinkPrefix() (themedLinkPrefix)
#define GetThemedLinkPrefixImg()  (cToSvConcat("themes/", effectiveTheme, "/img/"))
#define GetThemedLink(type, name) (cToSvConcat("themes/", effectiveTheme, "/", (type), "/", (name)))
</%cpp>
