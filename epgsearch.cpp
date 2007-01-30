#include <vector>
#include <vdr/channels.h>
#include <vdr/plugin.h>
#include "epgsearch/services.h"
#include "epgsearch.h"
#include "exception.h"
#include "tools.h"

namespace vdrlive {

using namespace std;

bool operator<( SearchTimer const& left, SearchTimer const& right )
{
	return left.m_id < right.m_id;
}

SearchTimer::SearchTimer( string const& data ):
		m_id( 0 ),
		m_useTime( false ),
		m_startTime( 0 ),
		m_stopTime( 0 ),
		m_useChannel( NoChannel ),
		m_useCase( false ),
		m_mode( 0 ),
		m_useTitle( false ),
		m_useSubtitle( false ),
		m_useDescription( false ),
		m_useDuration( false ),
		m_minDuration( 0 ),
		m_maxDuration( 0 ),
		m_useAsSearchTimer( false ),
		m_useDayOfWeek( false ),
		m_dayOfWeek( 0 ),
		m_useEpisode( false ),
		m_priority( 0 ),
		m_lifetime( 0 )
{
	vector< string > parts = StringSplit( data, ':' );
	try {
		vector< string >::const_iterator part = parts.begin();
		for ( int i = 0; part != parts.end(); ++i, ++part ) {
			switch ( i ) {
			case  0: m_id = lexical_cast< int >( *part ); break;
			case  1: m_search = StringReplace( StringReplace( *part, "|", ":" ), "!^pipe^!", "|" ); break;
			case  2: m_useTime = lexical_cast< bool >( *part ); break;
			case  3: if ( m_useTime ) m_startTime = lexical_cast< int >( *part ); break;
			case  4: if ( m_useTime ) m_stopTime = lexical_cast< int >( *part ); break;
			case  5: m_useChannel = lexical_cast< int >( *part ); break;
			case  6: ParseChannel( *part ); break;
			case  7: m_useCase = lexical_cast< int >( *part ); break;
			case  8: m_mode = lexical_cast< int >( *part ); break;
			case  9: m_useTitle = lexical_cast< bool >( *part ); break;
			case 10: m_useSubtitle = lexical_cast< bool >( *part ); break;
			case 11: m_useDescription = lexical_cast< bool >( *part ); break;
			case 12: m_useDuration = lexical_cast< bool >( *part ); break;
			case 13: if ( m_useDuration ) m_minDuration = lexical_cast< int >( *part ); break;
			case 14: if ( m_useDuration ) m_maxDuration = lexical_cast< int >( *part ); break;
			case 15: m_useAsSearchTimer = lexical_cast< bool >( *part ); break;
			case 16: m_useDayOfWeek = lexical_cast< bool >( *part ); break;
			case 17: m_dayOfWeek = lexical_cast< int >( *part ); break;
			case 18: m_useEpisode = lexical_cast< bool >( *part ); break;
			case 19: m_directory = *part; break;
			case 20: m_priority = lexical_cast< int >( *part ); break;
			case 21: m_lifetime = lexical_cast< int >( *part ); break;
			}
		}
	} catch ( bad_lexical_cast const& ex ) {
	}
}

void SearchTimer::ParseChannel( string const& data )
{
	switch ( m_useChannel ) {
	case NoChannel: m_channels = tr("All"); break;
	case Interval: ParseChannelIDs( data ); break;
	case Group: m_channels = data; break;
	case FTAOnly: m_channels = tr("FTA"); break;
	}
}

void SearchTimer::ParseChannelIDs( string const& data )
{
	vector< string > parts = StringSplit( data, '|' );
	m_channelMin = lexical_cast< tChannelID >( parts[ 0 ] );

	cChannel const* channel = Channels.GetByChannelID( m_channelMin );
	if ( channel != 0 )
		m_channels = channel->Name();

	if ( parts.size() < 2 )
		return;

	m_channelMax = lexical_cast< tChannelID >( parts[ 1 ] );

	channel = Channels.GetByChannelID( m_channelMax );
	if ( channel != 0 )
		m_channels += string( " - " ) + channel->Name();
}

SearchTimers::SearchTimers()
{
	Epgsearch_services_v1_0 service;
	if ( cPluginManager::CallFirstService("Epgsearch-services-v1.0", &service) == 0 )
		throw HtmlError( tr("No searchtimers available") );

	ReadLock channelsLock( Channels, 0 );
	list< string > timers = service.handler->SearchTimerList();
	m_timers.assign( timers.begin(), timers.end() );
}

} // namespace vdrlive
