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

#include "qgfxraster_qws.h"
#include "qpen.h"
#include "qcolormap.h"
#include "qpaintdevice.h"
//#include "qmemorymanager_qws.h"
#include "qwsdisplay_qws.h"
#include "qgfxdriverfactory_qws.h"
#include "qpixmap.h"
#include "qwidget.h"


#include <stdlib.h>
#include <math.h>

// Used for synchronisation with accelerated drivers (defined in qgfxrasterbase_qws.cpp)
extern volatile int *optype;
extern volatile int *lastop;

#if !defined(_OS_FREEBSD_) && !defined(Q_OS_MAC)
# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  define QT_QWS_REVERSE_BYTE_ENDIANNESS
//#  define QWS_BIG_ENDIAN
# endif
#endif

//===========================================================================
// Utility macros and functions [start]


// Uncomment the following for 1bpp/4bpp displays with a different
// endianness than the processor. This is endianness *within* a byte,
// ie. whether 0x01 is the rightmost or the leftmost pixel on the
// screen.
// This code is still experimental. There are no known bugs, but it
// has not been extensively tested.
//#define QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS


static Q_GFX_INLINE unsigned char *find_pointer_4(unsigned char * base,int x,int y, int w,
                                                int linestep, int &astat, unsigned char &ahold,
                                                bool rev
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                                                , bool reverse_endian
#endif
   )
{
    int nbits;
    int nbytes;

    if (rev) {
        nbits = 1 - (x+w) % 2;
        nbytes = (x+w) / 2;
    } else {
        nbits = x % 2;
        nbytes = x / 2;
    }

    unsigned char *ret = base + (y*linestep) + nbytes;
    astat = nbits;
    ahold = *ret;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
    if (reverse_endian)
        ahold = (ahold & 0x0f) << 4 |  (ahold & 0xf0) >> 4;
#endif

    if (rev)
        ahold = ahold << (nbits*4);
    else
        ahold = ahold >> (nbits*4);

    return ret;
}

// Utility macros and functions [end]
//===========================================================================





/*!
    \class QGfxRaster qgfxraster_qws.h
    \brief The QGfxRaster class is QGfxRasterBase specialized for a particular bit
    depth, specified by the depth parameter of the template. The type field
    is currently not used. In future versions, it may be used to specify the
    pixel storage format.

    \internal (for now)

    \ingroup qws

    Many operations with QGfxRaster are specified along the lines of

    if(depth==32) {
    ...
    } else if(depth==16) {
    ...

    The intention is that the compiler will realise when instantiating a template
    for a particular depth that it need only include the code for that depth -
    so if you never use an 8-bit gfx, for example, the 8-bit code will not
    be included in your executable. The actual drawing code for software-only
    rendering is all included in this class; it should be subclassed by hardware
    drivers so that they can fall back to it for anything that hardware can't
    handle.
*/

/*!
    \fn QGfxRaster::QGfxRaster(unsigned char *b, int w, int h)

    \internal

    Constructs a QGfxRaster for a particular depth with a framebuffer pointed
    to by \a b, with a width and height of \a w and \a h (specified in
    pixels, not bytes)
*/
template <const int depth, const int type>
QGfxRaster<depth,type>::QGfxRaster(unsigned char *b, int w, int h)
    : QGfxRasterBase(b,w,h)
{
    setLineStep((depth*width+7)/8);
}

/*!
    \fn QGfxRaster::~QGfxRaster()

    \internal

    Destroys a QGfxRaster.
*/
template <const int depth, const int type>
QGfxRaster<depth,type>::~QGfxRaster()
{
}

