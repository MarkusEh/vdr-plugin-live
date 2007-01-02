#include <getopt.h>
#include "setup.h"

namespace vdrlive {

Setup::Setup()
{
}

bool Setup::Parse( int argc, char* argv[] )
{
	static struct option opts[] = {
			{ "lib", required_argument, NULL, 'L' },
			{ 0 }
	};

	int optchar, optind = 0;
	while ( ( optchar = getopt_long( argc, argv, "L:", opts, &optind ) ) != -1 ) {
		switch ( optchar ) {
		case 'L': m_libraryPath = optarg; break;
		default:  return false;
		}
	}
	return true;
}

Setup& Setup::Get()
{
	static Setup instance;
	return instance;
}
	
} // namespace vdrlive
