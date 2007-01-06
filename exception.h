#ifndef VDR_LIVE_EXCEPTION_H
#define VDR_LIVE_EXCEPTION_H

#include <stdexcept>

namespace vdrlive {

class HtmlError: public std::runtime_error
{
public:
	HtmlError( std::string const& title, std::string const& message ): std::runtime_error( message ), m_title( title ), m_message( message ) {}
	virtual ~HtmlError() throw() {}

	std::string const& GetTitle() const { return m_title; }
	std::string const& GetMessage() const { return m_message; }

private:
	std::string m_title;
	std::string m_message;
};

} // namespace vdrlive

#endif // VDR_LIVE_EXCEPTION_H
