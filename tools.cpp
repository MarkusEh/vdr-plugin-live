
#include "tools.h"
#include "xxhash32.h"
#include "setup.h"


#include "md5.h"

#include <tnt/ecpp.h>
#include <tnt/htmlescostream.h>

// STL headers need to be before VDR tools.h (included by <vdr/recording.h>)
#include <iomanip>

#include <vdr/plugin.h>
#include <vdr/recording.h>
#include <vdr/videodir.h>

using namespace tnt;

std::istream& operator>>( std::istream& is, tChannelID& ret )
{
  std::string line;
  if (!std::getline( is, line ) ) {
    if (0 == is.gcount()) {
      is.clear(is.rdstate() & ~std::ios::failbit);
      return is;
    }
    if (!is.eof()) {
      is.setstate( std::ios::badbit );
      return is;
    }
  }

  if ( !line.empty() && !( ret = tChannelID::FromString( line.c_str() ) ).Valid() )
    is.setstate( std::ios::badbit );
  return is;
}

namespace vdrlive {

        void AppendHtmlEscaped(std::string &target, const char* s){
// append c-string s to target, html escape some chsracters
          if(!s) return;
          size_t i = 0;
          const char* notAppended = s;
// moving forward, notAppended is the position of the first character which is not yet appended, in i the number of not yet appended chars
          for (const char* current = s; *current; current++) {
            switch(*current) {
              case '&':  target.append(notAppended, i); target.append("&amp;");  notAppended = notAppended + i + 1; i = 0;   break;
              case '\"': target.append(notAppended, i); target.append("&quot;"); notAppended = notAppended + i + 1; i = 0;   break;
              case '\'': target.append(notAppended, i); target.append("&apos;"); notAppended = notAppended + i + 1; i = 0;   break;
              case '<':  target.append(notAppended, i); target.append("&lt;");   notAppended = notAppended + i + 1; i = 0;   break;
              case '>':  target.append(notAppended, i); target.append("&gt;");   notAppended = notAppended + i + 1; i = 0;   break;
              case 10:
              case 13:  target.append(notAppended, i); target.append("&lt;br/&gt;");   notAppended = notAppended + i + 1; i = 0;   break;
              default:   i++; break;
              }
            }
          target.append(notAppended, i);
        }

template<class T>
        void AppendHtmlEscapedAndCorrectNonUTF8(T &target, const char* s, const char *end, bool tooltip){
// append c-string s to target, html escape some characters
// replace invalid UTF8 characters with ?
          if(!s) return;
          if (!end) end = s + strlen(s);
          int l = 0;                    // length of current utf8 codepoint
          size_t i = 0;                 // number of not yet appended chars
          const char* notAppended = s;  // position of the first character which is not yet appended
          for (const char* current = s; *current && current < end; current+=l) {
	    l = utf8CodepointIsValid(current);
            switch(l) {
              case 1:
                switch(*current) {
                  case '&':  target.append(notAppended, i); target.append("&amp;");  notAppended = notAppended + i + 1; i = 0;   break;
                  case '\"': target.append(notAppended, i); target.append("&quot;"); notAppended = notAppended + i + 1; i = 0;   break;
                  case '\'': target.append(notAppended, i); target.append("&apos;"); notAppended = notAppended + i + 1; i = 0;   break;
                  case '<':  target.append(notAppended, i); target.append("&lt;");   notAppended = notAppended + i + 1; i = 0;   break;
                  case '>':  target.append(notAppended, i); target.append("&gt;");   notAppended = notAppended + i + 1; i = 0;   break;
                  case 10:
                  case 13:
		    if (LiveSetup().GetUseAjax() || !tooltip) {
		      target.append(notAppended, i); target.append("&lt;br/&gt;");   notAppended = notAppended + i + 1; i = 0;   break;
		    } else {
		      target.append(notAppended, i); target.append(*current==10?"\\n":"\\r");   notAppended = notAppended + i + 1; i = 0;   break;
                    }
                  default:   i++; break;
                  }
                break;
              case 2:
              case 3:
              case 4:
                i += l;
                break;
              default:
// invalid UTF8
                target.append(notAppended, i);
                target.append("?");
                notAppended = notAppended + i + 1;
	        i = 0;
                l = 1;
              }
            }
          target.append(notAppended, i);
        }
template void AppendHtmlEscapedAndCorrectNonUTF8<std::string>(std::string &target, const char* s, const char *end, bool tooltip);
template void AppendHtmlEscapedAndCorrectNonUTF8<cLargeString>(cLargeString &target, const char* s, const char *end, bool tooltip);