/*!
    \fn void QGfxRaster<depth,type>::calcPacking(void *m, int x1, int x2, int &frontadd, int &backadd, int &count)

    \internal

    This is an internal method used by methods which pack writes to the
    framebuffer for optimisation reasons. It takes longer to write
    80 8-bit values over the PCI or AGP bus to a graphics card than
    it does 20 32-bit values, as long as those 32-bit values are aligned
    on a 32-bit boundary. Therefore the code writes individual pixels
    up to a boundary, writes 32-bit values until it reaches the last boundary
    before the end of the line, and draws then individual pixels again.
    Given a pointer to a start of the line within the framebuffer \a m
    and starting and ending x coordinates \a x1 and \a x2, \a frontadd is filled
    with the number of individual pixels to write at the start of the line,
    \a count with the number of 32-bit values to write and \a backadd with the
    number of individual pixels to write at the end. This optimisation
    yields up to 60% drawing speed performance improvements when Memory
    Type Range Registers are not available, and still gives a few percent
    when write-combining on the framebuffer is enabled using those registers
    (which effectively causes the CPU to do a similar optimisation in hardware)
*/
template<const int depth,const int type>
Q_GFX_INLINE void QGfxRaster<depth,type>::calcPacking(void *m, int x1, int x2,
                                                    int &frontadd, int &backadd, int &count)
{
    // Calculate packing values for 32-bit writes
    int w = x2-x1+1;

#ifndef QWS_NO_WRITE_PACKING
    if (depth == 16) {
        if (w < 2)
            goto unpacked;

        unsigned short int * myptr=(unsigned short int *)m;
        frontadd=(((unsigned long)myptr)+(x1*2)) & 0x3;
        backadd=(((unsigned long)myptr)+((x2+1)*2)) & 0x3;
        if (frontadd)
            frontadd = 4 - frontadd;
        frontadd >>= 1;
        backadd >>= 1;
        count=(w-(frontadd+backadd));
        count >>= 1;
    } else if (depth == 8) {
        if (w < 4)
            goto unpacked;

        unsigned char * myptr=(unsigned char *)m;
        frontadd=(((unsigned long)myptr)+x1) & 0x3;
        backadd=(((unsigned long)myptr)+x2+1) & 0x3;
        if (frontadd)
            frontadd = 4 - frontadd;
        count = w-(frontadd+backadd);
        count >>= 2;
    } else {
        goto unpacked;
    }

    if(count<0)
        count=0;
    if(frontadd<0)
        frontadd=0;
    if(backadd<0)
        backadd=0;
    return;
#endif

unpacked:
    frontadd = w;
    backadd = 0;
    count = 0;
    if(frontadd<0)
        frontadd=0;
}


