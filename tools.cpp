#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <tnt/ecpp.h>
#include <tnt/htmlescostream.h>
#include <tnt/httprequest.h>
#include <tnt/httpreply.h>
#include <vdr/recording.h>
#include "exception.h"
#include "live.h"
#include "setup.h"
#include "tools.h"
#include "md5.h"


using namespace std;
using namespace tnt;

istream& operator>>( istream& is, tChannelID& ret )
{
  /* alternativ implementation
  string line;
  if ( !getline( is, line ) ) {
    if ( !is.eof() )
      is.setstate( ios::badbit );
    else
      is.clear();
    return is;
  }
  if ( !line.empty() && !( ret = tChannelID::FromString( line.c_str() ) ).Valid() )
    is.setstate( ios::badbit );
  return is;
  */

  string line;
  if (!getline( is, line ) ) {
    if (0 == is.gcount()) {
      is.clear(is.rdstate() & ~ios::failbit);
      return is;
    }
    if (!is.eof()) {
      is.setstate( ios::badbit );
      return is;
    }
  }

  if ( !line.empty() && !( ret = tChannelID::FromString( line.c_str() ) ).Valid() )
    is.setstate( ios::badbit );
  return is;
}

namespace vdrlive {

	string FormatDuration( char const* format, int hours, int minutes )
	{
		char result[ 32 ];
		if ( snprintf(result, sizeof(result), format, hours, minutes) < 0 ) {
			ostringstream builder;
			builder << "cannot represent duration " << hours << ":" << minutes << " as requested";
			throw runtime_error( builder.str() );
		}
		return result;
	}

	string FormatDateTime( char const* format, time_t time )
	{
		struct tm tm_r;
		if ( localtime_r( &time, &tm_r ) == 0 ) {
			ostringstream builder;
			builder << "cannot represent timestamp " << time << " as local time";
			throw runtime_error( builder.str() );
		}

		char result[ 256 ];
		if ( strftime( result, sizeof( result ), format, &tm_r ) == 0 ) {
			ostringstream builder;
			builder << "representation of timestamp " << time << " exceeds " << sizeof( result ) << " bytes";
			throw runtime_error( builder.str() );
		}
		return result;
	}

	string StringReplace( string const& text, string const& substring, string const& replacement )
	{
		string result = text;
		string::size_type pos = 0;
		while ( ( pos = result.find( substring, pos ) ) != string::npos ) {
			result.replace( pos, substring.length(), replacement );
			pos += replacement.length();
		}
		return result;
	}

	vector< string > StringSplit( string const& text, char delimiter )
	{
		vector< string > result;
		string::size_type last = 0, pos;
		while ( ( pos = text.find( delimiter, last ) ) != string::npos ) {
			result.push_back( text.substr( last, pos - last ) );
			last = pos + 1;
		}
		if ( last < text.length() )
			result.push_back( text.substr( last ) );
		return result;
	}

	int StringToInt( std::string const& string, int base )
	{
		char* end;
		int result = strtol( string.c_str(), &end, base );
		if ( *end == '\0' )
			return result;
		return 0;
	}

	string StringRepeat(int times, const string& input)
	{
		string result;
		for (int i = 0; i < times; i++) {
			result += input;
		}
		return result;
	}

	string StringWordTruncate(const string& input, size_t maxLen, bool& truncated)
	{
		if (input.length() <= maxLen)
		{
                        truncated = false;
			return input;
		}
		truncated = true;
		string result = input.substr(0, maxLen);
		size_t pos = result.find_last_of(" \t,;:.\n?!'\"/\\()[]{}*+-");
		return result.substr(0, pos);
	}

	string StringFormatBreak(string const& input)
	{
		return StringReplace( input, "\n", "<br/>" );
	}

	string StringEscapeAndBreak( string const& input )
	{
		stringstream plainBuilder;
		HtmlEscOstream builder( plainBuilder );
		builder << input;
		return StringReplace( plainBuilder.str(), "\n", "<br/>" );
	}

