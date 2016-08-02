#include "filter.h"




int main() {
    auto start = std::chrono::high_resolution_clock::now();
 //---------------------------------------------------------------------------------------------


    std::string FileName("building.jpg");

    filter::image im;

   im.read_jpeg(FileName.data());
   im.laplacian();
   im.sobel();
   im.sobelFeldman();
   im.prewitt();
   im.roberts();


//---------------------------------------------------------------------------------------------
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    std::cout << "\n Microseconds: " << microseconds;

    std::getchar();

}