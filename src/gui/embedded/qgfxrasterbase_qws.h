/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGFXRASTERBASE_QWS_H
#define QGFXRASTERBASE_QWS_H

#include "QtGui/qgfx_qws.h"
#include "QtGui/qscreen_qws.h"
#include "QtGui/qpen.h"
#include "QtGui/qbrush.h"
#include "QtGui/qwsdisplay_qws.h"
#include "QtGui/qregion.h"
#include "QtGui/qpixmap.h"

//===========================================================================
// Utility macros and functions [start]

#if !defined(QT_NO_QWS_GFX_SPEED)
# define QWS_EXPERIMENTAL_FASTPATH
# define Q_GFX_INLINE inline
#else
# define Q_GFX_INLINE
# define QWS_NO_WRITE_PACKING
#endif


// VGA16 code does not compile on sparc
#if defined(__sparc__) && !defined(QT_NO_QWS_VGA_16)
# define QT_NO_QWS_VGA_16
#endif

#define QGfxRaster_Generic 0
#define QGfxRaster_VGA16   1

#define GFX_CLOSEST_PIXEL(r,g,b) gfx_screen->alloc(r,g,b)
#define GFX_CLOSEST_PIXEL_CURSOR(r,g,b) qt_screen->alloc(r,g,b)

#define MASK4BPP(x) (0xf0 >> (x))


#if !defined(QT_NO_QWS_CURSOR) && !defined(QT_QWS_ACCEL_CURSOR)
# define GFX_START(r) \
    bool swc_do_save=false; \
    if(is_screen_gfx && gfx_swcursor) { \
        swc_do_save = gfx_screencursor->restoreUnder(r); \
    }

# define GFX_END \
    if(is_screen_gfx && gfx_swcursor) { \
        if(swc_do_save) \
            gfx_screencursor->saveUnder(); \
    }
#else //QT_NO_QWS_CURSOR
# define GFX_START(r)
# define GFX_END
#endif //QT_NO_QWS_CURSOR


#define GET_MASKED(rev, advance) \
    if(amonolittletest) { \
        if(amonobitval & 0x1) { \
            masked=false; \
        } \
        amonobitval=amonobitval >> 1; \
    } else { \
        if(amonobitval & 0x80) { \
            masked=false; \
        } \
        amonobitval=amonobitval << 1; \
        amonobitval=amonobitval & 0xff; \
    } \
    if(amonobitcount<7) { \
        amonobitcount++; \
    } else if (advance) { \
        amonobitcount=0; \
        if (rev) maskp--; \
        else maskp++; \
        amonobitval=*maskp; \
    } \


typedef unsigned int PackType;
// Pull this private function in from qglobal.cpp
extern unsigned int qt_int_sqrt(unsigned int n);

/*
    Finds a pointer to pixel (\a x, \a y) in a bitmap that
    is \a w pixels wide and stored in \a base. \a is_bigendian determines
    endianness. \a linestep is the bitmap's linestep in bytes, \a
    rev is true if this is being used for a reverse blt.

    \a astat returns the bit number within the byte
    \a ahold holds the \c monobitval which is the byte pre-shifted
            to match the algorithm using this function

    This is used by blt() to set up the pointer to the mask for
    Little/BigEndianMask alpha types.
*/
inline  unsigned char *find_pointer(unsigned char *base, int x, int y, int w, int linestep,
                                    int &astat, unsigned char &ahold, bool is_bigendian, bool rev)
{
    int nbits;
    int nbytes;

    if (rev) {
        is_bigendian = !is_bigendian;
        nbits = 7 - (x+w) % 8;
        nbytes = (x+w) / 8;
    } else {
        nbits = x % 8;
        nbytes = x / 8;
    }

    astat=nbits;

    unsigned char *ret = base + (y * linestep) + nbytes;

    ahold = *ret;
    if(is_bigendian) {
        ahold=ahold << nbits;
    } else {
        ahold=ahold >> nbits;
    }

    return ret;
}

inline unsigned const char *find_pointer(unsigned const char *base, int x, int y, int w,
                                         int linestep, int &astat, unsigned char &ahold,
                                         bool is_bigendian, bool rev)
{
    return  find_pointer(const_cast<unsigned char *>(base), x, y, w, linestep, astat, ahold,
                         is_bigendian, rev);
}