	string StringTrim(string const& str)
	{
		string res = str;
		string::size_type pos = res.find_last_not_of(' ');
		if(pos != string::npos) {
			res.erase(pos + 1);
			pos = res.find_first_not_of(' ');
			if(pos != string::npos) res.erase(0, pos);
		}
		else res.erase(res.begin(), res.end());
		return res;
	}

	string ZeroPad(int number)
	{
		ostringstream os;
		os << setw(2) << setfill('0') << number;
		return os.str();
	}

	std::string MD5Hash(std::string const& str)
	{
		char* szInput = strdup(str.c_str());
		if (!szInput) return "";
		char* szRes = MD5String(szInput);
		string res = szRes;
		free(szRes);
		return res;

/*	unsigned char md5[MD5_DIGEST_LENGTH];
	MD5(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), md5);

	ostringstream hashStr;
	hashStr << hex;
	for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++)
	hashStr << (0 + md5[i]);

	return hashStr.str();
*/
	}

#define HOURS(x) ((x)/100)
#define MINUTES(x) ((x)%100)

	std::string ExpandTimeString(std::string timestring)
	{
		string::size_type colonpos = timestring.find(":");
		if (colonpos == string::npos)
		{
			if (timestring.size() == 1)
				timestring = "0" + timestring + ":00";
			else if (timestring.size() == 2)
				timestring = timestring + ":00";
			else if (timestring.size() == 3)
				timestring = "0" + string(timestring.begin(), timestring.begin() + 1) + ":" + string(timestring.begin() + 1, timestring.end());
			else
				timestring = string(timestring.begin(), timestring.begin() + 2) + ":" + string(timestring.begin() + 2, timestring.end());
		}
		else
		{
			string hours = string(timestring.begin(), timestring.begin() + colonpos);
			string mins = string(timestring.begin() + colonpos + 1, timestring.end());
			hours = string(std::max(0,(int)(2 - hours.size())), '0') + hours;
			mins = string(std::max(0,(int)(2 - mins.size())), '0') + mins;
			timestring = hours + ":" + mins;
		}
		return timestring;
	}

	time_t GetTimeT(std::string timestring) // timestring in HH:MM
	{
		timestring = StringReplace(timestring, ":", "");
		int iTime = lexical_cast< int >( timestring );
		struct tm tm_r;
		time_t t = time(NULL);
		tm* tmnow = localtime_r(&t, &tm_r);
		tmnow->tm_hour = HOURS(iTime);
		tmnow->tm_min = MINUTES(iTime);
		return mktime(tmnow);
	}

	struct urlencoder
	{
			ostream& ostr_;

			urlencoder( ostream& ostr ): ostr_( ostr ) {}

			void operator()( char ch )
			{
				static const char allowedChars[] = {
					//	  0 ,  1 ,  2 ,  3 ,  4 ,  5 ,  6 ,  7 ,  8 ,  9 ,  A ,  B ,  C ,  D ,  E ,  F ,
					'_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',	//00
					'_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',	//10
					'_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', 0x2D,0x2E,0x2F,	//20
					0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,'_', '_', '_', '_', '_', '_',	//30
					'_', 0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,	//40
					0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,'_', '_', '_', '_', '_',	//50
					'_', 0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,	//60
					0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,'_', '_', '_', '_', '_' 	//70
					// everything above 127 (for signed char, below zero) is replaced with '_'
				};

				if ( ch == ' ' )
					ostr_ << '+';
				else if ( static_cast< signed char >( ch ) < 0 || allowedChars[ size_t( ch ) ] == '_' )
					ostr_ << '%' << setw( 2 ) << setfill( '0' ) << hex << int( static_cast< unsigned char >( ch ) );
				else
					ostr_ << ch;
			}
	};

