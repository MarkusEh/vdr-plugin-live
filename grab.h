#ifndef VDR_LIVE_GRAB_H
#define VDR_LIVE_GRAB_H

#include "stdext.h"
#include "tasks.h"

namespace vdrlive {

typedef std::tr1::shared_ptr< char > GrabImagePtr;
typedef std::pair< GrabImagePtr, int > GrabImageInfo;

class GrabImageTask;

class GrabImageManager
{
	friend GrabImageManager& LiveGrabImageManager();
	friend class GrabImageTask;

public:
	bool CanGrab() const;
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