inline void gfxSetRgb24(unsigned char *d, unsigned int p)
{
    *d = p & 0x0000ff;
    *(d+1) = (p & 0x00ff00) >> 8;
    *(d+2) = (p & 0xff0000) >> 16;
}

inline void gfxSetRgb24(unsigned char *d, int r, int g, int b)
{
    *d = b;
    *(d+1) = g;
    *(d+2) = r;
}

inline unsigned int gfxGetRgb24(unsigned const char *d)
{
    return *d | (*(d+1)<<8) | (*(d+2)<<16);
}

inline void gfxGetRgb24(unsigned const char *d, int &r, int &g, int &b)
{
    r = *(d + 2);
    g = *(d + 1);
    b = *d;
}

// Utility macros and functions [end]
//===========================================================================


class QGfxRasterBase : public QGfx {

public:

    QGfxRasterBase(unsigned char *, int w, int h);
    ~QGfxRasterBase();

    virtual void setBrush(const QBrush &);

    virtual void setClipDeviceRegion(const QRegion &);

    virtual void setAlphaType(AlphaType);

    virtual void setLineStep(int i) { lstep=i; }
    int linestep() const { return lstep; }

    int pixelWidth() const { return width; }
    int pixelHeight() const { return height; }
    virtual int bitDepth() = 0;

    virtual void setScreen(QScreen *t, QScreenCursor *c,bool swc, int *ot, int *lo) {
        gfx_screen=t;
#ifndef QT_NO_QWS_CURSOR
        gfx_screencursor=c;
        gfx_swcursor=swc;
#endif
        setClut(gfx_screen->clut(),gfx_screen->numCols());
    }

    void setClut(QRgb *cols, int numcols) { clut=cols; clutcols=numcols; }

protected:

    bool inClip(int x, int y, QRect *cr = 0, bool know_to_be_outside = false);

    unsigned char *scanLine(int i) { return buffer+(i*lstep); }
    unsigned const char *srcScanLine(int i) { return srcbits + (i*srclinestep); }

    // Convert to/from different bit depths (defined below)
    Q_GFX_INLINE unsigned int get_value_32(int sdepth,unsigned const char **srcdata,
                                         bool reverse = false);
    Q_GFX_INLINE unsigned int get_value_24(int sdepth,unsigned const char **srcdata,
                                         bool reverse = false);
    Q_GFX_INLINE unsigned int get_value_16(int sdepth,unsigned const char **srcdata,
                                         bool reverse = false);
    Q_GFX_INLINE unsigned int get_value_15(int sdepth,unsigned const char **srcdata,
                                         bool reverse = false);
    Q_GFX_INLINE unsigned int get_value_8(int sdepth,unsigned const char **srcdata,
                                         bool reverse = false);
    Q_GFX_INLINE unsigned int get_value_4(int sdepth,unsigned const char **srcdata,
                                         bool reverse = false);
    Q_GFX_INLINE unsigned int get_value_1(int sdepth,unsigned const char **srcdata,
                                         bool reverse = false);

#ifdef DEBUG_POINTERS
    void checkSource(unsigned char *c,int i) {
        unsigned char *tmp1 = srcbits + (i * srclinestep);
        unsigned char *tmp2 = tmp1 + srclinestep;
        if(i < 0)
            qFatal("Negative source coordinate");
        if(i >= srcheight)
            qFatal("Source pointer height overrun");
        if(c < tmp1)
            qFatal("Source pointer underrun");
        if(c >= tmp2)
            qFatal("Source pointer overrun");
    }

    void checkMask(unsigned char *c,int i) {
        unsigned char *tmp1 = alphabits + (i * alphalinestep);
        unsigned char *tmp2 = tmp1 + alphalinestep;
        if(i < 0)
            qFatal("Negative mask coordinate");    // Convert to/from different bit depths
        if(i >= srcheight)
            qFatal("Mask height overrun");
        if(c < tmp1)
            qFatal("Alpha pointer underrun");
        if(c >= tmp2)
            qFatal("Alpha pointer overrun");
    }

    void checkDest(unsigned char *c,int i) {
        unsigned char *tmp1 = buffer + (i * lstep);
        unsigned char *tmp2 = tmp1 + lstep;
        if(i < 0)
            qFatal("Negative dest coordinate");
        if(i >= height)
            qFatal("Destination height overrun");
        if(c < tmp1)
            qFatal("Destination pointer underrun");
        if(c >= tmp2)
            qFatal("Destination pointer overrun");
    }
#endif

protected:
    QScreen *gfx_screen;
#ifndef QT_NO_QWS_CURSOR
    QScreenCursor *gfx_screencursor;
#endif
    bool gfx_swcursor;               // Software cursor?

