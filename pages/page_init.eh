<%pre>
// do not add to Makefile
// and do not write a ecpp comment into this file. It must produce no
// html output not even a empty line.
#include <tnt/savepoint.h>
#include "exception.h"
</%pre>
<%request scope="global">
std::string pageTitle;
</%request>
<%cpp>
try {
	reply.setHeader("Expires", "Mon, 26 Jul 1997 05:00:00 GMT");
	tnt::Savepoint spoint( reply );
</%cpp>