	string StringUrlEncode( string const& input )
	{
		ostringstream ostr;
		for_each( input.begin(), input.end(), urlencoder( ostr ) );
		return ostr.str();
	}

// returns the content of <element>...</element>
	string GetXMLValue( std::string const& xml, std::string const& element )
	{
		string start = "<" + element + ">";
		string end = "</" + element + ">";
		string::size_type startPos = xml.find(start);
		if (startPos == string::npos) return "";
		string::size_type endPos = xml.find(end);
		if (endPos == string::npos) return "";
		return xml.substr(startPos + start.size(), endPos - startPos - start.size());
	}

// return the time value as time_t from <datestring> formatted with <format>
	time_t GetDateFromDatePicker(std::string const& datestring, std::string const& format)
	{
		if (datestring.empty())
			return 0;
		int year = lexical_cast< int >(datestring.substr(format.find("yyyy"), 4));
		int month = lexical_cast< int >(datestring.substr(format.find("mm"), 2));
		int day = lexical_cast< int >(datestring.substr(format.find("dd"), 2));
		struct tm tm_r;
		tm_r.tm_year = year - 1900;
		tm_r.tm_mon = month -1;
		tm_r.tm_mday = day;
		tm_r.tm_hour = tm_r.tm_min = tm_r.tm_sec = 0;
		tm_r.tm_isdst = -1; // makes sure mktime() will determine the correct DST setting
		return mktime(&tm_r);
	}

// format is in datepicker format ('mm' for month, 'dd' for day, 'yyyy' for year)
	std::string DatePickerToC(time_t date, std::string const& format)
	{
		if (date == 0) return "";
		std::string cformat = format;
		cformat = StringReplace(cformat, "mm", "%m");
		cformat = StringReplace(cformat, "dd", "%d");
		cformat = StringReplace(cformat, "yyyy", "%Y");
		return FormatDateTime(cformat.c_str(), date);
	}

	std::string EncodeDomId(std::string const & toEncode, char const * from, char const * to)
	{
		std::string encoded = toEncode;
		for (; *from && *to; from++, to++) {
			replace(encoded.begin(), encoded.end(), *from, *to);
		}
		return encoded;
	}

	std::string DecodeDomId(std::string const & toDecode, char const * from, char const * to)
	{
		std::string decoded = toDecode;
		for (; *from && *to; from++, to++) {
			replace(decoded.begin(), decoded.end(), *from, *to);
		}
		return decoded;
	}

	std::string FileSystemExchangeChars(std::string const & s, bool ToFileSystem)
	{
		char *str = strdup(s.c_str());
		str = ExchangeChars(str, ToFileSystem);
		std::string data = str;
		if (str) {
			free(str);
		}
		return data;
	}

