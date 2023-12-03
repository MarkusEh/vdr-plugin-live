
#include "timerconflict.h"

#include "tools.h"
#include "epgsearch/services.h"

// STL headers need to be before VDR tools.h (included by <vdr/plugin.h>)
#include <vector>

#include <vdr/timers.h>
#include <vdr/plugin.h>
#include <vdr/svdrp.h>

namespace vdrlive {

	bool CheckEpgsearchVersion();

	static char ServiceInterface[] = "Epgsearch-services-v1.1";

	bool operator<( TimerConflict const& left, TimerConflict const& right )
	{
		return left.conflictTime < right.conflictTime;
	}

	TimerConflict::TimerConflict()
	{
		Init();
	}

	void TimerConflict::Init()
	{
		conflictTime = 0;
	}

	TimerConflict::TimerConflict( std::string const& data )
	{
		Init();
//		dsyslog("live: TimerConflict() data '%s'", data.c_str());
		std::vector<std::string> parts = StringSplit( data, ':' );
		try {
			std::vector<std::string>::const_iterator part = parts.begin();
			if (parts.size() > 0) {
				conflictTime = parse_int<time_t>( *part++ );
				for ( int i = 1; part != parts.end(); ++i, ++part ) {
					std::vector<std::string> timerparts = StringSplit( *part, '|' );
					std::vector<std::string>::const_iterator timerpart = timerparts.begin();
					TimerInConflict timer;
					for ( int j = 0; timerpart != timerparts.end(); ++j, ++timerpart ) {
						switch (j) {
							case 0: timer.timerIndex = parse_int<int>( *timerpart ); break;
							case 1: timer.percentage = parse_int<int>( *timerpart ); break;
							case 2: {
								std::vector<std::string> conctimerparts = StringSplit( *timerpart, '#' );
								std::vector<std::string>::const_iterator conctimerpart = conctimerparts.begin();
								for ( int k = 0; conctimerpart != conctimerparts.end(); ++k, ++conctimerpart )
									timer.concurrentTimerIndices.push_back(parse_int<int>( *conctimerpart ));
								break;
							}
							case 3: {
								timer.remote = *timerpart;
								break;
							}
						}
					}
					conflictingTimers.push_back(timer);
				}
			}
		}
		catch ( bad_lexical_cast const& ex ) {
		}
	}

	TimerConflicts::TimerConflicts()
	{
		Epgsearch_services_v1_1 service;
		if ( CheckEpgsearchVersion() && cPluginManager::CallFirstService(ServiceInterface, &service))
		{
		    	cServiceHandler_v1_1* handler = dynamic_cast<cServiceHandler_v1_1*>(service.handler.get());
		    	if (handler)
		      	{
				std::list<std::string> conflicts = service.handler->TimerConflictList();
//				for(std::list<std::string>::const_iterator i = conflicts.begin(); i != conflicts.end(); ++i) {
//					dsyslog("live: TimerConflicts::TimerConflicts() conflicts '%s'",i->c_str());
//				}
				GetRemote(conflicts);			// add remote VDR conflicts
				m_conflicts.assign( conflicts.begin(), conflicts.end() );
				m_conflicts.sort();
					}
		}
//		for (TimerConflicts::iterator conflict=m_conflicts.begin(); conflict!=m_conflicts.end(); conflict++) {
//			const std::list<TimerInConflict>& conflTimers = conflict->ConflictingTimers();
//				for (std::list<TimerInConflict>::const_iterator confltimer = conflTimers.begin(); confltimer != conflTimers.end(); ++confltimer) {
//				dsyslog("live: TimerConflicts::TimerConflictsi() Timer ID with conflict '%d'", confltimer->timerIndex );
//				dsyslog("live: TimerConflicts::TimerConflictsi() conflict on server '%s'", confltimer->remote.c_str() );
//				for (std::list<int>::const_iterator timerIndex = confltimer->concurrentTimerIndices.begin(); timerIndex != confltimer->concurrentTimerIndices.end(); ++timerIndex) {
//					dsyslog("live: TimerConflicts::TimerConflicts() concurrent Timer IDs '%d'", *timerIndex);
//				}
//			}
//		}
	}