        void AppendCorrectNonUTF8(std::string &target, const char* s){
// append c-string s to target
// replace invalid UTF8 characters with ?
          if(!s) return;
          int l = 0;                    // length of current utf8 codepoint
          size_t i = 0;                 // number of not yet appended chars
          const char* notAppended = s;  // position of the first character which is not yet appended
          for (const char* current = s; *current; current+=l) {
	    l = utf8CodepointIsValid(current);
            if( l > 0) { i += l; continue; }
// invalid UTF8
            target.append(notAppended, i);
            target.append("?");
            notAppended = notAppended + i + 1;
	    i = 0;
            l = 1;
            }
          target.append(notAppended, i);
        }

	wint_t getNextUtfCodepoint(const char *&p){
// get next codepoint, and increment p
// 0 is returned at end of string, and p will point to the end of the string (0)
	  if(!p || !*p) return 0;
//          do { l = utf8CodepointIsValid(p); } while ( l == 0 && *(++p));
          int l = utf8CodepointIsValid(p);
          if( l == 0 ) { p++; return '?'; }
          return Utf8ToUtf32(p, l);
	}

	int utf8CodepointIsValid(const char *p){
// In case of invalid UTF8, return 0
// otherwise, return number of characters for this UTF codepoint
	  static const uint8_t LEN[] = {2,2,2,2,3,3,4,0};

	  int len = ((*p & 0xC0) == 0xC0) * LEN[(*p >> 3) & 7] + ((*p | 0x7F) == 0x7F);
	  for (int k=1; k < len; k++) if ((p[k] & 0xC0) != 0x80) len = 0;
	  return len;
	}

	wint_t Utf8ToUtf32(const char *&p, int len){
// assumes, that uft8 validity checks have already been done. len must be provided. call utf8CodepointIsValid first
// change p to position of next codepoint (p = p + len)
	  static const uint8_t FF_MSK[] = {0xFF >>0, 0xFF >>0, 0xFF >>3, 0xFF >>4, 0xFF >>5, 0xFF >>0, 0xFF >>0, 0xFF >>0};
	  wint_t val = *p & FF_MSK[len];
          const char *q = p + len;
	  for (p++; p < q; p++) val = (val << 6) | (*p & 0x3F);
	  return val;
	}
	void AppendUtfCodepoint(std::string &target, wint_t codepoint){
	  if (codepoint <= 0x7F){
	     target.push_back( (char) (codepoint) );
	     return;
	  }
	  if (codepoint <= 0x07FF) {
	     target.push_back( (char) (0xC0 | (codepoint >> 6 ) ) );
	     target.push_back( (char) (0x80 | (codepoint & 0x3F)) );
	     return;
	  }
	  if (codepoint <= 0xFFFF) {
	     target.push_back( (char) (0xE0 | ( codepoint >> 12)) );
	     target.push_back( (char) (0x80 | ((codepoint >>  6) & 0x3F)) );
	     target.push_back( (char) (0x80 | ( codepoint & 0x3F)) );
	     return;
	  }
	     target.push_back( (char) (0xF0 | ((codepoint >> 18) & 0x07)) );
	     target.push_back( (char) (0x80 | ((codepoint >> 12) & 0x3F)) );
	     target.push_back( (char) (0x80 | ((codepoint >>  6) & 0x3F)) );
	     target.push_back( (char) (0x80 | ( codepoint & 0x3F)) );
	     return;
	}

	void AppendDuration(cLargeString &target, char const* format, int hours, int minutes )
	{
                int numChars = snprintf(target.borrowEnd(32), 32, format, hours, minutes);
		if (numChars < 0) {
                        target.finishBorrow(0);
			std::stringstream builder;
			builder << "cannot represent duration " << hours << ":" << minutes << " as requested";
			throw std::runtime_error( builder.str() );
		}
                target.finishBorrow(numChars);
	}
	void AppendDuration(std::string &target, char const* format, int hours, int minutes )
	{
		char result[ 32 ];
		if ( snprintf(result, sizeof(result), format, hours, minutes) < 0 ) {
			std::stringstream builder;
			builder << "cannot represent duration " << hours << ":" << minutes << " as requested";
			throw std::runtime_error( builder.str() );
		}
                target.append(result);
	}
	std::string FormatDuration( char const* format, int hours, int minutes )
        {
            std::string result;
            AppendDuration(result, format, hours, minutes );
            return result;
        }