    // Pen, brushes and colors ------------------
    QBrush cbrush;
    QRgb brushColor;
    unsigned long int brushPixel;
    unsigned long int pixel;        // (palette) pixel used for current drawing operation

    unsigned int srcclut[256];      // Source color table - r,g,b values
    unsigned int transclut[256];    // Source clut transformed to destination
                                    // values - speed optimisation
    QRgb *clut;                     // Destination color table - r,g,b values
    int clutcols;                   // Colours in clut

    int calpha;                     // Constant alpha value

    // Sizes and offsets ------------------------
    int width;
    int height;
    unsigned int lstep;

    int srcwidth;
    int srcheight;
    int srcdepth;
    int srclinestep;

    bool src_little_endian;
    bool src_normal_palette;

    int monobitcount;
    unsigned char monobitval;
    int amonobitcount;
    unsigned char amonobitval;

    QScreen::PixelType pixeltype;
    QScreen::PixelType srcpixeltype;
    SourceType srctype;
    unsigned const char *srcbits;
    unsigned char *const buffer;

    AlphaType alphatype;
    unsigned char *alphabits;
    unsigned int *alphabuf;
    unsigned char *maskp;
    int alphalinestep;
    bool ismasking;
    bool amonolittletest;

    // Clipping and regions ---------------------
    QRegion cliprgn;
    QRect clipbounds;

    int clipcursor;
    QRect *cliprect;
    int ncliprect;

    friend class QScreenCursor;
};

static bool simple_8bpp_alloc = false;

//===========================================================================
// Inline function definitions [start]
Q_GFX_INLINE unsigned int QGfxRasterBase::get_value_32(int sdepth, unsigned const char ** srcdata,
                                                     bool reverse) //can't be true if sdepth!=depth
{
// Convert between pixel values for different depths
    unsigned int ret;
    if(sdepth==32) {
        ret = *reinterpret_cast<unsigned const int *>(*srcdata);
        if(reverse) {
            (*srcdata)-=4;
        } else {
            (*srcdata)+=4;
        }
#if !defined(QT_NO_QWS_DEPTH_24)
    } else if(sdepth==24) {
        ret = gfxGetRgb24(*srcdata);
        (*srcdata) += 3;
#endif
#if !defined(QT_NO_IMAGE_16_BIT) || !defined(QT_NO_QWS_DEPTH_16)
    } else if(sdepth==16) {
        unsigned short int hold=*reinterpret_cast<unsigned const short int *>(*srcdata);
        ret = qt_conv16ToRgb(hold);
        (*srcdata)+=2;
#endif
    } else if(sdepth==8) {
        unsigned char val=*(*srcdata);
        if(src_normal_palette) {
            ret=((val >> 5) << 16)  | ((val >> 6) << 8) | (val >> 5);
        } else {
            ret = srcclut[val];
        }
        (*srcdata)++;
    } else if(sdepth==1) {
        if(monobitcount<8) {
            monobitcount++;
        } else {
            monobitcount=1;        // yes, 1 is correct
            (*srcdata)++;
            monobitval=*((*srcdata));
        }
        if(src_little_endian) {
            ret=monobitval & 0x1;
            monobitval=monobitval >> 1;
        } else {
            ret=(monobitval & 0x80) >> 7;
            monobitval=monobitval << 1;
            monobitval=monobitval & 0xff;
        }
        ret=srcclut[ret];
    } else {
        qDebug("Odd source depth %d!",sdepth);
        ret=0;
    }
#ifndef QT_NO_QWS_DEPTH_32_BGR
        if (pixeltype != srcpixeltype) {
            ret = (ret&0x0000ff)<<16 | (ret&0xff00ff00) | (ret&0xff0000)>>16;
        }
#endif

    return ret;
}


Q_GFX_INLINE unsigned int QGfxRasterBase::get_value_24(int sdepth, unsigned const char **srcdata,
                                                     bool reverse) //can't be true if sdepth!=depth
{
    unsigned int ret;
    if (sdepth == 24) {
        ret = gfxGetRgb24(*srcdata);
        if (reverse)
            (*srcdata)-=3;
        else
            (*srcdata)+=3;
    } else {
        ret = get_value_32(sdepth, srcdata, reverse);
    }

    return ret;
}