	void TimerConflicts::GetRemote(std::list<std::string> & conflicts )
	{
		cStringList svdrpServerNames;

		if (GetSVDRPServerNames(&svdrpServerNames)) {
			svdrpServerNames.Sort(true);
		}
		for (int i = 0; i < svdrpServerNames.Size(); i++) {
			std::string remoteServer = svdrpServerNames[i];
//			dsyslog("live: TimerConflicts::GetRemote() found remote server '%s'", remoteServer.c_str());
			cStringList response;
                        std::string command = "PLUG epgsearch lscc";
                        bool svdrpOK = ExecSVDRPCommand(remoteServer.c_str(), command.c_str(), &response);
                        if ( !svdrpOK ) {
                                esyslog("live: TimerConflicts::GetRemote() svdrp command '%s' on remote server '%s'failed", command.c_str(), remoteServer.c_str());
                        }
                        else {
                                for (int i = 0; i < response.Size(); i++) {
                                        int code = SVDRPCode(response[i]);
//					dsyslog("live: GetRemote() response[i] '%s'", response[i]);
					switch ( code ) {
                                        	case 900: {
							std::string rConflict = response[i];
							std::string remConflict = rConflict.substr(4);
							remConflict.append("|");
							remConflict.append(remoteServer);
//							dsyslog("live: TimerConflicts::GetRemote() found remote conflict '%s' ", remConflict.c_str());
							conflicts.push_back(remConflict);
							break;
						}
						case 901: break; // no conflict found
						default: {
							esyslog("live: TimerConflicts::GetRemote() svdrp command '%s' failed, respone: %s", command.c_str(), response[i]);
							svdrpOK = false;
							break;
						}
					}
				}
                                if ( svdrpOK ) {
//					dsyslog("live: TimerConflicts::GetRemote() on server '%s' successful", remoteServer.c_str());
				}
                                else {
					esyslog("live: TimerConflicts::GetRemote() on server '%s' failed", remoteServer.c_str());
				}
			}
			response.Clear();
		}
		svdrpServerNames.Clear();
	}

	bool TimerConflicts::HasConflict(int timerId)
	{
		for (const auto& conflict : *this)
			for (const auto& tic : conflict.ConflictingTimers())
				if (tic.timerIndex == timerId)
					return true;
		return false;
	}

	bool TimerConflicts::CheckAdvised()
	{
		Epgsearch_services_v1_1 service;
		if (CheckEpgsearchVersion() && cPluginManager::CallFirstService(ServiceInterface, &service))
		{
			cServiceHandler_v1_1* handler = dynamic_cast<cServiceHandler_v1_1*>(service.handler.get());
			if (!handler)
				return false;
			else
				return handler->IsConflictCheckAdvised();
		}
		else
		  return false;
	}

	TimerConflictNotifier::TimerConflictNotifier()
		: lastCheck(0)
		, lastTimerModification(0)
		, conflicts()
	{
	}

	TimerConflictNotifier::~TimerConflictNotifier()
	{
	}

	bool TimerConflictNotifier::ShouldNotify()
	{
		time_t now = time(0);
		bool reCheckAdvised((now - lastCheck) > CHECKINTERVAL);
		bool recentTimerChange((now - lastTimerModification) <= CHECKINTERVAL);

		if (recentTimerChange || (reCheckAdvised && TimerConflicts::CheckAdvised())) {
			lastCheck = now;
			conflicts.reset(new TimerConflicts());
			return conflicts->size() > 0;
		}
		return false;
	}

	void TimerConflictNotifier::SetTimerModification()
	{
		lastTimerModification = time(0);
	}

	std::string TimerConflictNotifier::Message() const
	{
		int count = conflicts ? conflicts->size() : 0;
		std::string msg = tr("Timer conflict check detected ");
		msg += ConvertToString(count) + " ";
		if (count == 1)
			msg += tr("conflict");
		else
			msg += tr("conflicts");

		return count > 0 ? msg + "!" : "";
	}

	std::string TimerConflictNotifier::Url() const
	{
		return "timerconflicts.html";
	}


} // namespace vdrlive
