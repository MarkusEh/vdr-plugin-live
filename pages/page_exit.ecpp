<# do not add to Makefile #>
<%cpp>
	spoint.commit();
} catch ( HtmlError const& ex ) {
	cxxtools::QueryParams param = qparam;
	param.add( "pageTitle", pageTitle );
	param.add( "errorMessage", ex.what() );
	callComp( "error", request, reply, param );
}
</%cpp>
