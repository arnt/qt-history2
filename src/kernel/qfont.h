/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.h#8 $
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


class QFont
{
friend class QFontMetrics;
friend class QPainter;
public:
    enum StyleHint { Helvetica, Times, Courier, Decorative, System, AnyStyle };
    enum Weight    { Light = 25, Normal = 50, Bold = 75, AnyWeight = -1 };
    enum CharSet   { Latin1, ANSI, IBMPC, MAC, AnyCharSet };

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
    
    const char *family() const;
    int         pointSize() const ;
    bool        italic() const ;
    int         weight() const ;
    bool        fixedPitch() const ;
    StyleHint   styleHint() const ;
    CharSet     charSet() const ;
    bool        exactMatch();

    void        setRawMode( bool );
    bool        rawMode() const;


    bool	operator==( const QFont &f ) const;
    bool	operator!=( const QFont &f ) const
					{ return !(operator==(f)); }

#if defined(_WS_X11_)
    Font	fontId() const;
#endif

    static void initialize();			// initialize font system
    static void cleanup();			// cleanup font system

private:
    struct QFontData : QShared {		// font data
	QString   family;
        short     pointSize;
        uint      styleHint     : 8;
        uint      charSet       : 8;
        uint      weight        : 8;
        uint      italic        : 1;
        uint      fixedPitch    : 1;
        uint      hintSetByUser : 1;
        uint      rawMode       : 1;
        uint      dirty         : 1;
        uint      exactMatch    : 1;
#if defined(_WS_WIN_)
	HANDLE	hfont;
#elif defined(_WS_PM_)
#elif defined(_WS_X11_)
	QXFontStruct *f;
#endif
    } *data;

protected:
    QString defaultFamily()       { return "helvetica"; };
    QString systemDefaultFamily() { return "helvetica"; };
    QString defaultFont()         { return "6x13"; };

private:
    void init();
    void loadFont();

    friend QDataStream &operator<<( QDataStream &, const QFont & );
    friend QDataStream &operator>>( QDataStream &, QFont & );
};


// --------------------------------------------------------------------------
// QFont stream functions
//

QDataStream &operator<<( QDataStream &, const QFont & );
QDataStream &operator>>( QDataStream &, QFont & );



inline const char *QFont::family() const
{
    return data->family;
}

inline int QFont::pointSize() const
{
    return data->pointSize;
}

inline bool QFont::italic() const
{
    return data->italic;
}

inline int QFont::weight() const
{
    return (int) data->weight;
}

inline bool QFont::fixedPitch() const
{
    return data->fixedPitch;
}

inline QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) data->styleHint;
}

inline QFont::CharSet QFont::charSet() const
{
    return (CharSet) data->charSet;
}

inline bool QFont::rawMode() const
{
    return data->rawMode;
}


#endif // QFONT_H
