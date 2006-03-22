#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>

#ifndef FC_RGBA_UNKNOWN
#  error "This version of fontconfig is tool old, it is missing the FC_RGBA_UNKNOWN define"
#endif

int main(int, char **)
{
    FT_Face face;
    face = 0;
    FcPattern *pattern;
    pattern = 0;
    return 0;
}
