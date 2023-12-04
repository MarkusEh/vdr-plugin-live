
#include "epgsearch.h"

#include "epgsearch/services.h"
#include "exception.h"
#include "livefeatures.h"
#include "tools.h"

#include <iomanip>

namespace vdrlive {

static char ServiceInterface[] = "Epgsearch-services-v1.0";

bool operator<( SearchTimer const& left, SearchTimer const& right )
{
   std::string leftlower = left.m_search;
   std::string rightlower = right.m_search;
   std::transform(leftlower.begin(), leftlower.end(), leftlower.begin(), (int(*)(int)) tolower);
   std::transform(rightlower.begin(), rightlower.end(), rightlower.begin(), (int(*)(int)) tolower);
   return leftlower < rightlower;
}

bool CheckEpgsearchVersion()
{
	/* @winni: Falls Du an der Versionsnummer Anpassungen vornehmen willst, mach das bitte in livefeatures.h ganz unten. Danke */
	const Features<features::epgsearch>& f = LiveFeatures<features::epgsearch>();
	if ( f.Loaded() ) {
		if ( !f.Recent() )
			throw HtmlError( tr("Required minimum version of epgsearch: ") + std::string( f.MinVersion() ));
		return true;
	}
	return false;
}

SearchTimer::SearchTimer()
{
   Init();
}

void SearchTimer::Init()
{
	m_id = -1;
	m_useTime = false;
	m_startTime = 0;
	m_stopTime = 0;
	m_useChannel = NoChannel;
	m_useCase = false;
	m_mode = 0;
	m_useTitle = true;
	m_useSubtitle = true;
	m_useDescription = true;
	m_useDuration = false;
	m_minDuration = 0;
	m_maxDuration = 0;
	m_useDayOfWeek = false;
	m_dayOfWeek = 0;
	m_useEpisode = false;
	m_priority = parse_int<int>(EPGSearchSetupValues::ReadValue("DefPriority"));
	m_lifetime = parse_int<int>(EPGSearchSetupValues::ReadValue("DefLifetime"));
	m_fuzzytolerance = 1;
	m_useInFavorites = false;
	m_useAsSearchtimer = 0;
	m_action = 0;
	m_delAfterDays = 0;
	m_recordingsKeep = 0;
	m_pauseOnNrRecordings = 0;
	m_switchMinBefore = 1;
	m_useExtEPGInfo = false;
	m_useVPS = false;
	m_marginstart = parse_int<int>(EPGSearchSetupValues::ReadValue("DefMarginStart"));
	m_marginstop = parse_int<int>(EPGSearchSetupValues::ReadValue("DefMarginStop"));
	m_avoidrepeats = false;
	m_allowedrepeats = 0;
	m_compareTitle = false;
	m_compareSubtitle = 0;
	m_compareSummary = false;
	m_repeatsWithinDays = 0;
	m_blacklistmode = 0;
	m_menuTemplate = 0;
	m_delMode = 0;
	m_delAfterCountRecs = 0;
	m_delAfterDaysOfFirstRec = 0;
	m_useAsSearchTimerFrom = 0;
	m_useAsSearchTimerTil = 0;
	m_catvaluesAvoidRepeat = 0;
	m_ignoreMissingEPGCats = false;
}

SearchTimer::SearchTimer( std::string const& data )
{
   Init();
   std::vector<std::string> parts = StringSplit( data, ':' );
   try {
      std::vector<std::string>::const_iterator part = parts.begin();
      for ( int i = 0; part != parts.end(); ++i, ++part ) {
			switch ( i ) {
			case  0: m_id = parse_int<int>( *part ); break;
			case  1: m_search = StringReplace( StringReplace( *part, "|", ":" ), "!^pipe^!", "|" ); break;
			case  2: m_useTime = lexical_cast<bool>( *part ); break;
			case  3: if ( m_useTime ) m_startTime = parse_int<int>( *part ); break;
			case  4: if ( m_useTime ) m_stopTime = parse_int<int>( *part ); break;
			case  5: m_useChannel = parse_int<int>( *part ); break;
			case  6: ParseChannel( *part ); break;
			case  7: m_useCase = parse_int<int>( *part ); break;
			case  8: m_mode = parse_int<int>( *part ); break;
			case  9: m_useTitle = lexical_cast<bool>( *part ); break;
			case 10: m_useSubtitle = lexical_cast<bool>( *part ); break;
			case 11: m_useDescription = lexical_cast<bool>( *part ); break;
			case 12: m_useDuration = lexical_cast<bool>( *part ); break;
			case 13: if ( m_useDuration ) m_minDuration = parse_int<int>( *part ); break;
			case 14: if ( m_useDuration ) m_maxDuration = parse_int<int>( *part ); break;
			case 15: m_useAsSearchtimer = parse_int<int>( *part ); break;
			case 16: m_useDayOfWeek = lexical_cast<bool>( *part ); break;
			case 17: m_dayOfWeek = parse_int<int>( *part ); break;
			case 18: m_useEpisode = lexical_cast<bool>( *part ); break;
			case 19: m_directory = StringReplace( StringReplace( *part, "|", ":" ), "!^pipe^!", "|" ); break;
			case 20: m_priority = parse_int<int>( *part ); break;
			case 21: m_lifetime = parse_int<int>( *part ); break;
			case 22: m_marginstart = parse_int<int>( *part ); break;
			case 23: m_marginstop = parse_int<int>( *part ); break;
			case 24: m_useVPS = lexical_cast<bool>( *part ); break;
			case 25: m_action = parse_int<int>( *part ); break;
			case 26: m_useExtEPGInfo = lexical_cast<bool>( *part ); break;
			case 27: ParseExtEPGInfo( *part ); break;
			case 28: m_avoidrepeats = lexical_cast<bool>( *part ); break;
			case 29: m_allowedrepeats = parse_int<int>( *part ); break;
			case 30: m_compareTitle = lexical_cast<bool>( *part ); break;
			case 31: m_compareSubtitle = parse_int<int>( *part ); break;
			case 32: m_compareSummary = lexical_cast<bool>( *part ); break;
			case 33: m_catvaluesAvoidRepeat = parse_int< unsigned long >( *part ); break;
			case 34: m_repeatsWithinDays = parse_int<int>( *part ); break;
			case 35: m_delAfterDays = parse_int<int>( *part ); break;
			case 36: m_recordingsKeep = parse_int<int>( *part ); break;
			case 37: m_switchMinBefore = parse_int<int>( *part ); break;
			case 38: m_pauseOnNrRecordings = parse_int<int>( *part ); break;
			case 39: m_blacklistmode = parse_int<int>( *part ); break;
			case 40: ParseBlacklist( *part ); break;
			case 41: m_fuzzytolerance = parse_int<int>( *part ); break;
			case 42: m_useInFavorites = lexical_cast<bool>( *part ); break;
			case 43: m_menuTemplate = parse_int<int>( *part ); break;
			case 44: m_delMode = parse_int<int>( *part ); break;
			case 45: m_delAfterCountRecs = parse_int<int>( *part ); break;
			case 46: m_delAfterDaysOfFirstRec = parse_int<int>( *part ); break;
			case 47: m_useAsSearchTimerFrom = parse_int<time_t>( *part ); break;
			case 48: m_useAsSearchTimerTil = parse_int<time_t>( *part ); break;
			case 49: m_ignoreMissingEPGCats = lexical_cast<bool>( *part ); break;
			}
		}
	} catch ( bad_lexical_cast const& ex ) {
	}
}

std::string SearchTimer::ToText()
{
   std::string tmp_Start;
   std::string tmp_Stop;
   std::string tmp_minDuration;
   std::string tmp_maxDuration;
   std::string tmp_chanSel;
   std::string tmp_search;
   std::string tmp_directory;
   std::string tmp_catvalues;
   std::string tmp_blacklists;

   tmp_search = StringReplace(StringReplace(m_search, "|", "!^pipe^!"), ":", "|");
   tmp_directory = StringReplace(StringReplace(m_directory, "|", "!^pipe^!"), ":", "|");

   if (m_useTime)
   {
      std::stringstream os;
      os << std::setw(4) << std::setfill('0') << m_startTime;
      tmp_Start = os.str();
      os.str("");
      os << std::setw(4) << std::setfill('0') << m_stopTime;
      tmp_Stop = os.str();
   }
   if (m_useDuration)
   {
      std::stringstream os;
      os << std::setw(4) << std::setfill('0') << m_minDuration;
      tmp_minDuration = os.str();
      os.str("");
      os << std::setw(4) << std::setfill('0') << m_maxDuration;
      tmp_maxDuration = os.str();
   }

   if (m_useChannel==1)
   {
#if VDRVERSNUM >= 20301
      LOCK_CHANNELS_READ;
      cChannel const* channelMin = Channels->GetByChannelID( m_channelMin );
      cChannel const* channelMax = Channels->GetByChannelID( m_channelMax );
#else
      cChannel const* channelMin = Channels.GetByChannelID( m_channelMin );
      cChannel const* channelMax = Channels.GetByChannelID( m_channelMax );
#endif

      if (channelMax && channelMin->Number() < channelMax->Number())
         tmp_chanSel = *m_channelMin.ToString() + std::string("|") + *m_channelMax.ToString();
      else
         tmp_chanSel = *m_channelMin.ToString();
   }
   if (m_useChannel==2)
      tmp_chanSel = m_channels;

   if (m_useExtEPGInfo)
   {
      for(unsigned int i=0; i<m_ExtEPGInfo.size(); i++)
         tmp_catvalues += (tmp_catvalues != ""?"|":"") +
            StringReplace(StringReplace(m_ExtEPGInfo[i], ":", "!^colon^!"), "|", "!^pipe^!");
   }

   if (m_blacklistmode == 1)
   {
      for(unsigned int i=0; i<m_blacklistIDs.size(); i++)
         tmp_blacklists += (tmp_blacklists != ""?"|":"") +  m_blacklistIDs[i];
   }

   std::stringstream os;
   os << m_id << ":"
      << tmp_search << ":"
      << (m_useTime?1:0) << ":"
      << tmp_Start << ":"
      << tmp_Stop << ":"
      << m_useChannel << ":"
      << ((m_useChannel>0 && m_useChannel<3)?tmp_chanSel:"0") << ":"
      << (m_useCase?1:0) << ":"
      << m_mode << ":"
      << (m_useTitle?1:0) << ":"
      << (m_useSubtitle?1:0) << ":"
      << (m_useDescription?1:0) << ":"
      << (m_useDuration?1:0) << ":"
      << tmp_minDuration << ":"
      << tmp_maxDuration << ":"
      << m_useAsSearchtimer << ":"
      << (m_useDayOfWeek?1:0) << ":"
      << m_dayOfWeek << ":"
      << (m_useEpisode?1:0) << ":"
      << tmp_directory << ":"
      << m_priority << ":"
      << m_lifetime << ":"
      << m_marginstart << ":"
      << m_marginstop << ":"
      << (m_useVPS?1:0) << ":"
      << m_action << ":"
      << (m_useExtEPGInfo?1:0) << ":"
      << tmp_catvalues << ":"
      << (m_avoidrepeats?1:0) << ":"
      << m_allowedrepeats << ":"
      << (m_compareTitle?1:0) << ":"
      << m_compareSubtitle << ":"
      << (m_compareSummary?1:0) << ":"
      << m_catvaluesAvoidRepeat << ":"
      << m_repeatsWithinDays << ":"
      << m_delAfterDays << ":"
      << m_recordingsKeep << ":"
      <<  m_switchMinBefore << ":"
      << m_pauseOnNrRecordings << ":"
      << m_blacklistmode << ":"
      << tmp_blacklists << ":"
      << m_fuzzytolerance << ":"
      << (m_useInFavorites?1:0) << ":"
      << m_menuTemplate << ":"
      << m_delMode << ":"
      << m_delAfterCountRecs << ":"
      << m_delAfterDaysOfFirstRec << ":"
      << (long) m_useAsSearchTimerFrom << ":"
      << (long) m_useAsSearchTimerTil << ":"
      << m_ignoreMissingEPGCats;

   return os.str();
}

void SearchTimer::ParseChannel( std::string const& data )
{
	switch ( m_useChannel ) {
    case NoChannel: m_channels = tr("All"); break;
    case Interval: ParseChannelIDs( data ); break;
    case Group: m_channels = data; break;
    case FTAOnly: m_channels = tr("FTA"); break;
	}
}

void SearchTimer::ParseChannelIDs( std::string const& data )
{
	std::vector<std::string> parts = StringSplit( data, '|' );
	m_channelMin = tChannelID::FromString( parts[ 0 ].c_str() );

	LOCK_CHANNELS_READ;
	cChannel const* channel = Channels->GetByChannelID( m_channelMin );
	if ( channel != 0 )
		m_channels = channel->Name();

	if ( parts.size() < 2 )
		return;

	m_channelMax = tChannelID::FromString( parts[ 1 ].c_str() );

	channel = Channels->GetByChannelID( m_channelMax );
	if ( channel != 0 )
		m_channels += std::string( " - " ) + channel->Name();
}

void SearchTimer::ParseExtEPGInfo( std::string const& data )
{
   m_ExtEPGInfo = StringSplit( data, '|' );
}

void SearchTimer::ParseBlacklist( std::string const& data )
{
   m_blacklistIDs = StringSplit( data, '|' );
}

std::string SearchTimer::StartTimeFormatted()
{
	time_t start = cTimer::SetTime(time(NULL), (((StartTime() / 100 ) % 100) * 60 * 60) + (StartTime() % 100 * 60));
	return FormatDateTime(tr("%I:%M %p"), start);
}

std::string SearchTimer::StopTimeFormatted()
{
	time_t stop = cTimer::SetTime(time(NULL), (((StopTime() / 100 ) % 100) * 60 * 60) + (StopTime() % 100 * 60));
	return FormatDateTime(tr("%I:%M %p"), stop);
}

std::string SearchTimer::UseAsSearchTimerFrom(std::string const& format)
{
	return DatePickerToC(m_useAsSearchTimerFrom, format);
}

std::string SearchTimer::UseAsSearchTimerTil(std::string const& format)
{
	return DatePickerToC(m_useAsSearchTimerTil, format);
}

void SearchTimer::SetUseAsSearchTimerFrom(std::string const& datestring, std::string const& format)
{
	m_useAsSearchTimerFrom = GetDateFromDatePicker(datestring, format);
}

void SearchTimer::SetUseAsSearchTimerTil(std::string const& datestring, std::string const& format)
{
	m_useAsSearchTimerTil = GetDateFromDatePicker(datestring, format);
}


SearchTimers::SearchTimers()
{
	Reload();
}

bool SearchTimers::Reload()
{
	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

#if VDRVERSNUM >= 20301
	LOCK_CHANNELS_READ;
#else
	ReadLock channelsLock( Channels, 0 );
#endif
	std::list<std::string> timers = service.handler->SearchTimerList();
	m_timers.assign( timers.begin(), timers.end() );
	m_timers.sort();
	return true;
}

bool SearchTimers::Save(SearchTimer* searchtimer)
{
	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	if (!searchtimer) return false;
#if VDRVERSNUM >= 20301
	LOCK_CHANNELS_READ;
#else
	ReadLock channelsLock( Channels, 0 );
#endif
	if (searchtimer->Id() >= 0)
		return service.handler->ModSearchTimer(searchtimer->ToText());
	else
	{
		searchtimer->SetId(0);
		int id = service.handler->AddSearchTimer(searchtimer->ToText());
		if (id >= 0)
			searchtimer->SetId(id);
		return (id >= 0);
	}
}

SearchTimer* SearchTimers::GetByTimerId( std::string const& id )
{
  for (SearchTimers::iterator timer = m_timers.begin(); timer != m_timers.end(); ++timer)
    if (timer->Id() == parse_int<int>(id)) return &*timer;
  return NULL;
}

bool SearchTimers::ToggleActive(std::string const& id)
{
	SearchTimer* search = GetByTimerId( id );
	if (!search) return false;
	search->SetUseAsSearchTimer(search->UseAsSearchTimer()==1?0:1);
	return Save(search);
}

bool SearchTimers::Delete(std::string const& id)
{
	SearchTimer* search = GetByTimerId( id );
	if (!search) return false;

	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	if (service.handler->DelSearchTimer(parse_int<int>( id )))
		return Reload();
	return false;
}

void SearchTimers::TriggerUpdate()
{
	Epgsearch_updatesearchtimers_v1_0 service;
	service.showMessage = true;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService("Epgsearch-updatesearchtimers-v1.0", &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );
}

bool SearchTimer::BlacklistSelected(int id) const
{
   for(unsigned int i=0; i<m_blacklistIDs.size(); i++)
      if (parse_int<int>(m_blacklistIDs[i]) == id) return true;
   return false;
}

ExtEPGInfo::ExtEPGInfo( std::string const& data )
{
  m_id = -1;
  m_searchmode = 0;

  std::vector<std::string> parts = StringSplit( data, '|' );
  std::vector<std::string>::const_iterator part = parts.begin();
  for ( int i = 0; part != parts.end(); ++i, ++part ) {
    switch ( i ) {
      case  0: m_id = parse_int<int>( *part ); break;
      case  1: m_name = *part; break;
      case  2: m_menuname = *part; break;
      case  3: ParseValues( *part ); break;
      case  4: m_searchmode = parse_int<int>( *part ); break;
     }
  }
}

void ExtEPGInfo::ParseValues( std::string const& data )
{
   m_values = StringSplit( data, ',' );
}

bool ExtEPGInfo::Selected(unsigned int index, std::string const& values)
{
   if (index >= m_values.size()) return false;
   std::string extepgvalue(StringTrim(m_values[index]));

   std::vector<std::string> parts;
   parts = StringSplit( values, ',' );
   for(unsigned int i=0; i<parts.size(); i++) if (StringTrim(parts[i]) == extepgvalue) return true;
   parts = StringSplit( values, ';' );
   for(unsigned int i=0; i<parts.size(); i++) if (StringTrim(parts[i]) == extepgvalue) return true;
   parts = StringSplit( values, '|' );
   for(unsigned int i=0; i<parts.size(); i++) if (StringTrim(parts[i]) == extepgvalue) return true;
   parts = StringSplit( values, '~' );
   for(unsigned int i=0; i<parts.size(); i++) if (StringTrim(parts[i]) == extepgvalue) return true;
   return false;
}

ExtEPGInfos::ExtEPGInfos()
{
	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	std::list<std::string> infos = service.handler->ExtEPGInfoList();
	m_infos.assign( infos.begin(), infos.end() );
}

ChannelGroup::ChannelGroup( std::string const& data )
{
   std::vector<std::string> parts = StringSplit( data, '|' );
   try {
      std::vector<std::string>::const_iterator part = parts.begin();
      for ( int i = 0; part != parts.end(); ++i, ++part ) {
         switch ( i ) {
			case  0: m_name = *part; break;
         }
      }
   } catch ( bad_lexical_cast const& ex ) {
   }
}

ChannelGroups::ChannelGroups()
{
	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	std::list<std::string> list = service.handler->ChanGrpList();
	m_list.assign( list.begin(), list.end() );
}

Blacklist::Blacklist( std::string const& data )
{
   std::vector<std::string> parts = StringSplit( data, ':' );
   try {
      std::vector<std::string>::const_iterator part = parts.begin();
      for ( int i = 0; part != parts.end(); ++i, ++part ) {
			switch ( i ) {
			case  0: m_id = parse_int<int>( *part ); break;
			case  1: m_search = StringReplace( StringReplace( *part, "|", ":" ), "!^pipe^!", "|" ); break;
			}
		}
	} catch ( bad_lexical_cast const& ex ) {
	}
}

Blacklists::Blacklists()
{
	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	std::list<std::string> list = service.handler->BlackList();
	m_list.assign( list.begin(), list.end() );
    m_list.sort();
}

SearchResult::SearchResult( std::string const& data )
{
	std::vector<std::string> parts = StringSplit( data, ':' );
	try {
		std::vector<std::string>::const_iterator part = parts.begin();
		for ( int i = 0; part != parts.end(); ++i, ++part ) {
			switch ( i ) {
			case  0: m_searchId = parse_int<int>( *part ); break;
			case  1: m_eventId = parse_int<tEventID>( *part ); break;
			case  2: m_title = StringReplace( *part, "|", ":" ); break;
			case  3: m_shorttext = StringReplace( *part, "|", ":" ); break;
			case  4: m_description = StringReplace( *part, "|", ":" ); break;
			case  5: m_starttime = parse_int<time_t>( *part ); break;
			case  6: m_stoptime = parse_int<time_t>( *part ); break;
			case  7: m_channel = tChannelID::FromString( part->c_str() ); break;
			case  8: m_timerstart = parse_int<time_t>( *part ); break;
			case  9: m_timerstop = parse_int<time_t>( *part ); break;
			case 10: m_file = *part; break;
			case 11: m_timerMode = parse_int<int>( *part ); break;
			}
		}
	} catch ( bad_lexical_cast const& ex ) {
	}
}

const cEvent* SearchResult::GetEvent(const cChannel* Channel)
{
	if (!Channel) return NULL;

#if VDRVERSNUM >= 20301
	LOCK_SCHEDULES_READ;
#else
	cSchedulesLock schedulesLock;
	const cSchedules* Schedules = cSchedules::Schedules(schedulesLock);
	if (!Schedules) return NULL;
#endif

	const cSchedule *Schedule = Schedules->GetSchedule(Channel);
	if (!Schedule) return NULL;
	return Schedule->GetEvent(m_eventId);
}

std::set<std::string> SearchResults::querySet;

void SearchResults::GetByID(int id)
{
	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	std::list<std::string> list = service.handler->QuerySearchTimer(id);
	m_list.assign( list.begin(), list.end() );
    m_list.sort();
}

void SearchResults::GetByQuery(std::string const& query)
{
	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	std::list<std::string> list = service.handler->QuerySearch(query);
	m_list.assign( list.begin(), list.end() );
	m_list.sort();
}

std::string SearchResults::AddQuery(std::string const& query)
{
	querySet.insert(query);
	return std::string(cToSvXxHash128(query));
}

std::string SearchResults::PopQuery(cSv md5)
{
	if (md5.empty()) return std::string();
	std::string query;
  std::set<std::string>::iterator it;
  for (it = querySet.begin(); it != querySet.end(); it++)
  {
    if(md5 == cSv(cToSvXxHash128(*it)))
    {
      query = *it;
      querySet.erase(it);
      break;
    }
  }
	return query;
}

RecordingDirs::RecordingDirs(bool shortList)
{
	if (shortList)
    {
		Epgsearch_services_v1_2 service;
		if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
			throw HtmlError( tr("EPGSearch version outdated! Please update.") );

		m_set = service.handler->ShortDirectoryList();
    }
	else
    {
		Epgsearch_services_v1_0 service;
		if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
			throw HtmlError( tr("EPGSearch version outdated! Please update.") );

		m_set = service.handler->DirectoryList();
    }
}

std::string EPGSearchSetupValues::ReadValue(const std::string& entry)
{
	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	return service.handler->ReadSetupValue(entry);
}

bool EPGSearchSetupValues::WriteValue(const std::string& entry, const std::string& value)
{
	Epgsearch_services_v1_0 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	return service.handler->WriteSetupValue(entry, value);
}

std::string EPGSearchExpr::EvaluateExpr(const std::string& expr, const cEvent* event)
{
	Epgsearch_services_v1_2 service;
	if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
		throw HtmlError( tr("EPGSearch version outdated! Please update.") );

	return service.handler->Evaluate(expr, event);
}


} // namespace vdrlive