Q_GFX_INLINE unsigned int QGfxRasterBase::get_value_16(int sdepth, unsigned const char **srcdata,
                                                     bool reverse) //can't be true if sdepth!=depth
{
#if !defined(QT_NO_IMAGE_16_BIT) || !defined(QT_NO_QWS_DEPTH_16)
    unsigned int ret = 0;
    if (sdepth == 16) {
        unsigned short int hold = *reinterpret_cast<unsigned const short int *>(*srcdata);
        if(reverse) {
            (*srcdata)-=2;
        } else {
            (*srcdata)+=2;
        }
        ret=hold;
    } else if(sdepth==8) {
        unsigned char val=*((*srcdata));
        QRgb hold;
        if(src_normal_palette) {
            hold = val*0x010101;
        } else {
            hold=srcclut[val];
        }
        ret=qt_convRgbTo16(hold);
        (*srcdata)++;
    } else if(sdepth==1) {
        if(monobitcount<8) {
            monobitcount++;
        } else {
            monobitcount=1;
            (*srcdata)++;
            monobitval=*((*srcdata));
        }
        if(src_little_endian) {
            ret=monobitval & 0x1;
            monobitval=monobitval >> 1;
        } else {
            ret=(monobitval & 0x80) >> 7;
            monobitval=monobitval << 1;
            monobitval=monobitval & 0xff;
        }
        ret=srcclut[ret];
    } else if (sdepth == 32) {
        unsigned int hold = *reinterpret_cast<unsigned const int *>(*srcdata);
        ret=qt_convRgbTo16(hold);
        (*srcdata)+=4;
    } else {
        qFatal("Odd source depth %d!",sdepth);
        ret=0;
    }

    return ret;
#endif
}


Q_GFX_INLINE unsigned int QGfxRasterBase::get_value_8(int sdepth, unsigned const char **srcdata,
                                                    bool reverse) //can't be true if sdepth!=depth
{
    unsigned int ret;

    if(sdepth==8) {
        unsigned char val=*(*srcdata);
        // If source!=QImage, then the palettes will be the same
        if(src_normal_palette) {
            ret=val;
        } else {
            ret=transclut[val];
        }
        if(reverse) {
            (*srcdata)--;
        } else {
            (*srcdata)++;
        }
    } else if(sdepth==1) {
        if(monobitcount<8) {
            monobitcount++;
        } else {
            monobitcount=1;
            (*srcdata)++;
            monobitval=*((*srcdata));
        }
        if(src_little_endian) {
            ret=monobitval & 0x1;
            monobitval=monobitval >> 1;
        } else {
            ret=(monobitval & 0x80) >> 7;
            monobitval=monobitval << 1;
            monobitval=monobitval & 0xff;
        }
        ret = transclut[ret];
    } else if(sdepth==32) {
        unsigned int r,g,b;
        unsigned int hold=*reinterpret_cast<unsigned const int *>(*srcdata);
        r=(hold & 0xff0000) >> 16;
        g=(hold & 0x00ff00) >> 8;
        b=(hold & 0x0000ff);
        simple_8bpp_alloc=true;
        ret = GFX_CLOSEST_PIXEL(r,g,b);
        simple_8bpp_alloc=false;
        (*srcdata)+=4;
    } else if(sdepth==16) {
        unsigned int r,g,b;
        unsigned short int hold=*reinterpret_cast<unsigned const short int *>(*srcdata);
        r=((hold & (0x1f << 11)) >> 11) << 3;
        g=((hold & (0x3f << 5)) >> 5) << 2;
        b=(hold & 0x1f) << 3;
        simple_8bpp_alloc=true;
        ret = GFX_CLOSEST_PIXEL(r,g,b);
        simple_8bpp_alloc=false;
        (*srcdata)+=2;
    } else if (sdepth == 4) {
        ret = monobitval & 0x0f;
        if (!monobitcount) {
            monobitcount = 1;
            monobitval >>= 4;
        } else {
            monobitcount = 0;
            (*srcdata)++;
            monobitval = *(*srcdata);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (!src_little_endian)
                monobitval = (monobitval & 0x0f) << 4 |  (monobitval & 0xf0) >> 4;
#endif
        }
    } else {
        qDebug("Cannot do %d->8!",sdepth);
        ret=0;
    }

    return ret;
}


