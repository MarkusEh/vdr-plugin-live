#ifndef VDR_LIVE_TNTFEATURES_H
#define VDR_LIVE_TNTFEATURES_H

// This header mapps tntversion strings, whose 'structure' changes over time,
// to features of tntnet used in the live plugin. This avoids scattering the
// version check for TNTVERSION over several source files in live. Thus when
// an other change in the structure of the version string was needed then only
// this file needs to be adapted.

// Query params without boolean parameter
#define TNT_QUERYPARAMS_NO_BOOL (TNTVERSION >= 22000)

// new version of TNTNET allow the request watchdog to be silenced.
#define TNT_WATCHDOG_SILENCE	(TNTVERSION >= 16900)

// version of TNTNET that binds ipv6 addresses with IPV6_V6ONLY flag set to true
#define TNT_IPV6_V6ONLY	  	    (CXXTOOLVER >= 21000)

// version of TNTNET with properties deserializer for logger configuration args.
#define TNT_LOG_SERINFO			(CXXTOOLVER >= 22000)

// version of TNTNET wich expects name, value mappings for Url-Mapper arguments.
#define TNT_MAPURL_NAMED_ARGS	(TNTVERSION >= 22000)

// version of TNTNET where configuration is global
#define TNT_GLOBAL_TNTCONFIG	(TNTVERSION >= 22000)

#endif // VDR_LIVE_TNTFEATURES_H