/*!
    \fn QGfxRaster<depth,type>::setSource(const QImage *i)

    \internal

    This sets up future blt's to use a QImage \a i as a source - used by
    QPainter::drawImage()
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(const QImage *i)
{
    //qWarning("QGfxRaster::setSource(const QImage*)");
    srcpixeltype=QScreen::NormalPixel;
    srclinestep=i->bytesPerLine();
    srcdepth=i->depth();
    if(srcdepth==0)
        abort();
    srcbits=i->scanLine(0);
    src_little_endian= (i->format()==QImage::Format_MonoLSB);

    QSize s = gfx_screen->mapToDevice(QSize(i->width(), i->height()));
    srcwidth=s.width();
    srcheight=s.height();
    src_normal_palette=false;
    if (srcdepth == 1)
        buildSourceClut(0, 0);
    else  if(srcdepth<=8)
        buildSourceClut(const_cast<QRgb*>(i->colorTable().constData()),i->numColors());
}


template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(const QPixmap *pix)
{
    setSource(&pix->toImage()); //### address of temporary, but we know that the implementation makes it safe
}


/*!
    \fn QGfxRaster<depth,type>::setSource(unsigned char *c, int w, int h, int l, int d, QRgb *clut, int numcols)

    \internal

    This sets up future blt's to use \a c as source, and sets up necessary Cluts.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(unsigned char *c, int w, int h, int l, int d, QRgb *clut,
                                       int numcols)
{
    srcpixeltype=QScreen::NormalPixel;
    srclinestep=l;
    srcdepth=d;
    srcbits=c;
    src_little_endian=true;
    srcwidth=w;
    srcheight=h;
    src_normal_palette=false;
    if (srcdepth == 1)
        buildSourceClut(0, 0);
    else  if(srcdepth<=8)
        buildSourceClut(clut,numcols);
}

/*!
    \fn void QGfxRaster::buildSourceClut(const QRgb * cols,int numcols)
    \internal

    This is an internal method used to optimise blt's from paletted to paletted
    data, where the palettes are different for each. A lookup table
    indexed by the source value providing the destination value is
    filled in. \a cols is the source data's color lookup table,
    \a numcols the number of entries in it. If \a cols is 0 some default
    values are put in (this is for 1bpp sources which don't have
    a palette).
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::buildSourceClut(const QRgb * cols,int numcols)
{
    if (!cols) {
        pixel = brushPixel;
#if !defined(QT_NO_IMAGE_16_BIT) || !defined(QT_NO_QWS_DEPTH_16)
        if (qt_screen->depth() == 16 && depth==32) {
            srcclut[0]=qt_conv16ToRgb(pixel);
            transclut[0]=qt_conv16ToRgb(pixel);
        } else
#endif
        {
            srcclut[0]=pixel;
            transclut[0]=pixel;
        }
    }
    int loopc;

    // Copy clut
    for(loopc=0;loopc<numcols;loopc++)
        srcclut[loopc] = cols[loopc];

    if(depth<=8) {
        // Now look for matches
        for(loopc=0;loopc<numcols;loopc++) {
            int r = qRed(srcclut[loopc]);
            int g = qGreen(srcclut[loopc]);
            int b = qBlue(srcclut[loopc]);
            transclut[loopc] = gfx_screen->alloc(r,g,b);
        }
    }
}



/*!
    \fn void QGfxRaster<depth,type>::hline(int x1, int x2, int y)

    \internal

    Draw a line at coordinate \a y from \a x1 to \a x2 - used by the polygon
    drawing code. Performs clipping.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::hline(int x1, int x2, int y) //screen coordinates, clipped, x1<=x2
{
    // gross clip.
    if (x1 > clipbounds.right() || x2 < clipbounds.left())
        return;
    if (x1 < clipbounds.left())
        x1 = clipbounds.left();
    if (x2 > clipbounds.right())
        x2 = clipbounds.right();

    QRect cr;
    unsigned char *l = scanLine(y);
    bool plot = inClip(x1, y, &cr);
    int x=x1;
    for (;;) {
        int xr = cr.right();
        if (xr >= x2) {
            if (plot)
                hlineUnclipped(x,x2,l);
            break;
        } else {
            if (plot)
                hlineUnclipped(x,xr,l);
            x=xr+1;
            plot=inClip(x,y,&cr,plot);
        }
    }
}


static inline void drawPoint_4(int x, unsigned char *l, uint pixel)
{
    uchar *d = l + (x>>1);
    int s = (x & 1) << 2;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
    if (is_screen_gfx)
        s = (~x & 1) << 2;
#endif
    *d = (*d & MASK4BPP(s)) | (pixel << s);
}
/*!
    \fn void QGfxRaster<depth,type>::hlineUnclipped(int x1, int x2, unsigned char *l)

    \internal

    Draws a line in the current pen color from \a x1 to \a x2 on scanline \a l,
    ignoring clipping. Used by anything that draws in solid colors - drawLine,
    fillRect, and drawPolygon.
*/
template <const int depth, const int type> //screen coordinates, unclipped, x1<=x2, x1>=0
Q_GFX_INLINE void QGfxRaster<depth,type>::hlineUnclipped(int x1, int x2, unsigned char *l)
{
    int w = x2-x1+1;

    if (depth == 32) {
        unsigned int *myptr=(unsigned int *)l + x1;
        while (w--)
            *(myptr++) = pixel;
    } else if (depth == 24) {
        unsigned char *myptr = l + x1*3;
        while (w--) {
            gfxSetRgb24(myptr, pixel);
            myptr += 3;
        }
    } else if (depth == 16) {
        unsigned short int *myptr=(unsigned short int *)l;
#ifdef QWS_NO_WRITE_PACKING
        myptr+=x1;
        while (w--)
            *(myptr++) = pixel;
#else
        int frontadd;
        int backadd;
        int count;
        calcPacking(myptr,x1,x2,frontadd,backadd,count);

        myptr+=x1;

        PackType put = pixel | (pixel << 16);

        while (frontadd--)
            *(myptr++)=pixel;
        // Duffs device.
        PackType *myptr2 = (PackType*)myptr;
        myptr += count * 2;
        PackType *end2 = (PackType*)myptr;
        switch(count%8){
            case 0:
                while (myptr2 != end2) {
                    *myptr2++ = put;
            case 7: *myptr2++ = put;
            case 6: *myptr2++ = put;
            case 5: *myptr2++ = put;
            case 4: *myptr2++ = put;
            case 3: *myptr2++ = put;
            case 2: *myptr2++ = put;
            case 1: *myptr2++ = put;
                }
        }
        while (backadd--)
            *(myptr++)=pixel;
#endif
    } else if (depth == 8) {
        unsigned char *myptr=l;
#ifdef QWS_NO_WRITE_PACKING
        myptr+=x1;
        while (w--)
            *(myptr++) = pixel;
#else
        int frontadd,backadd,count;
        calcPacking(myptr,x1,x2,frontadd,backadd,count);

        myptr+=x1;

        PackType put = pixel | (pixel << 8)
            | (pixel << 16) | (pixel << 24);

        while (frontadd--)
            *(myptr++)=pixel;
        // Duffs device.
        PackType *myptr2 = (PackType*)myptr;
        myptr += count * 4;
        PackType *end2 = (PackType*)myptr;
        switch(count%8){
            case 0:
                while (myptr2 != end2) {
                    *myptr2++ = put;
                    case 7: *myptr2++ = put;
                    case 6: *myptr2++ = put;
                    case 5: *myptr2++ = put;
                    case 4: *myptr2++ = put;
                    case 3: *myptr2++ = put;
                    case 2: *myptr2++ = put;
                    case 1: *myptr2++ = put;
                }
        }
        while (backadd--)
            *(myptr++)=pixel;
#endif
    } else if (depth == 4) {
        unsigned char *myptr=l;
        unsigned char *dptr = myptr + x1/2;
        if (x1&1) {
            drawPoint_4(x1, myptr, pixel);
            w--;
            dptr++;
        }

        unsigned char val = pixel | (pixel << 4);
        while (w > 1) {
            *dptr++ = val;
            w -= 2;
        }

        if (!(x2&1))
            drawPoint_4(x2, myptr, pixel);
    } else if (depth == 1) {
        //#### we need to use semaphore
        l += x1/8;
        if (x1/8 == x2/8) {
            // Same byte

            uchar mask = (0xff << (x1 % 8)) & (0xff >> (7 - x2 % 8));
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
            mask = (0xff >> (x1 % 8)) & (0xff << (7 - x2 % 8));
#endif
            if (pixel)
                *l |= mask;
            else
                *l &= ~mask;
        } else {
            volatile unsigned char *last = l + (x2/8-x1/8);
            uchar mask = 0xff << (x1 % 8);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
                mask = 0xff >> (x1 % 8);
#endif
            if (pixel)
                *l++ |= mask;
            else
                *l++ &= ~mask;
            unsigned char byte = pixel ? 0xff : 0x00;
            while (l < last)
                *l++ = byte;

            mask = 0xff >> (7 - x2 % 8);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
                mask = 0xff << (7 - x2 % 8);
#endif
            if (pixel)
                *l |= mask;
            else
                *l &= ~mask;
        }
    }
}

