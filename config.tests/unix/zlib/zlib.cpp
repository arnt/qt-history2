#include <zlib.h>

#if !defined(ZLIB_VERNUM) || ZLIB_VERNUM < 0x1080
#  error "Required zlib version 1.0.8 not found."
#endif

int main(int, char **)
{
    z_streamp stream;
    stream = 0;
    const char *ver = zlibVersion();
    ver = 0;
    return 0;
}
