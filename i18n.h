#ifndef VDR_LIVE_I18N_H
#define VDR_LIVE_I18N_H

#include <vdr/config.h>
#include <vdr/i18n.h>

namespace vdrlive {

class I18n
{
	friend I18n& LiveI18n();

	char const* m_encoding; 

	I18n( I18n const& ); // don't copy
	I18n() : m_encoding( 
#if VDRVERSNUM >= 10503
		cCharSetConv::SystemCharacterTable() ? cCharSetConv::SystemCharacterTable() : "UTF-8"
#else
		I18nCharSets()[::Setup.OSDLanguage]
#endif
		) {}

public:
	char const* CharacterEncoding() const { return m_encoding; }
};

I18n& LiveI18n();

extern const tI18nPhrase Phrases[];

} // namespace vdrlive

#endif // VDR_LIVE_I18N_H
