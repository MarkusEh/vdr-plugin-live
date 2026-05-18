<%session scope="global">
std::string browserLocalTheme;
</%session>
<%pre>
// unfortunately there is no way to use inline functions as session-specific
// variables are not accessible outside of the function that builds the page
#define GetEffectiveTheme() (std::string(!browserLocalTheme.empty() ? browserLocalTheme : LiveSetup().GetTheme()))
#define GetThemedLinkPrefix() (std::string("themes/") + GetEffectiveTheme() + "/")
#define GetThemedLinkPrefixImg() (GetThemedLinkPrefix() + "img/")
#define GetThemedLink(type, name) (GetThemedLinkPrefix() + (type) + "/" + (name))
</%pre>