	bool MoveDirectory(std::string const & sourceDir, std::string const & targetDir, bool copy)
	{
		const char* delim = "/";
		std::string source = sourceDir;
		std::string target = targetDir;

		// add missing directory delimiters
		if (source.compare(source.size() - 1, 1, delim) != 0) {
			source += "/";
		}
		if (target.compare(target.size() - 1, 1, delim) != 0) {
			target += "/";
		}

		if (source != target) {
			// validate target directory
			if (target.find(source) != std::string::npos) {
				esyslog("[LIVE]: cannot move under sub-directory\n");
				return false;
			}
			RemoveFileOrDir(target.c_str());
			if (!MakeDirs(target.c_str(), true)) {
				esyslog("[LIVE]: cannot create directory %s", target.c_str());
				return false;
			}

			struct stat st1, st2;
			stat(source.c_str(), &st1);
			stat(target.c_str(),&st2);
			if (!copy && (st1.st_dev == st2.st_dev)) {
#if APIVERSNUM > 20101
				if (!cVideoDirectory::RenameVideoFile(source.c_str(), target.c_str())) {
#else
				if (!RenameVideoFile(source.c_str(), target.c_str())) {
#endif
					esyslog("[LIVE]: rename failed from %s to %s", source.c_str(), target.c_str());
					return false;
				}
			}
			else {
				int required = DirSizeMB(source.c_str());
				int available = FreeDiskSpaceMB(target.c_str());

				// validate free space
				if (required < available) {
					cReadDir d(source.c_str());
					struct dirent *e;
					bool success = true;

					// allocate copying buffer
					const int len = 1024 * 1024;
					char *buffer = MALLOC(char, len);
					if (!buffer) {
						esyslog("[LIVE]: cannot allocate renaming buffer");
						return false;
					}

					// loop through all files, but skip all subdirectories
					while ((e = d.Next()) != NULL) {
						// skip generic entries
						if (strcmp(e->d_name, ".") && strcmp(e->d_name, "..") && strcmp(e->d_name, "lost+found")) {
							string sourceFile = source + e->d_name;
							string targetFile = target + e->d_name;

							// copy only regular files
							if (!stat(sourceFile.c_str(), &st1) && S_ISREG(st1.st_mode)) {
								int r = -1, w = -1;
								cUnbufferedFile *inputFile = cUnbufferedFile::Create(sourceFile.c_str(), O_RDONLY | O_LARGEFILE);
								cUnbufferedFile *outputFile = cUnbufferedFile::Create(targetFile.c_str(), O_RDWR | O_CREAT | O_LARGEFILE);

								// validate files
								if (!inputFile || !outputFile) {
									esyslog("[LIVE]: cannot open file %s or %s", sourceFile.c_str(), targetFile.c_str());
									success = false;
									break;
								}

								// do actual copy
								dsyslog("[LIVE]: copying %s to %s", sourceFile.c_str(), targetFile.c_str());
								do {
									r = inputFile->Read(buffer, len);
									if (r > 0)
										w = outputFile->Write(buffer, r);
									else
										w = 0;
								} while (r > 0 && w > 0);
								DELETENULL(inputFile);
								DELETENULL(outputFile);

								// validate result
								if (r < 0 || w < 0) {
									success = false;
									break;
								}
							}
						}
					}

					// release allocated buffer
					free(buffer);

					// delete all created target files and directories
					if (!success) {
						size_t found = target.find_last_of(delim);
						if (found != std::string::npos) {
							target = target.substr(0, found);
						}
						if (!RemoveFileOrDir(target.c_str(), true)) {
							esyslog("[LIVE]: cannot remove target %s", target.c_str());
						}
						found = target.find_last_of(delim);
						if (found != std::string::npos) {
							target = target.substr(0, found);
						}
						if (!RemoveEmptyDirectories(target.c_str(), true)) {
							esyslog("[LIVE]: cannot remove target directory %s", target.c_str());
						}
						esyslog("[LIVE]: copying failed");
						return false;
					}
					else if (!copy && !RemoveFileOrDir(source.c_str(), true)) { // delete source files
						esyslog("[LIVE]: cannot remove source directory %s", source.c_str());
						return false;
					}

					// delete all empty source directories
					if (!copy) {
						size_t found = source.find_last_of(delim);
						if (found != std::string::npos) {
							source = source.substr(0, found);
#if APIVERSNUM > 20101
							while (source != cVideoDirectory::Name()) {
#else
							while (source != VideoDirectory) {
#endif
								found = source.find_last_of(delim);
								if (found == std::string::npos)
									break;
								source = source.substr(0, found);
								if (!RemoveEmptyDirectories(source.c_str(), true))
									break;
							}
						}
					}
				}
				else {
					esyslog("[LIVE]: %s requires %dMB - only %dMB available", copy ? "moving" : "copying", required, available);
					// delete all created empty target directories
					size_t found = target.find_last_of(delim);
					if (found != std::string::npos) {
						target = target.substr(0, found);
#if APIVERSNUM > 20101
						while (target != cVideoDirectory::Name()) {
#else
						while (target != VideoDirectory) {
#endif
							found = target.find_last_of(delim);
							if (found == std::string::npos)
								break;
							target = target.substr(0, found);
							if (!RemoveEmptyDirectories(target.c_str(), true))
								break;
						}
					}
					return false;
				}
			}
		}

		return true;
	}

} // namespace vdrlive
