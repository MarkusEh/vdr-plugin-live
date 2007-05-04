#include <cstdlib>
#include <vdr/device.h>
#include "grab.h"

namespace vdrlive {

using namespace std;

class GrabImageTask: public StickyTask
{
public:
	explicit GrabImageTask( int quality = 80, int width = 320, int height = 240 )
		: m_quality( quality )
		, m_width( width )
		, m_height( height )
		{}

private:
	int m_quality;
	int m_width;
	int m_height;
	
	virtual void Action();
};

void GrabImageTask::Action()
{
	cDevice* device = cDevice::PrimaryDevice();
	if ( device == 0 ) {
		SetError( tr("Couldn't aquire primary device") );
		return;
	}

	int size = 0;
	uchar* image = device->GrabImage( size, true, m_quality, m_width, m_height );
	if ( image == 0 ) {
		SetError( tr("Couldn't grab image from primary device") );
		return;
	}

	LiveGrabImageManager().PutImage( reinterpret_cast< char* >( image ), size );
}

GrabImageManager::GrabImageManager()
		: m_task( new GrabImageTask )
		, m_image( 0, &free )
		, m_size( 0 )
{
}

GrabImageInfo GrabImageManager::GetImage() const 
{
	cMutexLock lock( &LiveTaskManager() );
	return make_pair( m_image, m_size );
}

void GrabImageManager::PutImage( char* image, int size )
{
	m_image.reset( image, &free );
	m_size = size;
}

GrabImageManager& LiveGrabImageManager() 
{
	static GrabImageManager instance;
	return instance;
}

} // namespace vdrlive
