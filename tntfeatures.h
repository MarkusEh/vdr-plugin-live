#ifndef VDR_LIVE_TNTFEATURES_H
#define VDR_LIVE_TNTFEATURES_H

// This header mapps tntversion strings, whose 'structure' changes over time,
// to features of tntnet used in the live plugin. This avoids scattering the
// version check for TNTVERSION over several source files in live. Thus when
// an other change in the structure of the version string was needed then only
// this file needs to be adapted.

// SSL-Support works from tntnet version 1.6.1 onwards.
#define TNT_SSL_SUPPORT			(TNTVERSION >= 16100)

// Configuration of tntnet from within the source code and not with a
// dedicated config file.
#define TNT_CONFIG_INTERNAL		(TNTVERSION >= 16060)

// Query params are now in tntnet and not in cxxtools
#define TNT_HAS_QUERYPARAMS		(TNTVERSION >= 16060)

// One can request the host part of the request url
#define TNT_HAS_GETHOST			(TNTVERSION >= 16060)

// new version of TNTNET allow the request watchdog to be silenced.
#define TNT_WATCHDOG_SILENCE	(TNTVERSION >= 16900)

// version of TNTNET that binds ipv6 addresses with IPV6_V6ONLY flag set to true
#define TNT_IPV6_V6ONLY	  	    (CXXTOOLVER >= 21000)

#endif // VDR_LIVE_TNTFEATURES_H
