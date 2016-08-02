
namespace filter {
    class image {
    private:
        std::vector<unsigned char> _rgb;
        int _width;
        int _height;
        int _pixel_depth;
        std::string _fileName;
        std::vector<unsigned char> dest;
        

        void  flushToFile(std::string FileName, unsigned char *jpgSrc, unsigned long jpgSize) {
            using namespace boost::interprocess;

            if (!boost::filesystem::exists(FileName)) {
                {
                    file_mapping::remove(FileName.data());
                    std::filebuf fbuf;
                    fbuf.open(FileName, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
                    fbuf.pubseekoff(jpgSize - 1, std::ios_base::beg);
                    fbuf.sputc(0);
                }
            }
                file_mapping m_file(FileName.data(), read_write);
                mapped_region region(m_file, read_write);
                void *addr = region.get_address();
                std::size_t rSize = region.get_size();
                unsigned char *mem = static_cast<unsigned char*>(addr);

                std::memcpy(mem, jpgSrc, rSize);

                region.~mapped_region();
                m_file.~file_mapping();
        }

        void jpegIt(std::string FileName, unsigned char *img) {
            const int JPEG_QUALITY = 95;
            const int COLOR_COMPONENTS = _pixel_depth;

            unsigned char *outbuffer = NULL;
            unsigned long outsize = 0;

            tjhandle _jpegCompressor = tjInitCompress();

            tjCompress2(_jpegCompressor, dest.data(), _width, 0, _height, TJPF_RGB, &outbuffer, &outsize, TJSAMP_444, JPEG_QUALITY, TJFLAG_FASTDCT);

            flushToFile(FileName, outbuffer, outsize);

            tjDestroy(_jpegCompressor);

            tjFree(outbuffer);

        }

        void remove_extension(std::string name) {
            name.erase(name.end() - 4, name.end());
            std::cout << name;
            _fileName = name;
        }

    public:

        image() {

        }

       void read_jpeg(std::string FileName) {

           using namespace boost::interprocess;

               file_mapping m_file(FileName.data(), read_only);
               mapped_region region(m_file, read_only);
               void *addr = region.get_address();
               std::size_t jpgSize = region.get_size();
               unsigned char *jpgSrc = static_cast<unsigned char*>(addr);

            int jpegSubsamp;

            tjhandle _jpegDecompressor = tjInitDecompress();

            tjDecompressHeader2(_jpegDecompressor, jpgSrc, jpgSize, &_width, &_height, &jpegSubsamp);
            
            _pixel_depth = 3;
            _rgb.resize(_width*_height * 3);

            tjDecompress2(_jpegDecompressor, jpgSrc, jpgSize, _rgb.data(), _width, 0, _height, TJPF_RGB, TJFLAG_FASTDCT);

            tjDestroy(_jpegDecompressor);

            region.~mapped_region();
            m_file.~file_mapping();

            remove_extension(FileName);

           
        }

 /*      auto IJP(unsigned i, unsigned j, unsigned pixelComponent) {
           if ((i < 0) | (i >= (int)_width) | (j < 0) | (j >= (int)_height)) {
               return 0;

           } else {
               return (int)(i*_pixel_depth + j*_pixel_depth*_width + pixelComponent);
           }
       };*/

       void filter() {
           
          auto IJP = [=](unsigned i, unsigned j, unsigned pixelComponent) {
               if ( (i < 0) | (i >= (int)_width) | (j < 0) | (j >= (int)_height) ) {
                   return 0;

               } else {
                   return (int)(i*_pixel_depth + j*_pixel_depth*_width + pixelComponent);
               }
               };

        /* std::vector<int> kernel = {  1,  2, 1,
                                        2,  4, 2,
                                        1,  2, 1
           };*/
      /*     std::vector<int> kernel = { 1,  1, 1,
               1,  1, 1,
               1,  1, 1
           };*/
       /*    std::vector<int> kernel = { 0,  -1, 0,
               -1,  5, -1,
               0,  -1, 0
           };*/
           std::vector<int> kernel = { 1,  0, -1,
               0,  0, 0,
               -1,  0, 1
           };
         /*  std::vector<int> kernel = { 0,  1, 0,
               1,  -4, 1,
              0,  1, 0
           };
           */

           int sum = 0;
           for (int i = 0; i < kernel.size(); ++i) {
               sum += kernel[i];
           }
           if (sum == 0) sum = 1;


           //std::vector<unsigned char> dest;
           unsigned char *src = _rgb.data();

           dest.resize(_width*_height*_pixel_depth);
           unsigned stride = _pixel_depth*_width;

           unsigned char checkSrc=0,checkDest=0;
           for (unsigned y = 0; y < _height; ++y) {
               for (unsigned x = 0; x < _width; ++x) {
                   for (unsigned p = 0; p < _pixel_depth; ++p) {
                       checkDest =
                           (kernel[0] * src[IJP(x - 1, y - 1, p)] +
                               kernel[1] * src[IJP(x - 1, y, p)] +
                               kernel[2] * src[IJP(x - 1, y + 1, p)] +
                               kernel[3] * src[IJP(x, y - 1, p)] +
                               kernel[4] * src[IJP(x, y, p)] +
                               kernel[5] * src[IJP(x, y + 1, p)] +
                               kernel[6] * src[IJP(x + 1, y - 1, p)] +
                               kernel[7] * src[IJP(x + 1, y, p)] +
                               kernel[8] * src[IJP(x + 1, y + 1, p)]) / sum;
                           
                       dest[IJP(x, y, p)] = checkDest;
                       checkSrc = src[IJP(x, y, p)];
                   }
               }
           }

           std::string newName = _fileName + "filtered.jpg";
           boost::interprocess::file_mapping::remove(newName.data());
           jpegIt(newName, dest.data());
           dest.resize(0);
       }

    };

}