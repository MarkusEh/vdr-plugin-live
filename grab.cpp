#include <cstdlib>
#include <vdr/device.h>
#include <vdr/tools.h>
#include "grab.h"

namespace vdrlive {

using namespace std;

const unsigned int GrabMinIntervalMs = 500; 
const unsigned int GrabPauseIntervalMs = GrabMinIntervalMs * 10;

class GrabImageTask: public StickyTask
{
public:
	explicit GrabImageTask( int quality = 80, int width = 320, int height = 240 )
		: m_firstTime( 0 )
		, m_lastTime( 0 )
		, m_quality( quality )
		, m_width( width )
		, m_height( height )
		{}

	void Activate() { m_firstTime = 0; }

private:
	uint64_t m_firstTime;
	uint64_t m_lastTime;
	int m_quality;
	int m_width;
	int m_height;
	
	virtual void Action();
};

void GrabImageTask::Action()
{
	uint64_t now = cTimeMs::Now();

	if ( m_firstTime == 0 )
		m_firstTime = now;

	if ( now - m_lastTime < GrabMinIntervalMs || now - m_firstTime > GrabPauseIntervalMs )
		return;

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
	m_lastTime = now;
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
	m_task->Activate();
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
