/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor.h#25 $
**
** Definition of QColor class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCOLOR_H
#define QCOLOR_H

#include "qwindefs.h"


const ulong RGB_DIRTY	= 0x80000000;		// flags unset color
const ulong RGB_INVALID = 0x40000000;		// flags invalid color
const ulong RGB_DIRECT	= 0x20000000;		// flags directly set pixel
const ulong RGB_MASK	= 0x00ffffff;		// masks RGB values


inline int   QRED( ulong rgb )			// get red part of RGB
{ return (int)(rgb & 0xff); }

inline int   QGREEN( ulong rgb )		// get green part of RGB
{ return (int)((rgb >> 8) & 0xff); }

inline int   QBLUE( ulong rgb )			// get blue part of RGB
{ return (int)((rgb >> 16) & 0xff); }

inline ulong QRGB( int r, int g, int b )	// set RGB value
{ return (uchar)r | ((ushort)g << 8) | ((ulong)b << 16); }

inline int QGRAY( int r, int g, int b )		// convert R,G,B to gray 0..255
{ return (r*11+g*16+b*5)/32; }

inline int QGRAY( ulong rgb )			// convert RGB to gray 0..255
{ return QGRAY( QRED(rgb), QGREEN(rgb), QBLUE(rgb) ); }


class QColor					// color class
{
public:
    enum Spec { Rgb, Hsv };

    QColor();
    QColor( int r, int g, int b );
    QColor( int x, int y, int z, Spec );
    QColor( ulong rgb, ulong pixel=0xffffffff);
    QColor( const char *name );
    QColor( const QColor & );
    QColor &operator=( const QColor & );

    bool   isValid() const;
    bool   isDirty() const;

    void   setNamedColor( const char *name );

    void   rgb( int *r, int *g, int *b ) const;
    ulong  rgb()    const;
    void   setRgb( int r, int g, int b );
    void   setRgb( ulong rgb );

    int	   red()    const;
    int	   green()  const;
    int	   blue()   const;

    void   hsv( int *h, int *s, int *v ) const;
    void   setHsv( int h, int s, int v );

    QColor light( int f = 112 ) const;
    QColor dark( int f = 200 )	const;

    bool   operator==( const QColor &c ) const;
    bool   operator!=( const QColor &c ) const;

    static bool lazyAlloc();
    static void setLazyAlloc( bool );
    ulong  alloc();
    ulong  pixel()  const;

    static int maxColors();
    static int numBitPlanes();

#if defined(_WS_WIN_)
    static HANDLE hPal()  { return hpal; }
    static uint	  realizePal( QWidget * );
#endif

    static void initialize();
    static void cleanup();

private:
    static void initglobals();
    static bool ginit;
    static bool lalloc;
#if defined(_WS_WIN_)
    static HANDLE hpal;
#endif
    ulong  pix;
    ulong  rgbVal;
};


inline QColor::QColor()
{ rgbVal = RGB_INVALID; pix = 0; }

inline QColor::QColor( int r, int g, int b )
{ setRgb( r, g, b ); }

inline bool QColor::isValid() const
{ return (rgbVal & RGB_INVALID) == 0; }

inline bool QColor::isDirty() const
{ return (rgbVal & RGB_DIRTY) == RGB_DIRTY; }

inline ulong QColor::rgb() const
{ return rgbVal & RGB_MASK; }

inline int QColor::red() const
{ return QRED(rgbVal); }

inline int QColor::green() const
{ return QGREEN(rgbVal); }

inline int QColor::blue() const
{ return QBLUE(rgbVal); }

#if defined(_WS_WIN_) || defined(_WS_PM_)
inline ulong QColor::pixel() const
{ return pix; }
#else
inline ulong QColor::pixel() const
{ return isDirty() ? ((QColor*)this)->alloc() : pix; }
#endif


inline bool QColor::operator==( const QColor &c ) const
{
    return ((rgbVal | c.rgbVal) & RGB_DIRECT) == 0 ?
		rgbVal == c.rgbVal :
		rgbVal == c.rgbVal && pix == c.pix;
}

inline bool QColor::operator!=( const QColor &c ) const
{
    return ((rgbVal | c.rgbVal) & RGB_DIRECT) == 0 ?
		rgbVal != c.rgbVal :
		rgbVal != c.rgbVal || pix != c.pix;
}

inline bool QColor::lazyAlloc()
{ return lalloc; }


// --------------------------------------------------------------------------
// Global colors
//

extern const QColor color0;
extern const QColor color1;
extern const QColor black;
extern const QColor white;
extern const QColor darkGray;
extern const QColor gray;
extern const QColor lightGray;
extern const QColor red;
extern const QColor green;
extern const QColor blue;
extern const QColor cyan;
extern const QColor magenta;
extern const QColor yellow;
extern const QColor darkRed;
extern const QColor darkGreen;
extern const QColor darkBlue;
extern const QColor darkCyan;
extern const QColor darkMagenta;
extern const QColor darkYellow;


// --------------------------------------------------------------------------
// QColor stream functions
//

QDataStream &operator<<( QDataStream &, const QColor & );
QDataStream &operator>>( QDataStream &, QColor & );


#endif // QCOLOR_H
