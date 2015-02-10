<# do not add to Makefile #>
<%pre>
#include "exception.h"
</%pre>
<%cpp>
	spoint.commit();
} catch ( vdrlive::HtmlError const& ex ) {
	tnt::QueryParams param = qparam;
	param.add( "pageTitle", pageTitle );
	param.add( "errorMessage", ex.what() );
	callComp( "error", request, reply, param );
}
</%cpp>