/*!
    \fn void QGfxRaster<depth,type>::hImageLineUnclipped(int x1,int x2, unsigned char *l, unsigned const char *srcdata, bool reverse)

    \internal

    \a l points to the start of the destination line's data.
    \a x1 and \a x2 are the start and end pixels.
    \a srcdata points to the source's left pixel start byte if \a reverse is
    false. srcdata points to the source's right pixels's start byte if reverse
    is true. reverse will only be true if the source and destination are the same
    buffer and a mask is set.
    Image data comes from of the setSource calls (in which case the
    variable srcdata points to it) or as a solid value stored in pixel
    (if setSourcePen is used). This method is internal and called from blt and
    stretchBlt. Its complexity is caused by its need to deal with masks, copying
    from right to left or left to right (for overlapping blt's), packing writes
    for performance reasons and arbitrary source and destination depths.
*/
template <const int depth, const int type>
Q_GFX_INLINE void QGfxRaster<depth,type>::hImageLineUnclipped(int x1, int x2, unsigned char *l,
                                                              unsigned const char *srcdata,
                                                              bool reverse)
{
    int w = x2-x1+1;
    if (depth == 32) {
        unsigned int *myptr=(unsigned int *)l;
        int inc = 1;
        if(!reverse) {
            myptr+=x1;
        } else {
            myptr+=x2;
            inc = -1;
        }

        while (w--) {
            uint gv = get_value_32(srcdepth,&srcdata);
            *(myptr++) = gv;
        }

    } else if (depth == 24) {
        unsigned char *myptr = l;
        int inc = 3;
        if(!reverse) {
            myptr += x1*3;
        } else {
            myptr += x2*3;
            inc = -3;
        }

        while (w--) {
            uint gv = get_value_24(srcdepth,&srcdata);
            gfxSetRgb24(myptr, gv);
            myptr += 3;
        }

    } else if (depth == 16) {
        unsigned short int *myptr=(unsigned short int *)l;
        int inc = 1;
        if(!reverse) {
            myptr+=x1;
        } else {
            myptr+=x2;
            inc = -1;
        }

#ifdef QWS_NO_WRITE_PACKING
        while (w--) {
            *(myptr++)=get_value_16(srcdepth,&srcdata);
        }
#else
        // 32-bit writes
        int frontadd;
        int backadd;
        int count;

        calcPacking(myptr-x1,x1,x2,frontadd,backadd,count);

        PackType dput;
        while (frontadd--)
            *(myptr++)=get_value_16(srcdepth,&srcdata);
        PackType *myptr2 = (PackType*)myptr;
        myptr += count * 2;
        while (count--) {
# ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
            dput = (get_value_16(srcdepth,&srcdata) << 16);
            dput |= get_value_16(srcdepth,&srcdata);
# else
            dput = get_value_16(srcdepth,&srcdata);
            dput |= (get_value_16(srcdepth,&srcdata) << 16);
# endif
            *myptr2++ = dput;
        }
        while (backadd--)
            *(myptr++)=get_value_16(srcdepth,&srcdata);
#endif

    } else if (depth == 8) {
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        // cursor code uses 8bpp backing store
        if (srcdepth == 4 && srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
        unsigned char *myptr=(unsigned char *)l;
        int inc = 1;
        if(!reverse) {
            myptr+=x1;
        } else {
            myptr+=x2;
            inc = -1;
        }

#ifdef QWS_NO_WRITE_PACKING
        while (w--)
            *(myptr++)=get_value_8(srcdepth,&srcdata);
#else
        // 32-bit writes
        int frontadd;
        int backadd;
        int count;

        calcPacking(myptr-x1,x1,x2,frontadd,backadd,count);

        PackType dput;
        while (frontadd--)
            *(myptr++)=get_value_8(srcdepth,&srcdata);
        while (count--) {
#ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
            dput = (get_value_8(srcdepth,&srcdata) << 24);
            dput |= (get_value_8(srcdepth,&srcdata) << 16);
            dput |= (get_value_8(srcdepth,&srcdata) << 8);
            dput |= get_value_8(srcdepth,&srcdata);
#else
            dput = get_value_8(srcdepth,&srcdata);
            dput |= (get_value_8(srcdepth,&srcdata) << 8);
            dput |= (get_value_8(srcdepth,&srcdata) << 16);
            dput |= (get_value_8(srcdepth,&srcdata) << 24);
#endif
            *((PackType*)myptr) = dput;
            myptr += 4;
        }
        while (backadd--)
            *(myptr++)=get_value_8(srcdepth,&srcdata);
#endif

#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //restore
        if (srcdepth == 4 && srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
    } else if (depth == 4) {
        unsigned char *dp = l;
        unsigned int gv = pixel;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //really ugly hack: screen is opposite endianness to everything else
        if (srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
        if (reverse) {
            dp += (x2/2);
            int x = x2;
            while (w--) {
                gv = get_value_4(srcdepth, &srcdata, reverse);
                int s = (x&1) << 2;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                if (is_screen_gfx)
                    s = (~x&1) << 2;
#endif
                *dp = (*dp & MASK4BPP(s)) | (gv << s);

                if (!(x&1))
                    dp--;
                x--;
            }
        } else {
            dp += (x1/2);
            int x = x1;
            while (w--) {
                gv = get_value_4(srcdepth, &srcdata, reverse);

                int s = (x&1) << 2;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                if (is_screen_gfx)
                    s = (~x&1) << 2;
#endif
                *dp = (*dp & MASK4BPP(s)) | (gv << s);

                if (x&1)
                    dp++;
                x++;
            }
        }
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //end of really ugly hack; undo the damage
        if (srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
    } else if (depth == 1) {
        // General case only implemented.
        // Lots of optimisation can be performed for special cases.
        unsigned char * dp=l;
        unsigned int gv = pixel;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //really ugly hack: screen is opposite endianness to everything else
        if (srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
        if (reverse) {
            dp+=(x2/8);
            int skipbits = 7 - (x2%8);
            for (int b = x1/8; b <= x2/8; b++) {
                unsigned char m = *dp;
                for (int i = 0; i < 8 && w; i++) {
                    if (skipbits)
                        skipbits--;
                    else {
                        w--;
                        gv = get_value_1(srcdepth, &srcdata, true);

#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                        if (is_screen_gfx) {
                            if (gv)
                                m |= 0x01 << i;
                            else
                                m &= ~(0x01 << i);
                        } else
#endif
                            if (gv)
                                m |= 0x80 >> i;
                            else
                                m &= ~(0x80 >> i);

                    }
                }
                *(dp--) = m;
            }
        } else {
            dp+=(x1/8);
            int skipbits = x1%8;
            for (int b = x1/8; b <= x2/8; b++) {
                unsigned char m = *dp;
                for (int i = 0; i < 8 && w; i++) {
                    if (skipbits)
                        skipbits--;
                    else {
                        w--;
                        gv = get_value_1(srcdepth, &srcdata, false);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                        if (is_screen_gfx) {
                            if (gv)
                                m |= 0x80 >> i;
                            else
                                m &= ~(0x80 >> i);
                        } else
#endif
                            if (gv)
                                m |= 1 << i;
                            else
                                m &= ~(1 << i);
                    }
                }
                *(dp++) = m;
            }
        }
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //end of really ugly hack; undo the damage
        if (srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
    }
}

/*!
    \fn void QGfxRaster<depth,type>::fillRect(int rx, int ry, int w, int h)

    \internal

    Draw a filled rectangle in the current brush color from \a rx,\a ry to \a w,
    \a h.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::fillRect(int rx,int ry,int w,int h) //widget coordinates
{
    if (!ncliprect)
        return;
    GFX_START(QRect(rx, ry, w+1, h+1))

    if (w <= 0 || h <= 0) {
        GFX_END
        return;
    }

#ifdef QWS_EXPERIMENTAL_FASTPATH
    // ### fix for 8bpp
    // This seems to be reliable now, at least for 16bpp

    if (ncliprect == 1) {
        // Fast path
        if(depth==16) {
            pixel = brushPixel;
            int x1,y1,x2,y2;
            x2=rx+w-1;
            y2=ry+h-1;
            if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
               x2<cliprect[0].left() || y2<cliprect[0].top()) {
                GFX_END
                return;
            }
            x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
            y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
            x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
            y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();
            w=(x2-x1)+1;
            h=(y2-y1)+1;

            if(w<1 || h<1) {
                GFX_END
                return;
            }

            unsigned short int * myptr=(unsigned short int *)scanLine(y1);

        // 64-bit writes make a /big/ difference from 32-bit ones,
        // at least on my (fast AGP) hardware - 856 rects/second as opposed
        // to 550, although MTRR makes this difference much less
            int frontadd;
            int backadd;
            int count;
            calcPacking(myptr,x1,x2,frontadd,backadd,count);

            int loopc,loopc2;

            PackType put;
            put = pixel | (pixel << 16);
            int add=linestep()/2;
            add-=(frontadd+(count * 2)+backadd);

            myptr=((unsigned short int *)scanLine(y1))+x1;
            for(loopc=0;loopc<h;loopc++) {
                for(loopc2=0;loopc2<frontadd;loopc2++)
                    *(myptr++)=pixel;
                // Duffs device.
                PackType *myptr2 = (PackType*)myptr;
                myptr += count * 2;
                PackType *end2 = (PackType*)myptr;
                switch(count%8) {
                case 0:
                    while (myptr2 != end2) {
                        *myptr2++ = put;
                case 7: *myptr2++ = put;
                case 6: *myptr2++ = put;
                case 5: *myptr2++ = put;
                case 4: *myptr2++ = put;
                case 3: *myptr2++ = put;
                case 2: *myptr2++ = put;
                case 1: *myptr2++ = put;
                                           }
                }
                for(loopc2=0;loopc2<backadd;loopc2++)
                    *(myptr++)=pixel;
                myptr+=add;
            }
            GFX_END
            return;
        } else if(depth==32) {
            pixel = brushPixel;
            int x1,y1,x2,y2;
            x2=rx+w-1;
            y2=ry+h-1;
            if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
               x2<cliprect[0].left() || y2<cliprect[0].top()) {
                GFX_END
                return;
            }
            x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
            y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
            x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
            y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();
            w=(x2-x1)+1;
            h=(y2-y1)+1;

            if(w<1 || h<1) {
                GFX_END
                return;
            }

            unsigned int * myptr=(unsigned int *)scanLine(y1);

            int frontadd;
            int backadd;
            int count;
            calcPacking(myptr,x1,x2,frontadd,backadd,count);

            int loopc,loopc2;
            PackType put;
            unsigned int * sp=(unsigned int *)&put;
            *sp=pixel;
            int add=linestep()/4;
            add-=(frontadd+(count * 2)+backadd);

            myptr=((unsigned int *)scanLine(y1))+x1;

            for(loopc=0;loopc<h;loopc++) {
                for(loopc2=0;loopc2<frontadd;loopc2++)
                    *(myptr++)=pixel;
                for(loopc2=0;loopc2<count;loopc2++) {
                    *((PackType *)myptr)=put;
                    myptr+=2;
                }
                for(loopc2=0;loopc2<backadd;loopc2++)
                    *(myptr++)=pixel;
                myptr+=add;
            }
            GFX_END
            return;
        } else if(depth==8) {
            pixel = brushPixel;
            int x1,y1,x2,y2;
            x2=rx+w-1;
            y2=ry+h-1;
            if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
               x2<cliprect[0].left() || y2<cliprect[0].top()) {
                GFX_END
                return;
            }
            x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
            y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
            x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
            y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();
            w=(x2-x1)+1;
            h=(y2-y1)+1;

            if(w<1 || h<1) {
                GFX_END
                return;
            }

            unsigned char * myptr=(unsigned char *)scanLine(y1);

            int frontadd;
            int backadd;
            int count;
            calcPacking(myptr,x1,x2,frontadd,backadd,count);

            int loopc,loopc2;
            PackType put;
            if (count)
                put = pixel | (pixel<<8) | (pixel<<16) | (pixel<<24);

            int add=linestep();
            add-=(frontadd+(count * 4)+backadd);

            myptr=((unsigned char *)scanLine(y1))+x1;

            for(loopc=0;loopc<h;loopc++) {
                for(loopc2=0;loopc2<frontadd;loopc2++)
                    *(myptr++)=pixel;
                for(loopc2=0;loopc2<count;loopc2++) {
                    *((PackType *)myptr)=put;
                    myptr+=4;
                }
                for(loopc2=0;loopc2<backadd;loopc2++)
                    *(myptr++)=pixel;
                myptr+=add;
            }
            GFX_END
            return;
        } else {

        }
    }
#endif // QWS_EXPERIMENTAL_FASTPATH
    if(cbrush.style()!=Qt::NoBrush) {
        pixel = brushPixel;
        // Gross clip
        if (rx < clipbounds.left()) {
            w -= clipbounds.left()-rx;
            rx = clipbounds.left();
        }
        if (ry < clipbounds.top()) {
            h -= clipbounds.top()-ry;
            ry = clipbounds.top();
        }
        if (rx+w-1 > clipbounds.right())
            w = clipbounds.right()-rx+1;
        if (ry+h-1 > clipbounds.bottom())
            h = clipbounds.bottom()-ry+1;
        if (w > 0 && h > 0)
            for (int j=0; j<h; j++,ry++) {
                hline(rx,rx+w-1,ry); }
    }
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::blt(int rx, int ry, int w, int h, int sx, int sy)

    \internal

    This corresponds to QPixmap::drawPixmap (into a QPainter with no transformation
    other than a translation) or bitBlt. The source is set up using
    setSource and setSourceWidgetOffset before the blt. \a rx and \a ry are the
    destination coordinates, \a w and \a h the size of the rectangle to blt,
    \a sx and \a sy the source coordinates relative to the source's widget offset.
    In the case of a pen source sx and sy are ignored. Source and destination
    can overlap and can be of arbitrary (different) depths.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::blt(int rx,int ry,int w,int h, int sx, int sy)
{
    if (!w || !h) return;
    int osrcdepth=srcdepth;

    QRect cursRect(rx, ry, w+1, h+1);
    GFX_START(cursRect&clipbounds);

    // Very gross clip
    if (!clipbounds.intersects(QRect(rx,ry,w,h))) {
        GFX_END
            return;
    }

    //slightly tighter clip
    int leftd = clipbounds.x() - rx;
    if (leftd > 0) {
        rx += leftd;
        sx += leftd;
        w -= leftd;
    }
    int topd =  clipbounds.y() - ry;
    if (topd > 0) {
        ry += topd;
        sy += topd;
        h -= topd;
    }
    int rightd = rx + w - 1 - clipbounds.right();
    if (rightd > 0)
        w -= rightd;
    int botd = ry + h - 1 - clipbounds.bottom();
    if (botd > 0)
        h -= botd;

    // have we already clipped away everything necessary
    bool mustclip = ncliprect != 1;

    QPoint srcoffs = QPoint(sx, sy);

    int dl = linestep();
    int sl = srclinestep;
    int dj = 1;
    int dry = 1;
    int tj;
    int j;
    if (srcbits == buffer && srcoffs.y() < ry) {
        // Back-to-front
        dj = -dj;
        dl = -dl;
        sl = -sl;
        dry = -dry;
        j = h-1;
        ry=(ry+h)-1;
        tj = -1;
    } else {
        j = 0;
        tj = h;
    }

    bool xrev = (srcbits == buffer && srcoffs.x() < rx);

    QRect cr(rx, ry, w, h);

    unsigned char *l = scanLine(ry);
    unsigned const char *srcline = srcScanLine(j+srcoffs.y());
    int right = rx+w-1;

    // Fast path for 8/16/32 bit same-depth opaque blit. (ie. the common case)
    if (srcdepth == depth
        && pixeltype == srcpixeltype
        && (depth > 8 || (depth == 8 && src_normal_palette))) {
        int bytesPerPixel = depth/8;
        if (mustclip) {
            if (xrev) {
                for (; j!=tj; j+=dj,ry+=dry,l+=dl,srcline+=sl) {
                    bool plot = inClip(right,ry,&cr);
                    int x2=right;
                    for (;;) {
                        int x = cr.left();
                        if (x < rx) {
                            x = rx;
                            if (x2 < x) break;
                        }
                        if (plot) {
                            unsigned const char *srcptr=srcline+(x-rx+srcoffs.x())*bytesPerPixel;
                            unsigned char *destptr = l + x*bytesPerPixel;
                            memmove(destptr, srcptr, (x2-x+1) * bytesPerPixel);
                        }
                        if (x <= rx)
                            break;
                        x2=x-1;
                        plot=inClip(x2,ry,&cr,plot);
                    }
                }
            } else {
                for (; j!=tj; j+=dj,ry+=dry,l+=dl,srcline+=sl) {
                    bool plot = inClip(rx,ry,&cr);
                    int x=rx;
                    for (;;) {
                        int x2 = cr.right();
                        if (x2 > right) {
                            x2 = right;
                            if (x2 < x) break;
                        }
                        if (plot) {
                            unsigned const char *srcptr=srcline+(x-rx+srcoffs.x())*bytesPerPixel;
                            unsigned char *destptr = l + x*bytesPerPixel;
                            memmove(destptr, srcptr, (x2-x+1) * bytesPerPixel);
                        }
                        x=x2+1;
                        if (x > right)
                            break;
                        plot=inClip(x,ry,&cr,plot);
                    }
                }
            }
        } else {
            unsigned const char *srcptr = srcline + srcoffs.x()*bytesPerPixel;
            unsigned char *destptr = l + rx*bytesPerPixel;
            int bytes = w * bytesPerPixel;
            for (; j!=tj; j+=dj,destptr+=dl,srcptr+=sl) {
                memmove(destptr, srcptr, bytes);
            }
        }
    } else {

        // reverse will only ever be true if the source and destination
        // are the same buffer.
        bool reverse = srcoffs.y()==ry && rx>srcoffs.x() &&
                       srcbits == buffer;

        unsigned const char *srcptr = 0;
        for (; j!=tj; j+=dj,ry+=dry,l+=dl,srcline+=sl) {
            bool plot = mustclip ? inClip(rx,ry,&cr) : true;
            int x=rx;
            for (;;) {
                int x2 = cr.right();
                if (x2 > right) {
                    x2 = right;
                    if (x2 < x) break;
                }
                if (plot) {
                    if (srcdepth == 1) {
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                        if  (srcbits == qt_screen->base())
                            src_little_endian =  !src_little_endian;
#endif
                        srcptr=find_pointer(srcbits,(x-rx)+srcoffs.x(),
                                            j+srcoffs.y(), x2-x, srclinestep,
                                            monobitcount, monobitval,
                                            !src_little_endian, reverse);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                        if  (srcbits == qt_screen->base())
                            src_little_endian =  !src_little_endian;
#endif
                    } else if (srcdepth == 4) {
                        srcptr = find_pointer_4(const_cast<unsigned char*>(srcbits),(x-rx)+srcoffs.x(),
                                                j+srcoffs.y(), x2-x, srclinestep,
                                                monobitcount, monobitval, reverse
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                                                , srcbits == qt_screen->base()
#endif
                            );
                    } else if (reverse)
                        srcptr = srcline + (x2-rx+srcoffs.x())*srcdepth/8;
                    else
                        srcptr = srcline + (x-rx+srcoffs.x())*srcdepth/8;
                    hImageLineUnclipped(x,x2,l,srcptr,reverse);
                }
                x=x2+1;
                if (x > right)
                    break;
                if (mustclip)
                    plot=inClip(x,ry,&cr,plot);
            }
        }
    }

    srcdepth=osrcdepth;
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::tiledBlt(int rx,int ry,int w,int h)

    \internal

    Like scroll(), this is intended as a candidate for hardware acceleration
    - it's a special case of blt where the source can be a different size
    to the destination and is tiled across the destination. \a rx and \a ry
    specify the x and y position of the rectangle to fill, \a w and \a h
    its size.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::tiledBlt(int rx,int ry,int w,int h)
{
    if (srcwidth == 0 || srcheight == 0)
        return;
    GFX_START(QRect(rx, ry, w+1, h+1))

    pixel = brushPixel;

    int offx =  rx;
    int offy = ry;

    // from qpainter_qws.cpp
    if (offx < 0)
        offx = srcwidth - -offx % srcwidth;
    else
        offx = offx % srcwidth;
    if (offy < 0)
        offy = srcheight - -offy % srcheight;
    else
        offy = offy % srcheight;

    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = ry;
    yOff = offy;
    while(yPos < ry + h) {
        drawH = srcheight - yOff;    // Cropping first row
        if (yPos + drawH > ry + h)        // Cropping last row
            drawH = ry + h - yPos;
        xPos = rx;
        xOff = offx;
        while(xPos < rx + w) {
            drawW = srcwidth - xOff; // Cropping first column
            if (xPos + drawW > rx + w)    // Cropping last column
                drawW = rx + w - xPos;
            blt(xPos, yPos, drawW, drawH,xOff,yOff);
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
    GFX_END
}







// explicit template instaniation
#ifndef QT_NO_QWS_DEPTH_1
template class QGfxRaster<1,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_4
template class QGfxRaster<4,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_8
template class QGfxRaster<8,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_16
template class QGfxRaster<16,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_24
template class QGfxRaster<24,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_32
template class QGfxRaster<32,0>;
#endif


//##### this function must be in the gfx file, since all the template functions are inline




/*!
    Creates a gfx on an arbitrary buffer \a bytes, width \a w and height \a h in
    pixels, depth \a d and \a linestep (length in bytes of each line in the
    buffer). Accelerated drivers can check to see if \a bytes points into
    graphics memory and create an accelerated Gfx.
*/

QGfx * QScreen::createGfx(unsigned char * bytes,int w,int h,int d, int linestep)
{
    QGfx* ret;
    if (false) {
        //Just to simplify the ifdeffery
#ifndef QT_NO_QWS_DEPTH_1
    } else if(d==1) {
        ret = new QGfxRaster<1,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_4
    } else if(d==4) {
        ret = new QGfxRaster<4,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if(d==8) {
        ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
        ret = new QGfxRaster<16,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_24
    } else if(d==24) {
        ret = new QGfxRaster<24,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_32
    } else if(d==32) {
        ret = new QGfxRaster<32,0>(bytes,w,h);
#endif
    } else {
        qFatal("Can't drive depth %d",d);
        ret = 0; // silence gcc
    }
    ret->setLineStep(linestep);
    return ret;
}