Q_GFX_INLINE unsigned int QGfxRasterBase::get_value_4(int sdepth, unsigned const char **srcdata,
                                                    bool reverse) //can't be true if sdepth!=depth
{
    unsigned int ret;

    if (sdepth == 4) {
        if (reverse) {
            ret = (monobitval & 0xf0) >> 4;
            if (!monobitcount) {
                monobitcount = 1;
                monobitval <<= 4;
            } else {
                monobitcount = 0;
                (*srcdata)--;
                monobitval = *(*srcdata);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                if (!src_little_endian)
                    monobitval = (monobitval & 0x0f) << 4 |  (monobitval & 0xf0) >> 4;
#endif
            }
        } else {
            ret = monobitval & 0x0f;
            if (!monobitcount) {
                monobitcount = 1;
                monobitval >>= 4;
            } else {
                monobitcount = 0;
                (*srcdata)++;
                monobitval = *(*srcdata);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                if (!src_little_endian)
                    monobitval = (monobitval & 0x0f) << 4 |  (monobitval & 0xf0) >> 4;
#endif
            }
        }
    } else if (sdepth == 1) {
        if(monobitcount<8) {
            monobitcount++;
        } else {
            monobitcount=1;
            (*srcdata)++;
            monobitval=*((*srcdata));
        }
        if(src_little_endian) {
            ret=monobitval & 0x1;
            monobitval=monobitval >> 1;
        } else {
            ret=(monobitval & 0x80) >> 7;
            monobitval=monobitval << 1;
            monobitval=monobitval & 0xff;
        }
        ret = transclut[ret];
    } else if (sdepth == 8) {
        unsigned char val=*(*srcdata);
        ret = transclut[val];
        if(reverse)
            (*srcdata)--;
        else
            (*srcdata)++;
    } else if(sdepth==32) {
        unsigned int r,g,b;
        unsigned int hold=*reinterpret_cast<unsigned const int*>(*srcdata);
        r=(hold & 0xff0000) >> 16;
        g=(hold & 0x00ff00) >> 8;
        b=(hold & 0x0000ff);
        ret = qGray(r, g, b) >> 4;
        (*srcdata)+=4;
    } else {
        qDebug("Cannot do %d->4!",sdepth);
        ret=0;
    }

    return ret;
}


Q_GFX_INLINE unsigned int QGfxRasterBase::get_value_1(int sdepth, unsigned const char **srcdata,
                                                    bool reverse) //can't be true if sdepth!=depth
{
    unsigned int ret;

    if(sdepth==1) {
        if (reverse) {
            if(monobitcount<8) {
                monobitcount++;
            } else {
                monobitcount=1;
                (*srcdata)--;
                monobitval=**srcdata;
            }
            if(src_little_endian) {
                ret = (monobitval & 0x80) >> 7;
                monobitval=monobitval << 1;
                monobitval=monobitval & 0xff;
            } else {
                ret=monobitval & 0x1;
                monobitval=monobitval >> 1;
            }
        } else {
            if(monobitcount<8) {
                monobitcount++;
            } else {
                monobitcount=1;
                (*srcdata)++;
                monobitval=**srcdata;
            }
            if(src_little_endian) {
                ret=monobitval & 0x1;
                monobitval=monobitval >> 1;
            } else {
                ret = (monobitval & 0x80) >> 7;
                monobitval=monobitval << 1;
                monobitval=monobitval & 0xff;
            }
        }
    } else if(sdepth==32 && is_screen_gfx) {
        unsigned int hold=*reinterpret_cast<unsigned const int *>(*srcdata);
        unsigned int r,g,b;
        r=(hold & 0xff0000) >> 16;
        g=(hold & 0x00ff00) >> 8;
        b=(hold & 0x0000ff);
        (*srcdata)+=4;
        simple_8bpp_alloc=true;
        ret = GFX_CLOSEST_PIXEL(r,g,b);
        simple_8bpp_alloc=false;
    } else {
        qDebug("get_value_1(): Unsupported source depth %d!",sdepth);
        ret=0;
    }

    return ret;
}
// Inline function definitions [end]
//===========================================================================

#endif // QGFXRASTERBASE_QWS_H
