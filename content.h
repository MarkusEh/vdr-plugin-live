#ifndef VDR_LIVE_CONTENT_H_
#define VDR_LIVE_CONTENT_H_

#include <fstream>

namespace vdrlive {

/// File content class
/// @details May represent a reduced image or just a file.
class cFileContent
{
  std::ifstream file;
  std::string reduced;

public:
  cFileContent(const std::string& path) : file(path, std::ifstream::binary) {}
  /// Check whether the source stream is valid.
  explicit operator bool() const { return !!file; }
  /// Scale an image file for a minimum target size.
  /// @return Size of the reduced data in bytes.
  /// 0 if failed. In this case the original file content is returned.
  /// @remarks The scaling uses DCT decimation only and will likely return a larger image.
  std::size_t ReduceImageSize(unsigned width, unsigned height);
  /// Write the (reduced) content to a stream.
  void WriteTo(std::ostream& out)
  { if (reduced.empty())
      out << file.rdbuf();
    else
      out << reduced;
  }
};

}
#endif // VDR_LIVE_CONTENT_H_