	size_t AppendDateTime(char *target, int target_size, char const* format, time_t time)
// writes data to target, make sure that sizeof(target) >= target_size, before calling
	{
		if (!time) return 0;
		struct tm tm_r;
		if ( localtime_r( &time, &tm_r ) == 0 ) {
			std::stringstream builder;
			builder << "cannot represent timestamp " << time << " as local time";
			throw std::runtime_error( builder.str() );
		}
                size_t len = strftime(target, target_size, format, &tm_r); 
		if ( len == 0 ) {
			std::stringstream builder;
			builder << "representation of timestamp " << time << " exceeds " << (target_size - 1) << " bytes";
			throw std::runtime_error( builder.str() );
		}
		return len;
	}
	void AppendDateTime(cLargeString &target, char const* format, time_t time)
	{
		int len = AppendDateTime(target.borrowEnd(80), 80, format, time);
                target.finishBorrow(len);
	}
	void AppendDateTime(std::string &target, char const* format, time_t time )
	{
		char result[80];
		AppendDateTime(result, sizeof( result ), format, time);
                target.append(result);
	}
	std::string FormatDateTime( char const* format, time_t time )
	{
            char result[80];
            AppendDateTime(result, sizeof( result ), format, time);
            return result;
	}

	std::string StringReplace( std::string const& text, std::string const& substring, std::string const& replacement )
	{
		std::string result = text;
		size_t pos = 0;
		while ( ( pos = result.find( substring, pos ) ) != std::string::npos ) {
			result.replace( pos, substring.length(), replacement );
			pos += replacement.length();
		}
		return result;
	}

