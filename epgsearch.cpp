
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
  cSplit parts(data, ':');
  auto part = parts.begin();
  for (int i = 0; part != parts.end(); ++i, ++part ) {
    switch ( i ) {
    case  0: m_id = parse_int<int>( *part ); break;
    case  1: m_search = cToSvReplace( *part, "|", ":" ).replaceAll("!^pipe^!", "|" ); break;
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
    case 19: m_directory = cToSvReplace( *part, "|", ":" ).replaceAll("!^pipe^!", "|" ); break;
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
}

std::string SearchTimer::ToText() {
  cToSvConcat os;
  os << m_id << ':'
     << cToSvReplace(m_search, "|", "!^pipe^!").replaceAll(":", "|")  << ':';
  if (m_useTime) {
    os << "1:";
    os.appendInt<4>(m_startTime) << ':';
    os.appendInt<4>(m_stopTime) << ':';
  } else {
    os << "0:::";
  }
  os << m_useChannel << ':';
  if (m_useChannel==1) {
    LOCK_CHANNELS_READ;
    cChannel const* channelMin = Channels->GetByChannelID( m_channelMin );
    cChannel const* channelMax = Channels->GetByChannelID( m_channelMax );

    if (channelMax && channelMin->Number() < channelMax->Number())
      os << m_channelMin << '|' << m_channelMax;
    else
      os << m_channelMin;
  } else if (m_useChannel==2) {
    os << m_channels;
  } else {
    os << "0";
  }
  os<< ':';
  os<< m_useCase << ':'
    << m_mode << ':'
    << m_useTitle << ':'
    << m_useSubtitle << ':'
    << m_useDescription << ':';
  if (m_useDuration) {
    os << "1:";
    os.appendInt<4>(m_minDuration) << ':';
    os.appendInt<4>(m_maxDuration) << ':';
  } else {
    os << "0:::";
  }
  os<< m_useAsSearchtimer << ':'
    << m_useDayOfWeek << ':'
    << m_dayOfWeek << ':'
    << m_useEpisode << ':'
    << cToSvReplace(m_directory, "|", "!^pipe^!").replaceAll(":", "|") << ':'
    << m_priority << ':'
    << m_lifetime << ':'
    << m_marginstart << ':'
    << m_marginstop << ':'
    << m_useVPS << ':'
    << m_action << ':'
    << m_useExtEPGInfo << ':';
  if (m_useExtEPGInfo) {
    for(unsigned int i=0; i<m_ExtEPGInfo.size(); i++) {
      if (i > 0) os <<  '|';
      os << cToSvReplace(m_ExtEPGInfo[i], ":", "!^colon^!").replaceAll("|", "!^pipe^!");
    }
  }
  os << ':';
  os<< m_avoidrepeats << ':'
    << m_allowedrepeats << ':'
    << m_compareTitle << ':'
    << m_compareSubtitle << ':'
    << m_compareSummary << ':'
    << m_catvaluesAvoidRepeat << ':'
    << m_repeatsWithinDays << ':'
    << m_delAfterDays << ':'
    << m_recordingsKeep << ':'
    << m_switchMinBefore << ':'
    << m_pauseOnNrRecordings << ':'
    << m_blacklistmode << ':';
  if (m_blacklistmode == 1) {
    for (unsigned int i=0; i<m_blacklistIDs.size(); i++) {
      if (i > 0) os <<  '|';
      os << m_blacklistIDs[i];
    }
  }
  os << ':';
  os<< m_fuzzytolerance << ':'
    << m_useInFavorites << ':'
    << m_menuTemplate << ':'
    << m_delMode << ':'
    << m_delAfterCountRecs << ':'
    << m_delAfterDaysOfFirstRec << ':'
    << m_useAsSearchTimerFrom << ':'
    << m_useAsSearchTimerTil << ':'
    << m_ignoreMissingEPGCats;

  return std::string(cSv(os));
}

void SearchTimer::ParseChannel(cSv data)
{
  switch ( m_useChannel ) {
    case NoChannel: m_channels = tr("All"); break;
    case Interval: ParseChannelIDs( data ); break;
    case Group: m_channels = std::string(data); break;
    case FTAOnly: m_channels = tr("FTA"); break;
  }
}

void SearchTimer::ParseChannelIDs(cSv data)
{
  cSplit parts(data, '|');
  auto part = parts.begin();
  m_channelMin = lexical_cast<tChannelID>(*part);

  LOCK_CHANNELS_READ;
  const cChannel *channel = Channels->GetByChannelID( m_channelMin );
  if (channel)
    m_channels = channel->Name();

  if (++part == parts.end()) return;
  m_channelMax = lexical_cast<tChannelID>(*part);

  channel = Channels->GetByChannelID( m_channelMax );
  if (channel)
    m_channels += cSv(cToSvConcat(" - ", channel->Name()));
}

void SearchTimer::ParseExtEPGInfo(cSv data)
{
   m_ExtEPGInfo = std::vector<std::string>(cSplit<std::string>(data, '|').begin(), cSplit<std::string>::s_end());
}

void SearchTimer::ParseBlacklist(cSv data)
{
   m_blacklistIDs = std::vector<std::string>(cSplit<std::string>(data, '|').begin(), cSplit<std::string>::s_end());
}

std::string SearchTimer::StartTimeFormatted()
{
  time_t start = cTimer::SetTime(time(NULL), (((StartTime() / 100 ) % 100) * 60 * 60) + (StartTime() % 100 * 60));
  return std::string(cToSvDateTime(tr("%I:%M %p"), start));
}

std::string SearchTimer::StopTimeFormatted()
{
  time_t stop = cTimer::SetTime(time(NULL), (((StopTime() / 100 ) % 100) * 60 * 60) + (StopTime() % 100 * 60));
  return std::string(cToSvDateTime(tr("%I:%M %p"), stop));
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

  std::list<std::string> timers = service.handler->SearchTimerList();
  m_timers.assign( timers.begin(), timers.end() );
  std::sort(m_timers.begin(), m_timers.end());
//  m_timers.sort();
  return true;
}

bool SearchTimers::Save(SearchTimer* searchtimer)
{
  if (!searchtimer) return false;
  Epgsearch_services_v1_0 service;
  if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
    throw HtmlError( tr("EPGSearch version outdated! Please update.") );

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

  cSplit parts(data, '|');
  auto part = parts.begin();
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

void ExtEPGInfo::ParseValues(cSv data)
{
   m_values = std::vector<std::string>(cSplit<std::string>(data, ',').begin(), cSplit<std::string>::s_end());
}

bool ExtEPGInfo::Selected(unsigned int index, cSv values)
{
   if (index >= m_values.size()) return false;
   std::string extepgvalue(StringTrim(m_values[index]));

   for (cSv part: cSplit(values, ',')) if (StringTrim(part) == extepgvalue) return true;
   for (cSv part: cSplit(values, ';')) if (StringTrim(part) == extepgvalue) return true;
   for (cSv part: cSplit(values, '|')) if (StringTrim(part) == extepgvalue) return true;
   for (cSv part: cSplit(values, '~')) if (StringTrim(part) == extepgvalue) return true;

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
  cSplit parts(data, '|');
  auto part = parts.begin();
  for (int i = 0; part != parts.end(); ++i, ++part) {
    switch ( i ) {
      case  0: m_name = *part; break;
    }
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
  cSplit parts(data, ':');
  auto part = parts.begin();
  for ( int i = 0; part != parts.end(); ++i, ++part ) {
    switch ( i ) {
      case  0: m_id = parse_int<int>( *part ); break;
      case  1: m_search = cToSvReplace( *part, "|", ":" ).replaceAll("!^pipe^!", "|" ); break;
    }
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
  cSplit parts(data, ':');
  auto part = parts.begin();
  for ( int i = 0; part != parts.end(); ++i, ++part ) {
    switch ( i ) {
      case  0: m_searchId = parse_int<int>( *part ); break;
      case  1: m_eventId = parse_int<tEventID>( *part ); break;
      case  2: m_title = cToSvReplace( *part, "|", ":" ); break;
      case  3: m_shorttext = cToSvReplace( *part, "|", ":" ); break;
      case  4: m_description = cToSvReplace( *part, "|", ":" ); break;
      case  5: m_starttime = parse_int<time_t>( *part ); break;
      case  6: m_stoptime = parse_int<time_t>( *part ); break;
      case  7: m_channel = lexical_cast<tChannelID>(*part); break;
      case  8: m_timerstart = parse_int<time_t>( *part ); break;
      case  9: m_timerstop = parse_int<time_t>( *part ); break;
      case 10: m_file = *part; break;
      case 11: m_timerMode = parse_int<int>( *part ); break;
    }
  }
}

const cEvent* SearchResult::GetEvent(const cChannel* Channel, const cSchedules *Schedules)
{
  if (!Channel) return nullptr;
  const cSchedule *Schedule = Schedules->GetSchedule(Channel);
  if (!Schedule) return nullptr;
#if APIVERSNUM >= 20502
  return Schedule->GetEventById(m_eventId);
#else
  return Schedule->GetEvent(m_eventId);
#endif
}

std::vector<cQueryEntry> SearchResults::queryList;

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

std::string SearchResults::AddQuery(cSv query)
{
  for (auto it = queryList.begin(); it != queryList.end(); ++it) if (it->Value() == query) {
    it->Used();
    return std::string(cToSvXxHash128(query));
  }
  queryList.emplace_back(query);
  return std::string(cToSvXxHash128(query));
}

std::string SearchResults::GetQuery(cSv md5)
{
  if (md5.empty()) return std::string();
  std::string query;
  for (auto it = queryList.begin(); it != queryList.end(); ++it)
  {
    if(md5 == cSv(cToSvXxHash128(it->Value() )))
    {
      query = it->Value() ;
      it->Used();
      break;
    }
  }
  return query;
}

void SearchResults::CleanQuery() {
  time_t now = time(NULL);
  size_t old_s = queryList.size();
  for (auto it = queryList.begin(); it != queryList.end();)
  {
    if(it->IsOutdated(now))
      it = queryList.erase(it);
    else
      ++it;
  }
  if (old_s != queryList.size() ) {
    size_t mem = 0;
    for (auto it = queryList.begin(); it != queryList.end(); ++it) mem += it->Value().length();

    dsyslog("live, cleanup queryList, size was %zu, is %zu, requ. mem %zu", old_s, queryList.size(), mem);
  }
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
