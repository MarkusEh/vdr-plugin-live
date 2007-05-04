#ifndef VDR_LIVE_GRAB_H
#define VDR_LIVE_GRAB_H

#include <boost/shared_array.hpp>
#include "tasks.h"

namespace vdrlive {

typedef boost::shared_array< char > GrabImagePtr;
typedef std::pair< GrabImagePtr, int > GrabImageInfo;

class GrabImageTask;

class GrabImageManager
{
	friend GrabImageManager& LiveGrabImageManager();
	friend class GrabImageTask;

public:
	GrabImageInfo GetImage() const;

private:
	GrabImageManager();
	GrabImageManager( GrabImageManager const& );

	GrabImageManager& operator=( GrabImageManager const& );

	void PutImage( char* image, int size );

	std::auto_ptr< GrabImageTask > m_task;
	GrabImagePtr m_image;
	int m_size;
};

GrabImageManager& LiveGrabImageManager();

} // namespace vdrlive

#endif // VDR_LIVE_GRAB_H