	std::vector<std::string> StringSplit( std::string const& text, char delimiter )
	{
		std::vector<std::string> result;
		size_t last = 0, pos;
		while ( ( pos = text.find( delimiter, last ) ) != std::string::npos ) {
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

	std::string StringWordTruncate(const std::string& input, size_t maxLen, bool& truncated)
	{
		if (input.length() <= maxLen)
		{
                        truncated = false;
			return input;
		}
		truncated = true;
		std::string result = input.substr(0, maxLen);
		size_t pos = result.find_last_of(" \t,;:.\n?!'\"/\\()[]{}*+-");
		return result.substr(0, pos);
	}

	std::string StringFormatBreak(std::string const& input)
	{
		return StringReplace( input, "\n", "<br/>" );
	}

	std::string StringEscapeAndBreak(std::string const& input, const char* nl)
	{
		std::stringstream plainBuilder;
		HtmlEscOstream builder(plainBuilder);
		builder << input;
		return StringReplace(plainBuilder.str(), "\n", nl);
	}

	std::string StringTrim(std::string const& str)
	{
		std::string res = str;
		size_t pos = res.find_last_not_of(' ');
		if(pos != std::string::npos) {
			res.erase(pos + 1);
			pos = res.find_first_not_of(' ');
			if(pos != std::string::npos) res.erase(0, pos);
		}
		else res.erase(res.begin(), res.end());
		return res;
	}


        const char *getText(const char *shortText, const char *description) {
          if (shortText && *shortText) return shortText;
          return description;
        }
// Spielfilm Thailand / Deutschland / Großbritannien 2015 (Rak ti Khon Kaen)
#define MAX_LEN_ST 70
template<class T>
        void AppendTextMaxLen(T &target, const char *text) {
// append text to target, but
//   stop at line break in text (10 || 13)
//   only up to MAX_LEN_ST characters. If such truncation is required, truncate at ' '

// escape html characters, and correct invalid utf8
          if (!text || !*text ) return;
          int len = strlen(text);
          int lb = len;
          for (const char *s = text; *s; s++) if (*s == 10 || *s == 13) { lb = s-text; break;}
          if (len < MAX_LEN_ST && lb == len)
            AppendHtmlEscapedAndCorrectNonUTF8(target, text);
          else if (lb < MAX_LEN_ST) {
            AppendHtmlEscapedAndCorrectNonUTF8(target, text, text + lb);
            target.append("...");
          } else {
            const char *end = text + MAX_LEN_ST;
            for (; *end && *end != ' ' && *end != 10 && *end != 13; end++);
            AppendHtmlEscapedAndCorrectNonUTF8(target, text, end);
            if (*end) target.append("...");
          }
        }
template void AppendTextMaxLen<std::string>(std::string &target, const char *text);
template void AppendTextMaxLen<cLargeString>(cLargeString &target, const char *text);

template<class T>
void AppendTextTruncateOnWord(T &target, const char *text, int max_len, bool tooltip) {
// append text to target, but
//   only up to max_len characters. If such truncation is required, truncate at ' '

// escape html characters, and correct invalid utf8
          if (!text || !*text ) return;
          const char *end = text + std::min((int)strlen(text), max_len);
          for (; *end && *end != ' '; end++);
          AppendHtmlEscapedAndCorrectNonUTF8(target, text, end, tooltip);
          if (*end) target.append("...");
}
template void AppendTextTruncateOnWord<std::string>(std::string &target, const char *text, int max_len, bool tooltip);
template void AppendTextTruncateOnWord<cLargeString>(cLargeString &target, const char *text, int max_len, bool tooltip);

	std::string ZeroPad(int number)
	{
		std::stringstream os;
		os << std::setw(2) << std::setfill('0') << number;
		return os.str();
	}

	std::string MD5Hash(std::string const& str)
	{
		char* szInput = strdup(str.c_str());
		if (!szInput) return "";
		char* szRes = MD5String(szInput);
		std::string res = szRes;
		free(szRes);
		return res;

	}

	std::string xxHash32(std::string const& str)
	{
	  char res[9];
	  uint32_t result = XXHash32::hash(str.c_str(), str.length(), 20);
	  res[8] = 0;
          for (int i = 7; i >= 0; i--) {
	    int dig = result % 16;
            if (dig < 10) res[i] = dig + '0';
            else          res[i] = dig - 10 + 'A';
            result /= 16;
          }
          return res;
	}

#define HOURS(x) ((x)/100)
#define MINUTES(x) ((x)%100)

	std::string ExpandTimeString(std::string timestring)
	{
		size_t colonpos = timestring.find(":");
		if (colonpos == std::string::npos)
		{
			if (timestring.size() == 1)
				timestring = "0" + timestring + ":00";
			else if (timestring.size() == 2)
				timestring = timestring + ":00";
			else if (timestring.size() == 3)
				timestring = "0" + std::string(timestring.begin(), timestring.begin() + 1) + ":" + std::string(timestring.begin() + 1, timestring.end());
			else
				timestring = std::string(timestring.begin(), timestring.begin() + 2) + ":" + std::string(timestring.begin() + 2, timestring.end());
		}
		else
		{
			std::string hours = std::string(timestring.begin(), timestring.begin() + colonpos);
			std::string mins = std::string(timestring.begin() + colonpos + 1, timestring.end());
			hours = std::string(std::max(0,(int)(2 - hours.size())), '0') + hours;
			mins = std::string(std::max(0,(int)(2 - mins.size())), '0') + mins;
			timestring = hours + ":" + mins;
		}
		return timestring;
	}

	time_t GetTimeT(std::string timestring) // timestring in HH:MM
	{
		timestring = StringReplace(timestring, ":", "");
		int iTime = lexical_cast<int>( timestring );
		struct tm tm_r;
		time_t t = time(NULL);
		tm* tmnow = localtime_r(&t, &tm_r);
		tmnow->tm_hour = HOURS(iTime);
		tmnow->tm_min = MINUTES(iTime);
		return mktime(tmnow);
	}

	struct urlencoder
	{
			std::ostream& ostr_;

			explicit urlencoder( std::ostream& ostr ): ostr_( ostr ) {}

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
				else if ( static_cast<signed char>(ch) < 0 || allowedChars[ size_t( ch ) ] == '_' )
					ostr_ << '%' << std::setw( 2 ) << std::setfill( '0' ) << std::hex << int( static_cast<unsigned char>(ch) );
				else
					ostr_ << ch;
			}
	};

	std::string StringUrlEncode( std::string const& input )
	{
		std::stringstream ostr;
		for_each( input.begin(), input.end(), urlencoder( ostr ) );
		return ostr.str();
	}

// returns the content of <element>...</element>
	std::string GetXMLValue( std::string const& xml, std::string const& element )
	{
		std::string start = "<" + element + ">";
		std::string end = "</" + element + ">";
		size_t startPos = xml.find(start);
		if (startPos == std::string::npos) return "";
		size_t endPos = xml.find(end);
		if (endPos == std::string::npos) return "";
		return xml.substr(startPos + start.size(), endPos - startPos - start.size());
	}

// return the time value as time_t from <datestring> formatted with <format>
	time_t GetDateFromDatePicker(std::string const& datestring, std::string const& format)
	{
		if (datestring.empty())
			return 0;
		int year = lexical_cast<int>(datestring.substr(format.find("yyyy"), 4));
		int month = lexical_cast<int>(datestring.substr(format.find("mm"), 2));
		int day = lexical_cast<int>(datestring.substr(format.find("dd"), 2));
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
int timeStringToInt(const char *t) {
// input: t in xx:xx format
// output: time in epgsearch format (int, 100*h + min)
  int h = 0, min = 0;
  sscanf(t, "%2d:%2d", &h, &min);
  return 100*h + min;
}
int timeStringToInt(const std::string &t) {
  return timeStringToInt(t.c_str() );
}
void intToTimeString(char *t, int tm) {
// sizeof (t) must be >= 6 "hh:mm", + ending 0
// see int timeStringToInt(const char *t) for formats
  unsigned int h = tm / 100;
  unsigned int min = tm % 100;
  snprintf(t, 6, "%.2u:%.2u", h%100u, min);
}

std::string intToTimeString(int tm) {
// see int timeStringToInt(const char *t) for formats
  char t[6];
  intToTimeString(t, tm);
  return t;
}
std::string charToString(const char *s) {
  if (!s) return "";
  return s;
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
				esyslog("live: cannot move under sub-directory\n");
				return false;
			}
			if (!MakeDirs(target.c_str(), true)) {
				esyslog("live: cannot create directory %s", target.c_str());
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
					esyslog("live: rename failed from %s to %s", source.c_str(), target.c_str());
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
						esyslog("live: cannot allocate renaming buffer");
						return false;
					}

					// loop through all files, but skip all subdirectories
					while ((e = d.Next()) != NULL) {
						// skip generic entries
						if (strcmp(e->d_name, ".") && strcmp(e->d_name, "..") && strcmp(e->d_name, "lost+found")) {
							std::string sourceFile = source + e->d_name;
							std::string targetFile = target + e->d_name;

							// copy only regular files
							if (!stat(sourceFile.c_str(), &st1) && S_ISREG(st1.st_mode)) {
								int r = -1, w = -1;
								cUnbufferedFile *inputFile = cUnbufferedFile::Create(sourceFile.c_str(), O_RDONLY | O_LARGEFILE);
								cUnbufferedFile *outputFile = cUnbufferedFile::Create(targetFile.c_str(), O_RDWR | O_CREAT | O_LARGEFILE);

								// validate files
								if (!inputFile || !outputFile) {
									esyslog("live: cannot open file %s or %s", sourceFile.c_str(), targetFile.c_str());
									success = false;
									break;
								}

								// do actual copy
								dsyslog("live: copying %s to %s", sourceFile.c_str(), targetFile.c_str());
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
							esyslog("live: cannot remove target %s", target.c_str());
						}
						found = target.find_last_of(delim);
						if (found != std::string::npos) {
							target = target.substr(0, found);
						}
						if (!RemoveEmptyDirectories(target.c_str(), true)) {
							esyslog("live: cannot remove target directory %s", target.c_str());
						}
						esyslog("live: copying failed");
						return false;
					}
					else if (!copy && !RemoveFileOrDir(source.c_str(), true)) { // delete source files
						esyslog("live: cannot remove source directory %s", source.c_str());
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
					esyslog("live: %s requires %dMB - only %dMB available", copy ? "moving" : "copying", required, available);
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

  const char *intToChar(char *buffer, char *buffer_end, int i) {
// buffer_end = buffer + sizeof(buffer)
// [buffer, buffer_end) is available
// buffer length is note checked :( . Just use long enough buffer, like 20
// I wrote this, because the standard tools make 1000 -> 1.000 :( :( :(
    buffer_end--;
    *buffer_end = 0;
    if (i < 0) {
      buffer_end--;
      *buffer_end = '-';
      i *= -1;
    }
    if (i < 10) {
      buffer_end--;
      *buffer_end = i + '0';
      return buffer_end;
    }
    for (; i > 0; i = i/10) {
      buffer_end--;
      *buffer_end = (i%10) + '0';
    }
    return buffer_end;  
  }
  std::string ScraperImagePath2Live(const std::string &path){
    int tvscraperImageDirLength = LiveSetup().GetTvscraperImageDir().length();
    if (tvscraperImageDirLength == 0) return "";
    if (path.compare(0, tvscraperImageDirLength, LiveSetup().GetTvscraperImageDir()) != 0) return "";
    return path.substr(tvscraperImageDirLength);
  }

  bool ScraperCallService(const char *Id, void *Data) {
    cPlugin *pScraper = LiveSetup().GetPluginScraper();
    if (!pScraper) return false;
    return pScraper->Service(Id, Data);
  }
 
} // namespace vdrlive
