#include <vector>
#include "timerconflict.h"
#include "tools.h"
#include "epgsearch/services.h"
#include "exception.h"
#include "epgsearch.h"
#include <vdr/plugin.h>

namespace vdrlive {

	bool CheckEpgsearchVersion();

	using namespace std;

	static char ServiceInterface[] = "Epgsearch-services-v1.0";

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

	TimerConflict::TimerConflict( string const& data )
	{
		Init();
		vector< string > parts = StringSplit( data, ':' );
		try {
			vector< string >::const_iterator part = parts.begin();
			if (parts.size() > 0) {
				conflictTime = lexical_cast< time_t >( *part++ );
				for ( int i = 1; part != parts.end(); ++i, ++part ) {
					vector< string > timerparts = StringSplit( *part, '|' );
					vector< string >::const_iterator timerpart = timerparts.begin();
					TimerInConflict timer;
					for ( int j = 0; timerpart != timerparts.end(); ++j, ++timerpart )
						switch (j) {
						case 0: timer.timerIndex = lexical_cast< int >( *timerpart ); break;
						case 1: timer.percentage = lexical_cast< int >( *timerpart ); break;
						case 2: {
							vector< string > conctimerparts = StringSplit( *timerpart, '#' );
							vector< string >::const_iterator conctimerpart = conctimerparts.begin();
							for ( int k = 0; conctimerpart != conctimerparts.end(); ++k, ++conctimerpart )
								timer.concurrentTimerIndices.push_back(lexical_cast< int >( *conctimerpart ));
							break;
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
		Epgsearch_services_v1_0 service;
		if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
			throw HtmlError( tr("EPGSearch version outdated! Please update.") );

		list< string > conflicts = service.handler->TimerConflictList();
		m_conflicts.assign( conflicts.begin(), conflicts.end() );
		m_conflicts.sort();
	}

	bool TimerConflicts::CheckAdvised()
	{
		Epgsearch_services_v1_0 service;
		if ( !CheckEpgsearchVersion() || cPluginManager::CallFirstService(ServiceInterface, &service) == 0 )
			throw HtmlError( tr("EPGSearch version outdated! Please update.") );
		return service.handler->IsConflictCheckAdvised();
	}

} // namespace vdrlive
