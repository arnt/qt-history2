/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor.h#3 $
**
** Definition of QColor class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QCOLOR_H
#define QCOLOR_H

#include "qwindefs.h"


const ulong RGB_INVALID = 0x80000000;		// flags invalid color


class QColor					// RGB based color
{
public:
    QColor();					// default RGB=0,0,0
    QColor( const QColor & );			// copy color
    QColor( int r, int g, int b );		// specify RGB
    QColor( const char *name );			// load color from database
   ~QColor();

    bool   setRGB( int r, int g, int b );	// set RGB value
    bool   setRGB( ulong rgb );
    ulong  getRGB() const { return rgb & 0x7fffffff; }

    int	   red()    const { return (int)(rgb & 0xff); }
    int	   green()  const { return (int)((rgb >> 8) & 0xff); }
    int	   blue()   const { return (int)((rgb >> 16) & 0xff); }

    bool   isValid()const { return !(rgb & 0x80000000); }

    QColor light( double f = 1.12) const;	// get lighter color
    QColor dark( double f = 2.0 )  const;	// get darker color

#if defined(_WS_WIN_) || defined(_WS_WIN32_)
    ulong  pixel()  const { return pix; }
    static HANDLE hPal()  { return hpal; }
    static uint	  realizePal( QWidget * );
#elif defined(_WS_PM_)
    ulong  pixel()  const { return pix; }
#elif defined(_WS_X11_)
    ulong  pixel()  const { return pix; }	// get X11 pixel value
#endif

    static void initialize();			// initialize color system
    static void cleanup();			// cleanup color system

private:
#if defined(_WS_WIN_) || defined(_WS_WIN32_)
    static HANDLE hpal;
    ulong  pix;
#elif defined(_WS_PM_)
    ulong  pix;
#elif defined(_WS_X11_)
    ulong  pix;
#endif
    ulong  rgb;					// RGB value
};


// --------------------------------------------------------------------------
// Global colors
//

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
