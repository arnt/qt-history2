/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.h#32 $
**
** Definition of QFont class
**
** Author  : Eirik Eng
** Created : 940514
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFONT_H
#define QFONT_H

#include "qwindefs.h"
#include "qstring.h"


class  QStrList;
struct QFontData;				// internal font data


class QFont					// font class
{
public:
    enum StyleHint { Helvetica, Times, Courier, OldEnglish,  System, AnyStyle,
		     SansSerif	= Helvetica,
		     Serif	= Times,
		     TypeWriter = Courier,
		     Decorative = OldEnglish};
    enum Weight	   { Light = 25, Normal = 50, DemiBold = 63,
		     Bold  = 75, Black	= 87 };
    enum CharSet   { Latin1, ISO_8859_1 = Latin1, AnyCharSet };

    QFont();					// default font
    QFont( const char *family, int pointSize = 12,
	   int weight = Normal, bool italic = FALSE );
    QFont( const QFont & );
    virtual ~QFont();
    QFont      &operator=( const QFont & );

    const char *family()	const;
    void	setFamily( const char * );
    int		pointSize()	const;
    void	setPointSize( int );
    int		weight()	const;
    void	setWeight( int );
    bool	bold()		const;
    void	setBold( bool );
    bool	italic()	const;
    void	setItalic( bool );
    bool	underline()	const;
    void	setUnderline( bool );
    bool	strikeOut()	const;
    void	setStrikeOut( bool );
    bool	fixedPitch()	const;
    void	setFixedPitch( bool );
    StyleHint	styleHint()	const;
    void	setStyleHint( StyleHint );
    CharSet	charSet()	const;
    void	setCharSet( CharSet );
    bool	rawMode()	const;
    void	setRawMode( bool );

    bool	exactMatch()	const;

    bool	operator==( const QFont & ) const;
    bool	operator!=( const QFont & ) const;

    HANDLE	handle( HANDLE=0 ) const;

    static const QFont &defaultFont();
    static void setDefaultFont( const QFont & );

    static const char  *substitute( const char *familyName );
    static void insertSubstitution( const char *, const char * );
    static void removeSubstitution( const char * );
    static void listSubstitutions( QStrList * );

    static void initialize();
    static void cleanup();
    static void cacheStatistics();

protected:
    bool	dirty()			const;

    QString	defaultFamily()		const;
    QString	lastResortFamily()	const;
    QString	lastResortFont()	const;
    int		deciPointSize()		const;

private:
    QFont( QFontData * );
    QFont( bool );
    void	init();
    void	detach();
    void	updateFontInfo()	const;
    void	loadFont( HANDLE=0 )	const;

    friend class QFontMetrics;
    friend class QFontInfo;
    friend class QPainter;
    friend void *get_tm( QWidget *, QPainter * );
    friend QDataStream &operator<<( QDataStream &, const QFont & );
    friend QDataStream &operator>>( QDataStream &, QFont & );

    QFontData	 *d;				// internal font data
    static QFont *defFont;
};


bool QFont::bold() const
{ return weight() > Normal; }

void QFont::setBold( bool enable )
{ setWeight( enable ? Bold : Normal ); }


// --------------------------------------------------------------------------
// QFont stream functions
//

QDataStream &operator<<( QDataStream &, const QFont & );
QDataStream &operator>>( QDataStream &, QFont & );


#endif // QFONT_H
