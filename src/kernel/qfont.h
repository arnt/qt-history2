/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.h#9 $
**
** Definition of QFont class
**
** Author  : Eirik Eng
** Created : 940514
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QFONT_H
#define QFONT_H

#include "qwindefs.h"
#include "qstring.h"

struct QFontData;

class QFont
{
friend class QFontMetrics;
friend class QPainter;
public:
    enum StyleHint { Helvetica, Times, Courier, Decorative, System, AnyStyle };
    enum Weight    { Light = 25, Normal = 50, Bold = 75, AnyWeight = -1 };
    enum CharSet   { Latin1, AnyCharSet };

    QFont();				 	    // default font
    QFont( const char *family, int pointSize = 120, // 12.0 default point size
                 int weight = Normal, bool italic = FALSE );
    QFont( const QFont & );
    virtual ~QFont();
    QFont &operator=( const QFont & );

    QFont	copy() const;

    void        setFamily( const char * );
    void        setPointSize( int tenTimesPointSize );
    void        setItalic( bool );
    void        setWeight( int );
    void        setFixedPitch( bool );
    void        setStyleHint( StyleHint );
    void        setCharSet( CharSet );
    
    const char *family()	const;
    int         pointSize()	const;
    bool        italic()	const;
    int         weight()	const;
    bool        fixedPitch()	const;
    StyleHint   styleHint()	const;
    CharSet     charSet()	const;
    bool        exactMatch()	const;

    void        setRawMode( bool );
    bool        rawMode()	const;


    bool	operator==( const QFont &f ) const;
    bool	operator!=( const QFont &f ) const
					{ return !(operator==(f)); }

#if defined(_WS_X11_)
    Font	fontId() const;
#endif

    static void initialize();			// initialize font system
    static void cleanup();			// cleanup font system

protected:
    QString defaultFamily()       { return "helvetica"; };
    QString systemDefaultFamily() { return "helvetica"; };
    QString defaultFont()         { return "6x13"; };

private:
    void init();
    void loadFont() const;

    friend QDataStream &operator<<( QDataStream &, const QFont & );
    friend QDataStream &operator>>( QDataStream &, QFont & );

    QFontData *data;	// font data
};


// --------------------------------------------------------------------------
// QFont stream functions
//

QDataStream &operator<<( QDataStream &, const QFont & );
QDataStream &operator>>( QDataStream &, QFont & );


#endif // QFONT_H

