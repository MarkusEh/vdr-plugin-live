#ifndef VDR_LIVE_EXCEPTION_H
#define VDR_LIVE_EXCEPTION_H

#include <stdexcept>

namespace vdrlive {

class HtmlError: public std::runtime_error
{
public:
	HtmlError( std::string const& message ): std::runtime_error( message ) {}
	virtual ~HtmlError() throw() {}
};

} // namespace vdrlive

#endif // VDR_LIVE_EXCEPTION_H
