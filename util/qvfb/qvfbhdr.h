
#include <qcolor.h>

#define QT_VFB_MOUSE_PIPE "/tmp/.qtvfb_mouse"

struct QVFbHeader
{
    int width;
    int height;
    int depth;
    int linestep;
    int dataoffset;
    QRgb clut[256];
    int  numcols;
    QRect update;
    bool dirty;
};


