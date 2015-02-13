#ifndef VDR_LIVE_I18N_H
#define VDR_LIVE_I18N_H

#include <string>
#include <vdr/config.h>
#include <vdr/i18n.h>

namespace vdrlive {

class I18n
{
	friend I18n& LiveI18n();

	private:
		std::string m_encoding;

		I18n( I18n const& ); // don't copy
		I18n();

	public:
		std::string const& CharacterEncoding() const { return m_encoding; }
};

I18n& LiveI18n();

} // namespace vdrlive

#endif // VDR_LIVE_I18N_H
