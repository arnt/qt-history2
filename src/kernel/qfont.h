/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.h#14 $
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
public:
    enum StyleHint { Helvetica, Times, Courier, OldEnglish,  System, AnyStyle,
                     SansSerif  = Helvetica,
                     Serif      = Times,
                     TypeWriter = Courier,
                     Decorative = OldEnglish,};
    enum Weight	   { Light = 25, Normal = 50, DemiBold = 63,
                     Bold = 75, Black = 87 };
    enum CharSet   { Latin1, AnyCharSet };

    QFont();					    // default font
    QFont( const char *family, int pointSize = 12,  // 12 is default point size
	   int weight = Normal, bool italic = FALSE );
    QFont( const QFont & );
    virtual ~QFont();
    QFont      &operator=( const QFont & );
    QFont	copy() const;

    void	setFamily( const char * );
    void	setPointSize( int pointSize );
    void	setItalic( bool );
    void	setWeight( int );
    void        setUnderline( bool );
    void        setStrikeOut( bool );
    void	setFixedPitch( bool );
    void	setStyleHint( StyleHint );
    void	setCharSet( CharSet );

    const char *family()	const;
    int		pointSize()	const;
    bool	italic()	const;
    int		weight()	const;
    bool        underline()     const;
    bool        strikeOut()     const;
    bool	fixedPitch()	const;
    StyleHint	styleHint()	const;
    CharSet	charSet()	const;
    bool	exactMatch()	const;

    void	setRawMode( bool );
    bool	rawMode()	const;

    bool	operator==( const QFont &f ) const;
    bool	operator!=( const QFont &f ) const
				  { return !(operator==(f)); }

#if defined(_WS_X11_)
    Font	handle() const;
#endif
    bool        dirty() const;
    void        updateSubscribers();

    static const QFont &defaultFont();
    static void  setDefaultFont( const QFont & );
    static void initialize();			// initialize font system
    static void cacheStatistics();		// output cache statistics
    static void cleanup();			// cleanup font system

protected:
    bool    isDefaultFont();
    QString defaultFamily() const;
    QString lastResortFamily() const;
    QString lastResortFont() const;
    int     deciPointSize() const;

private:
    QFont( bool referenceDefaultFont ); // Used by QWidget and QPainter
    QFont( QFontData * );               // used by QFont::copy()
    QFont( const char *family, int pointSize,int weight, bool italic, 
           bool defaultFont );          // used to create default font
    void updateFontInfo() const;
    void init();
    void loadFont() const;

    friend class QFontMetrics;
    friend class QFontInfo;
    friend class QWidget;       // QWidget and QPainter uses a private 
    friend class QPainter;      // constructor and isDefaultFont()
    friend QDataStream &operator<<( QDataStream &, const QFont & );
    friend QDataStream &operator>>( QDataStream &, QFont & );

    QFontData *d;	// font data
    static QFont defFont;
};


// --------------------------------------------------------------------------
// QFont stream functions
//

QDataStream &operator<<( QDataStream &, const QFont & );
QDataStream &operator>>( QDataStream &, QFont & );


#endif // QFONT_H
