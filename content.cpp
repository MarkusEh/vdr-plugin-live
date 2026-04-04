#include "content.h"
#include <istream>
#include <memory>
#include <jpeglib.h>

#define JPEG_QUALITY 85
#define JPEG_BUFFER_SIZE 65536

namespace vdrlive {

namespace {

struct error_mgr : public jpeg_error_mgr
{ error_mgr()
  { jpeg_std_error(this);
    error_exit = [](j_common_ptr cinfo) { throw cinfo->err; };
  }
};

class cpp_source_mgr : public jpeg_source_mgr
{
  std::ifstream& in;
  const std::unique_ptr<JOCTET[]> buffer;

  void InitSource()
  { in.rdbuf()->pubsetbuf(nullptr, 0); // avoid double buffering
    in.seekg(0);
  }
  boolean FillInputBuffer()
  { in.read((char*)buffer.get(), JPEG_BUFFER_SIZE);
    bytes_in_buffer = in.gcount();
    if (!bytes_in_buffer)
      return FALSE;
    next_input_byte = buffer.get();
    return TRUE;
  }
  void SkipInputData(long num)
  { if (num <= 0)
      return;
    if ((unsigned long)num <= bytes_in_buffer)
    { next_input_byte += num;
      bytes_in_buffer -= num;
    }
    else
    { in.seekg(num - bytes_in_buffer, in.cur);
      FillInputBuffer();
    }
  }

public:
  cpp_source_mgr(std::ifstream& in)
  : in(in),
    buffer(new JOCTET[JPEG_BUFFER_SIZE])
  {
    bytes_in_buffer = 0;
    init_source = [](j_decompress_ptr cinfo) { ((cpp_source_mgr*)cinfo->src)->InitSource(); };
    fill_input_buffer = [](j_decompress_ptr cinfo) { return ((cpp_source_mgr*)cinfo->src)->FillInputBuffer(); };
    skip_input_data = [](j_decompress_ptr cinfo, long num) { ((cpp_source_mgr*)cinfo->src)->SkipInputData(num); };
    resync_to_restart = jpeg_resync_to_restart;
    term_source = [](j_decompress_ptr cinfo) {};
  }
};

class decompress : public jpeg_decompress_struct
{
  error_mgr Err;
  cpp_source_mgr Src;
  decompress(const decompress&) = delete;
  void operator=(const decompress&) = delete;
public:
  decompress(std::ifstream& in)
  : Src(in)
  { jpeg_create_decompress(this);
    err = &Err;
    src = &Src;
  }
  ~decompress()
  { jpeg_destroy_decompress(this);
  }
};

class cpp_destination_mgr : public jpeg_destination_mgr
{
  void Enlarge()
  { size_t s = data.size();
    data.resize(s + JPEG_BUFFER_SIZE);
    next_output_byte = (JOCTET*)data.data() + s;
    free_in_buffer = JPEG_BUFFER_SIZE;
  }
  void Truncate()
  { data.resize(data.size() - free_in_buffer);
  }
public:
  std::string data;
  cpp_destination_mgr()
  { free_in_buffer = 0;
    init_destination = [](j_compress_ptr cinfo) { ((cpp_destination_mgr*)cinfo->dest)->Enlarge(); };
    empty_output_buffer = [](j_compress_ptr cinfo) { ((cpp_destination_mgr*)cinfo->dest)->Enlarge(); return TRUE; };
    term_destination = [](j_compress_ptr cinfo) { ((cpp_destination_mgr*)cinfo->dest)->Truncate(); };
  }
};

class compress : public jpeg_compress_struct
{
  error_mgr Err;
  cpp_destination_mgr Dest;
public:
  compress()
  { jpeg_create_compress(this);
    err = &Err;
    dest = &Dest;
  }
  ~compress()
  { jpeg_destroy_compress(this);
  }
  std::string& Data() { return Dest.data; }
};

} // namespace

std::size_t cFileContent::ReduceImageSize(unsigned width, unsigned height)
{
  struct decompress src(file);

  try
  {
    jpeg_read_header(&src, TRUE);

    // calculate decimation factor
    int df = 0;
    do
    { width <<= 1;
      height <<= 1;
    } while ((src.image_width >= width || src.image_height >= height) && ++df != 3);
    if (df == 0) // no scaling required
      goto nope;

    src.scale_num = 1;
    src.scale_denom = 1 << df;

    // init decompress
    src.data_precision = 8; // no HDR in preview
    jpeg_start_decompress(&src);

    // allocate scanline buffer
    unsigned row_stride = src.output_width * src.output_components;
    JSAMPARRAY buffer = (*src.mem->alloc_sarray)((j_common_ptr)&src, JPOOL_IMAGE, row_stride, src.rec_outbuf_height);

    // init compress
    compress dst;
    dst.image_width = src.output_width;
    dst.image_height = src.output_height;
    dst.input_components = src.output_components;
    dst.in_color_space = src.out_color_space;
    dst.data_precision = 8;
    jpeg_set_defaults(&dst);
    jpeg_set_quality(&dst, JPEG_QUALITY, TRUE);
    jpeg_start_compress(&dst, TRUE);

    // transfer data
    while (src.output_scanline < src.output_height)
    { unsigned lines = jpeg_read_scanlines(&src, buffer, src.rec_outbuf_height);
      jpeg_write_scanlines(&dst, buffer, lines);
    }

    jpeg_finish_compress(&dst);
    jpeg_finish_decompress(&src);

    // capture result
    reduced = std::move(dst.Data());
    return reduced.size();
  }
  catch (jpeg_error_mgr*) {} // from error_mgr.error_exit

nope:
  file.seekg(0);
  return 0; // keep original file content
}

}
