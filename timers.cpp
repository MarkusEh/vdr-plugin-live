#include <memory>
#include <sstream>
#include <vector>
#include "exception.h"
#include "timers.h"
#include "tools.h"
#include "i18n.h"

static bool operator<( cTimer const& left, cTimer const& right )
{
	return left.Compare( right ) < 0;
}

namespace vdrlive {

	using namespace std;
	using namespace vdrlive;

	static char const* const TIMER_DELETE = "DELETE";
	static char const* const TIMER_TOGGLE = "TOGGLE";

	SortedTimers::SortedTimers():
		m_state( 0 )
	{
		ReloadTimers( true );
	}

	string SortedTimers::GetTimerId( cTimer const& timer )
	{
		ostringstream builder;
		builder << timer.Channel()->GetChannelID() << ":" << timer.WeekDays() << ":"
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


	string SortedTimers::EncodeDomId(string const& timerid)
	{
		string tId("timer_");
		tId += vdrlive::EncodeDomId(timerid, ".-:", "pmc");
		return tId;
	}

	string SortedTimers::DecodeDomId(string const &timerDomId)
	{
		string const timerStr("timer_");

		string tId = timerDomId.substr(timerStr.length());

		return vdrlive::DecodeDomId(tId, "pmc", ".-:");
	}


	void SortedTimers::ReloadTimers( bool initial )
	{
		// dsyslog("live reloading timers");

		clear();
		for ( cTimer* timer = Timers.First(); timer; timer = Timers.Next( timer ) ) {
			push_back( *timer );
		}
		sort();
	}

	string SortedTimers::GetTimerDays(cTimer const& timer)
	{
		string currentDay = timer.WeekDays() > 0 ?
			*cTimer::PrintDay(0, timer.WeekDays(), true) :
			FormatDateTime(tr("%A, %x"), timer.Day());
		return currentDay;
	}

	string SortedTimers::GetTimerInfo(cTimer const& timer)
	{
		ostringstream info;
		info << trVDR("Priority") << ": " << timer.Priority() << endl;
		info << trVDR("Lifetime") << ": " << timer.Lifetime() << endl;
		info << trVDR("VPS") << ": " << (timer.HasFlags(tfVps)?trVDR("yes"):trVDR("no")) << endl;

		if (timer.Aux())
		{
			string epgsearchinfo = GetXMLValue(timer.Aux(), "epgsearch");
			if (!epgsearchinfo.empty())
			{
				string searchtimer = GetXMLValue(epgsearchinfo, "searchtimer");
				if (!searchtimer.empty())
					info << tr("Searchtimer") << ": " << searchtimer << endl;
			}
		}
		return info.str();
	}

	TimerManager::TimerManager()
	{
	}

	void TimerManager::UpdateTimer( cTimer* timer, int flags, tChannelID& channel, string const& weekdays, string const& day,
									int start, int stop, int priority, int lifetime, string const& title, string const& aux )
	{
		cMutexLock lock( this );

		ostringstream builder;
		builder << flags << ":"
				<< channel << ":"
				<< ( weekdays != "-------" ? weekdays : "" )
				<< ( weekdays == "-------" || day.empty() ? "" : "@" ) << day << ":"
				<< start << ":"
				<< stop << ":"
                << priority << ":"
				<< lifetime << ":"
				<< StringReplace(title, ":", "|" ) << ":"
				<< StringReplace(aux, ":", "|" );
		// Use StringReplace here because if ':' are characters in the
		// title or aux string it breaks parsing of timer definition
		// in VDRs cTimer::Parse method.  The '|' will be replaced
		// back to ':' by the cTimer::Parse() method.

		// Fix was submitted by rofafor: see
		// http://www.vdr-portal.de/board/thread.php?threadid=100398

 		// dsyslog("%s", builder.str().c_str());

		TimerPair timerData( timer, builder.str() );

		// dsyslog("SV: in UpdateTimer");
		m_updateTimers.push_back( timerData );
		// dsyslog("SV: wait for update");
		m_updateWait.Wait( *this );
		// dsyslog("SV: update done");

		string error = GetError( timerData );
		if ( !error.empty() )
			throw HtmlError( error );
	}

	void TimerManager::DelTimer( cTimer* timer )
	{
		cMutexLock lock( this );

		TimerPair timerData( timer, TIMER_DELETE );

		m_updateTimers.push_back( timerData );
		m_updateWait.Wait( *this );

		string error = GetError( timerData );
		if ( !error.empty() )
			throw HtmlError( error );
	}

	void TimerManager::ToggleTimerActive( cTimer* timer)
	{
		cMutexLock lock( this );

		TimerPair timerData( timer, TIMER_TOGGLE );

		m_updateTimers.push_back( timerData );
		m_updateWait.Wait( *this );

		string error = GetError( timerData );
		if ( !error.empty() )
			throw HtmlError( error );
	}

	void TimerManager::DoPendingWork()
	{
		if ( m_updateTimers.size() == 0 && !m_timers.Modified() )
			return;

		cMutexLock lock( this );
		if ( m_updateTimers.size() > 0 ) {
			DoUpdateTimers();
		}
		DoReloadTimers();
		// dsyslog("SV: signalling waiters");
		m_updateWait.Broadcast();
	}

	void TimerManager::DoUpdateTimers()
	{
		// dsyslog("SV: updating timers");
		for ( TimerList::iterator timer = m_updateTimers.begin(); timer != m_updateTimers.end(); ++timer ) {
			if ( timer->first == 0 ) // new timer
				DoInsertTimer( *timer );
			else if ( timer->second == TIMER_DELETE ) // delete timer
				DoDeleteTimer( *timer );
			else if ( timer->second == TIMER_TOGGLE ) // toggle timer
				DoToggleTimer( *timer );
			else // update timer
				DoUpdateTimer( *timer );
		}
		m_updateTimers.clear();
	}

	void TimerManager::DoInsertTimer( TimerPair& timerData )
	{
		auto_ptr< cTimer > newTimer( new cTimer );
		if ( !newTimer->Parse( timerData.second.c_str() ) ) {
			StoreError( timerData, tr("Error in timer settings") );
			return;
		}

		cTimer* checkTimer = Timers.GetTimer( newTimer.get() );
		if ( checkTimer ) {
			StoreError( timerData, tr("Timer already defined") );
			return;
		}

		Timers.Add( newTimer.get() );
		Timers.SetModified();
		isyslog( "live timer %s added", *newTimer->ToDescr() );
		newTimer.release();
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

	void TimerManager::DoDeleteTimer( TimerPair& timerData )
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
		if ( oldTimer->Recording() ) {
			oldTimer->Skip();
			cRecordControls::Process( time( 0 ) );
		}
		Timers.Del( oldTimer );
		Timers.SetModified();
		isyslog("live timer %s deleted", *copy.ToDescr());
	}

	void TimerManager::DoToggleTimer( TimerPair& timerData )
	{
		if ( Timers.BeingEdited() ) {
			StoreError( timerData, tr("Timers are being edited - try again later") );
			return;
		}

		cTimer* toggleTimer = Timers.GetTimer( timerData.first );
		if ( toggleTimer == 0 ) {
			StoreError( timerData, tr("Timer not defined") );
			return;
		}

		toggleTimer->OnOff();
		Timers.SetModified();
		isyslog("live timer %s toggled %s", *toggleTimer->ToDescr(), toggleTimer->HasFlags(tfActive) ? "on" : "off");
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

	const cTimer* TimerManager::GetTimer(tEventID eventid, tChannelID channelid)
	{
		cMutexLock timersLock( &LiveTimerManager() );
		SortedTimers& timers = LiveTimerManager().GetTimers();

		for ( SortedTimers::iterator timer = timers.begin(); timer != timers.end(); ++timer )
			if (timer->Channel() && timer->Channel()->GetChannelID() == channelid)
			{
				if (!timer->Event()) timer->SetEventFromSchedule();
				if (timer->Event() && timer->Event()->EventID() == eventid)
					return &*timer;
			}
		return NULL;
	}

	TimerManager& LiveTimerManager()
	{
		static TimerManager instance;
		return instance;
	}

} // namespace vdrlive
