<# do not add to Makefile #>
<%pre>
#include "exception.h"
</%pre>
<%cpp>
	spoint.commit();
} catch ( vdrlive::HtmlError const& ex ) {
#if TNTVERSION >= 1606
	tnt::QueryParams param = qparam;
#else
	cxxtools::QueryParams param = qparam;
#endif
	param.add( "pageTitle", pageTitle );
	param.add( "errorMessage", ex.what() );
	callComp( "error", request, reply, param );
}
</%cpp>
