#include <iostream>
#include <fstream>
#include "tools.h"
#include "xxhash.h"
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

  std::string FormatDuration(char const* format, int duration)
  {
    cToSvConcat result;
    AppendDuration(result, format, duration);
    return std::string(result);
  }

  cSv StringWordTruncate(cSv text, size_t maxLen, bool& truncated) {
// Return text truncted to maxLen characters. If such truncation is required,
// truncate at any char in the char list in find_last_of (see code below ...)
    if (text.length() <= maxLen) {
      truncated = false;
      return text;
    }
    truncated = true;
    cSv result = text.substr(0, maxLen);
    size_t pos = result.find_last_of(" \t\n,;:.?!()[]{}*+-");
    if (pos == std::string::npos) return result;
    if (strchr("?!)]}", result[pos])) ++pos; // truncate after these characters
    return StringTrim(result.substr(0, pos));
  }

  cSv StringTrim(cSv str)
  {
    size_t pos = str.find_last_not_of(' ');
    if (pos == std::string::npos) return cSv();
    cSv trailBlankRemoved = str.substr(0, pos+1);
    pos = trailBlankRemoved.find_first_not_of(' ');
    if (pos == std::string::npos) return cSv();
    return trailBlankRemoved.substr(pos);
  }

  std::string MD5Hash(std::string const& str)
  {
    char* szInput = strdup(str.c_str());
    if (!szInput) return "";
    char* szRes = MD5String(szInput);
    std::string res = szRes;
    free(szRes);
    free(szInput);
    return res;
  }

  XXH64_hash_t parse_hex_64(cSv str) {
    if (str.length() != 16) {
      esyslog("live: ERROR in parse_hex_64, hex = \"%.*s\" is not 16 chars long", (int)str.length(), str.data());
      return 0;
    }
    size_t parsed_chars;
    XXH64_hash_t result = parse_hex<XXH64_hash_t>(str, &parsed_chars);
    if (parsed_chars == 16) return result;
    esyslog("live: ERROR in  parse_hex_64, hex = \"%.*s\" contains invalid characters", (int)str.length(), str.data());
    return 0;
  }
  XXH128_hash_t parse_hex_128(cSv str) {
    XXH128_hash_t result;
    if (str.length() != 32) {
      esyslog("live: ERROR in parse_hex_128, hex = \"%.*s\" is not 32 chars long", (int)str.length(), str.data());
      result.low64 = 0;
      result.high64 = 0;
      return result;
    }
    size_t parsed_chars_h, parsed_chars_l;
    result.high64 = parse_hex<XXH64_hash_t>(str.substr(0, 16), &parsed_chars_h);
    result.low64  = parse_hex<XXH64_hash_t>(str.substr(16),    &parsed_chars_l);
    if (parsed_chars_l == 16 && parsed_chars_h == 16) return result;
    esyslog("live: ERROR in  parse_hex_128, hex = \"%.*s\" contains invalid characters", (int)str.length(), str.data());
    result.low64 = 0;
    result.high64 = 0;
    return result;
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

  time_t GetTimeT(cSv timestring) // timestring in HH:MM
  {
    int iTime = parse_int<int>( cToSvReplace(timestring, ":", "") );
    struct tm tm_r;
    time_t t = time(NULL);
    tm* tmnow = localtime_r(&t, &tm_r);
    tmnow->tm_hour = HOURS(iTime);
    tmnow->tm_min = MINUTES(iTime);
    return mktime(tmnow);
  }

// return the time value as time_t from <datestring> formatted with <format>
  time_t GetDateFromDatePicker(cSv datestring, cSv format)
  {
    if (datestring.empty())
      return 0;
    int year = parse_int<int>(datestring.substr(format.find("yyyy"), 4));
    int month = parse_int<int>(datestring.substr(format.find("mm"), 2));
    int day = parse_int<int>(datestring.substr(format.find("dd"), 2));
    struct tm tm_r;
    tm_r.tm_year = year - 1900;
    tm_r.tm_mon = month -1;
    tm_r.tm_mday = day;
    tm_r.tm_hour = tm_r.tm_min = tm_r.tm_sec = 0;
    tm_r.tm_isdst = -1; // makes sure mktime() will determine the correct DST setting
    return mktime(&tm_r);
  }

// format is in datepicker format ('mm' for month, 'dd' for day, 'yyyy' for year)
  std::string DatePickerToC(time_t date, cSv format)
  {
    if (date == 0) return "";
    cToSvConcat cformat;
    cformat.appendReplace(format, "mm", "%m");
    cformat.replaceAll("dd", "%d");
    cformat.replaceAll("yyyy", "%Y");
    return std::string(cToSvDateTime(cformat, date));
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

  void EncodeDomId(char *toEncode_s, char *toEncode_e, char const * from, char const * to)
  {
    for (; *from && *to; from++, to++) {
      std::replace(toEncode_s, toEncode_e, *from, *to);
    }
  }

  std::string EncodeDomId(cSv toEncode, char const * from, char const * to)
  {
    std::string encoded(toEncode);
    EncodeDomId(encoded.data(), encoded.data() + encoded.length(), from, to);
    return encoded;
  }

  std::string FileSystemExchangeChars(cSv s, bool ToFileSystem)
  {
    if (s.empty()) return std::string();
    char *str = reinterpret_cast<char*>(std::malloc(s.length() + 1)); // VDR ExchangeChars needs a pointer to data allocated with malloc
    if (!str) {
      esyslog("live, ERROR: out of memory in FileSystemExchangeChars");
      return std::string(s);
    }
    std::memcpy(str, s.data(), s.length());
    str[s.length()] = 0;
    str = ExchangeChars(str, ToFileSystem);
    std::string data = str;
    std::free(str);
    return data;
  }

  bool MoveDirectory(cSv sourceDir, cSv targetDir, bool copy)
  {
    const char* delim = "/";
    std::string source(sourceDir);
    std::string target(targetDir);

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
      if (stat(source.c_str(), &st1) != 0) {
        esyslog("live: rename. Source %s does not exist", source.c_str());
        return false;
      }
      stat(target.c_str(), &st2);
      if (!copy && (st1.st_dev == st2.st_dev)) {
        if (!cVideoDirectory::RenameVideoFile(source.c_str(), target.c_str())) {
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
              while (source != cVideoDirectory::Name()) {
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
            while (target != cVideoDirectory::Name()) {
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

  cSv ScraperImagePath2Live(cSv path) {
    int tvscraperImageDirLength = LiveSetup().GetTvscraperImageDir().length();
    if (tvscraperImageDirLength == 0) return cSv();
    if (path.compare(0, tvscraperImageDirLength, LiveSetup().GetTvscraperImageDir()) != 0) {
      esyslog("live: ERROR, image path %.*s does not start with %s", (int)path.length(), path.data(), LiveSetup().GetTvscraperImageDir().c_str());
      return cSv();
    }
    return path.substr(tvscraperImageDirLength);
  }

  bool ScraperCallService(const char *Id, void *Data) {
    cPlugin *pScraper = LiveSetup().GetPluginScraper();
    if (!pScraper) return false;
    return pScraper->Service(Id, Data);
  }
} // namespace vdrlive
