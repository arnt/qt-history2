/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor.h#10 $
**
** Definition of QColor class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCOLOR_H
#define QCOLOR_H

#include "qwindefs.h"


const ulong RGB_DIRTY = 0x80000000;		// flags unset color
const ulong RGB_INVALID = 0x40000000;		// flags invalid color
const ulong RGB_MASK = 0x00ffffff;		// masks RGB values


class QColor					// RGB based color
{
public:
    QColor();					// default RGB=0,0,0
    QColor( const QColor & );			// copy color
    QColor( int r, int g, int b );		// specify RGB
    QColor( const char *name );			// load color from database
   ~QColor();

    static void setAutoAlloc( bool );
    static bool autoAlloc()	{ return aalloc; }

    bool   alloc();				// allocate color

    bool   setNamedColor( const char *name );	// load color from database

    void   getRGB( int *r, int *g, int *b ) const; // get RGB value
    ulong  getRGB() const { return rgb & RGB_MASK; }
    bool   setRGB( int r, int g, int b );	// set RGB value
    bool   setRGB( ulong rgb );

    int	   red()    const { return (int)(rgb & 0xff); }
    int	   green()  const { return (int)((rgb >> 8) & 0xff); }
    int	   blue()   const { return (int)((rgb >> 16) & 0xff); }

    void   getHSV( int *h, int *s, int *v ) const; // get HSV value
    bool   setHSV( int h, int s, int v );	// set HSV value
    
    bool   isValid()const { return (rgb & RGB_INVALID) == 0; }
    bool   isDirty()const { return (rgb & RGB_DIRTY) == RGB_DIRTY; }

    QColor light( int f = 112 ) const;		// get lighter color
    QColor dark( int f = 200 )  const;		// get darker color

    ulong  pixel()  const;			// get pixel value

    bool   operator==( const QColor &c ) const;
    bool   operator!=( const QColor &c ) const;

#if defined(_WS_WIN_)
    static HANDLE hPal()  { return hpal; }
    static uint	  realizePal( QWidget * );
#endif

    static void initialize();			// initialize color system
    static void cleanup();			// cleanup color system

private:
    static bool aalloc;
#if defined(_WS_WIN_)
    static HANDLE hpal;
    ulong  pix;
#elif defined(_WS_PM_)
    ulong  pix;
#elif defined(_WS_X11_)
    ulong  pix;
#endif
    ulong  rgb;					// RGB value
};


#if defined(_WS_WIN_) || defined(_WS_PM_)
inline ulong QColor::pixel() const
{
    return pix;
}
#else
inline ulong QColor::pixel() const
{
    if ( isDirty() )
	((QColor*)this)->alloc();
    return pix;
}
#endif


inline bool QColor::operator==( const QColor &c ) const
{
    return rgb == c.rgb;
}

inline bool QColor::operator!=( const QColor &c ) const
{
    return rgb != c.rgb;
}


// --------------------------------------------------------------------------
// Global colors
//

extern const QColor trueColor;
extern const QColor falseColor;
extern const QColor FFColor;
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
