#include <mmintrin.h>

int main(int, char**)
{
    _mm_unpackhi_pi16(_mm_setzero_pd(), _mm_setzero_pd());
    return 0;
}
