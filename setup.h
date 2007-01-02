#ifndef VDR_LIVE_SETUP_H
#define VDR_LIVE_SETUP_H

#include <string>

namespace vdrlive {

class Setup
{
public:
	static Setup& Get();

	std::string const& GetLibraryPath() const { return m_libraryPath; }

	bool Parse( int argc, char* argv[] );
	
private:
	Setup();
	Setup( Setup const& );

	std::string m_libraryPath;
};

} // namespace vdrlive

#endif // VDR_LIVE_SETUP_H
