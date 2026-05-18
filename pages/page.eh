<%session scope="global">
std::string browserLocalTheme;
</%session>
<%pre>
// unfortunately there is no way to use inline functions as session-specific
// variables are not accessible outside of the function that builds the page
#include "stringhelpers.h"
</%pre>
<%cpp>
std::string effectiveTheme = browserLocalTheme.empty() ? LiveSetup().GetTheme(): browserLocalTheme;
std::string themedLinkPrefix = concat("themes/", effectiveTheme, "/");

#define GetEffectiveTheme() (effectiveTheme)
#define GetThemedLinkPrefix() (themedLinkPrefix)
#define GetThemedLinkPrefixImg()  (cToSvConcat("themes/", browserLocalTheme.empty() ? LiveSetup().GetTheme(): browserLocalTheme, "/img/"))
#define GetThemedLink(type, name) (cToSvConcat("themes/", browserLocalTheme.empty() ? LiveSetup().GetTheme(): browserLocalTheme, "/", (type), "/", (name)))
</%cpp>
