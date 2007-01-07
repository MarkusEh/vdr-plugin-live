<# do not add to Makefile #>
<%pre>
#include <tnt/savepoint.h>
#include "exception.h"
</%pre>
<%request scope="global">
std::string pageTitle;
</%request>
<%cpp>
try {
	tnt::Savepoint spoint( reply );
</%cpp>
