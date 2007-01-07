#include <memory>
#include <sstream>
#include <vector>
#include "exception.h"
#include "timers.h"
#include "tools.h"

static bool operator<( cTimer const& left, cTimer const& right )
{
	return left.Compare( right ) < 0;
}

namespace vdrlive {

using namespace std;
using namespace vdrlive;

SortedTimers::SortedTimers():
		m_state( 0 )
{
	ReloadTimers( true );
}

string SortedTimers::GetTimerId( cTimer const& timer )
{
	ostringstream builder;
	builder << *timer.Channel()->GetChannelID().ToString() << ":" << timer.WeekDays() << ":"
			<< timer.Day() << ":" << timer.Start() << ":" << timer.Stop();
	return builder.str();
}

cTimer* SortedTimers::GetByTimerId( string const& timerid )
{
	vector< string > parts = StringSplit( timerid, ':' );
	if ( parts.size() < 5 ) {
		esyslog("GetByTimerId: invalid format %s", timerid.c_str() );
		return 0;
	}

	cChannel* channel = Channels.GetByChannelID( tChannelID::FromString( parts[0].c_str() ) );
	if ( channel == 0 ) {
		esyslog("GetByTimerId: no channel %s", parts[0].c_str() );
		return 0;
	}

	try {
		int weekdays = lexical_cast< int >( parts[1] );
		time_t day = lexical_cast< time_t >( parts[2] );
		int start = lexical_cast< int >( parts[3] );
		int stop = lexical_cast< int >( parts[4] );

		for ( SortedTimers::iterator timer = begin(); timer != end(); ++timer ) {
			if ( timer->Channel() == channel && 
					( ( weekdays != 0 && timer->WeekDays() == weekdays ) || ( weekdays == 0 && timer->Day() == day ) ) &&
					timer->Start() == start && timer->Stop() == stop )
				return &*timer;
		}
	} catch ( bad_lexical_cast const& ex ) {
		esyslog("GetByTimer: bad cast");
	}
	return 0;
}

void SortedTimers::ReloadTimers( bool initial )
{
	if ( !Timers.Modified( m_state ) && !initial )
		return;

	dsyslog("live reloading timers");

	clear();
	for ( cTimer* timer = Timers.First(); timer != 0; timer = Timers.Next( timer ) ) {
		push_back( *timer );
	}
	sort();
}

TimerManager::TimerManager()
{
}

void TimerManager::UpdateTimer( cTimer* timer, int flags, string const& channel, string const& weekdays, string const& day,
								int start, int stop, int priority, int lifetime, string const& title, string const& aux )
{
	ostringstream builder;
	builder << flags << ":" << channel << ":" << ( weekdays != "-------" ? weekdays : "" )
			<< ( weekdays == "-------" || day.empty() ? "" : "@" ) << day << ":" << start << ":" << stop << ":"
			<< priority << ":" << lifetime << ":" << title << ":" << aux;
	dsyslog("%s", builder.str().c_str());

	TimerPair timerData( timer, builder.str() );

	dsyslog("SV: in UpdateTimer");
	m_updateTimers.push_back( timerData );
	dsyslog("SV: wait for update");
	m_updateWait.Wait( *this );
	dsyslog("SV: update done");

	string error = GetError( timerData );
	if ( !error.empty() )
		throw HtmlError( error );
}

void TimerManager::DoPendingWork()
{
	cMutexLock lock( this );
	if ( m_updateTimers.size() > 0 )
		DoUpdateTimers();
	m_timers.ReloadTimers();
}

void TimerManager::DoUpdateTimers()
{
	dsyslog("SV: updating timers");
	for ( TimerList::iterator timer = m_updateTimers.begin(); timer != m_updateTimers.end(); ++timer ) {
		if ( timer->first == 0 ) // new timer
			DoInsertTimer( *timer );
		else if ( timer->second == "" ) // delete timer
			; // XXX
		else // update timer
			DoUpdateTimer( *timer );
	}
	m_updateTimers.clear();
	dsyslog("SV: signalling waiters");
	m_updateWait.Broadcast();
}

void TimerManager::DoInsertTimer( TimerPair& timerData )
{
	auto_ptr< cTimer > newTimer( new cTimer );
	if ( !newTimer->Parse( timerData.second.c_str() ) ) {
		StoreError( timerData, tr("Error in timer settings") );
		return;
	}

	cTimer* checkTimer = Timers.GetTimer( newTimer.get() );
	if ( checkTimer != 0 ) {
		StoreError( timerData, tr("Timer already defined") );
		return;
	}

	Timers.Add( newTimer.release() );
	Timers.SetModified();
	isyslog( "live timer %s added", *newTimer->ToDescr() );
}

void TimerManager::DoUpdateTimer( TimerPair& timerData )
{
	if ( Timers.BeingEdited() ) {
		StoreError( timerData, tr("Timers are being edited - try again later") );
		return;
	}

	cTimer* oldTimer = Timers.GetTimer( timerData.first );
	if ( oldTimer == 0 ) {
		StoreError( timerData, tr("Timer not defined") );
		return;
	}

	cTimer copy = *oldTimer;
	if ( !copy.Parse( timerData.second.c_str() ) ) {
		StoreError( timerData, tr("Error in timer settings") );
		return;
	}

	*oldTimer = copy;
	Timers.SetModified();
	isyslog("live timer %s modified (%s)", *oldTimer->ToDescr(), oldTimer->HasFlags(tfActive) ? "active" : "inactive");
}

void TimerManager::StoreError( TimerPair const& timerData, std::string const& error )
{
	m_failedUpdates.push_back( ErrorPair( timerData, error ) );
}

string TimerManager::GetError( TimerPair const& timerData )
{
	for ( ErrorList::iterator error = m_failedUpdates.begin(); error != m_failedUpdates.end(); ++error ) {
		if ( error->first == timerData ) {
			string message = error->second;
			m_failedUpdates.erase( error );
			return message;
		}
	}
	return "";
}

TimerManager& LiveTimerManager()
{
	static TimerManager instance;
	return instance;
}

} // namespace vdrlive
