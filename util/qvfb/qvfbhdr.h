
#include <qcolor.h>

#define QT_VFB_MOUSE_PIPE	"/tmp/.qtvfb_mouse"
#define QT_VFB_KEYBOARD_PIPE	"/tmp/.qtvfb_keyboard"

struct QVFbHeader
{
    int width;
    int height;
    int depth;
    int linestep;
    int dataoffset;
    QRect update;
    bool dirty;
    int  numcols;
    QRgb clut[256];
};

struct QVFbKeyData
{
    unsigned int unicode;
    unsigned int modifiers;
    bool press;
    bool repeat;
};

