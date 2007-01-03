#ifndef VDR_LIVE_TOOLS_H
#define VDR_LIVE_TOOLS_H

#include <ctime>
#include <string>

namespace vdrlive {

std::string FormatDateTime( char const* format, time_t time );

} // namespace vdrlive

#endif // VDR_LIVE_TOOLS_H
