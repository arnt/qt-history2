/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.h#73 $
**
** Definition of QFont class
**
** Created : 940514
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QFONT_H
#define QFONT_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qstring.h"
#endif // QT_H


class QFontPrivate;                                     /* don't touch */
class QStringList;

class Q_EXPORT QFont
{
public:
    enum StyleHint {
	Helvetica,  SansSerif = Helvetica,
	Times,      Serif = Times,
	Courier,    TypeWriter = Courier,
	OldEnglish, Decorative = OldEnglish,
	System,
	AnyStyle
    };

    enum StyleStrategy {
	PreferDefault = 0x0001,
	PreferBitmap  = 0x0002,
	PreferDevice  = 0x0004,
	PreferOutline = 0x0008,
	ForceOutline  = 0x0010,
	PreferMatch   = 0x0020,
	PreferQuality = 0x0040
    };

    enum Weight {
	Light    = 25,
	Normal   = 50,
	DemiBold = 63,
	Bold     = 75,
	Black	 = 87
    };

    // default font
    QFont();
    // specific font
    QFont( const QString &family, int pointSize = 12, int weight = Normal,
	   bool italic = FALSE );
    // copy constructor
    QFont( const QFont & );

    ~QFont();

    QString family() const;
    void setFamily( const QString &);

    int pointSize() const;
    float pointSizeFloat() const;
    void setPointSize( int );
    void setPointSizeFloat( float );

    int pixelSize() const;
    void setPixelSize( int );
    void setPixelSizeFloat( float );

    int weight() const;
    void setWeight( int );

    bool bold() const;
    void setBold( bool );

    bool italic() const;
    void setItalic( bool );

    bool underline() const;
    void setUnderline( bool );

    bool strikeOut() const;
    void setStrikeOut( bool );

    bool fixedPitch() const;
    void setFixedPitch( bool );

    StyleHint styleHint() const;
    StyleStrategy styleStrategy() const;
    void setStyleHint( StyleHint, StyleStrategy = PreferDefault );

    // is raw mode still needed?
    bool rawMode() const;
    void setRawMode( bool );

    // dupicated from QFontInfo
    bool exactMatch() const;

    QFont &operator=( const QFont & );
    bool operator==( const QFont & ) const;
    bool operator!=( const QFont & ) const;
    bool isCopyOf( const QFont & ) const;


#ifdef Q_WS_WIN
    HFONT handle() const;
#else // !Q_WS_WIN
    Qt::HANDLE handle() const;
#endif // Q_WS_WIN


    // needed for X11
    void setRawName( const QString & );
    QString rawName() const;

    QString key() const;

    QString toString() const;
    bool fromString(const QString &);

    static QString substitute(const QString &);
    static QStringList substitutes(const QString &);
    static QStringList substitutions();
    static void insertSubstitution(const QString&, const QString &);
    static void insertSubstitutions(const QString&, const QStringList &);
    static void removeSubstitution(const QString &);

    static void initialize();
    static void cleanup();
    static void cacheStatistics();


#if defined(Q_WS_QWS)
    void qwsRenderToDisk(bool all=TRUE);
#endif

    enum Script {
	// Basic Latin with Latin-1 Supplement
	BasicLatin,

	LatinExtendedA,
	LatinExtendedB, // TODO
	IPAExtensions,
	LatinExtendedAdditional,
	LatinLigatures,

	Diacritical,

	Greek,
	GreekExtended,
	Cyrillic,
	CyrillicHistoric,
	CyrillicExtended,
	Armenian,
	Georgian,
	Runic,
	Ogham,

	Hebrew,
	HebrewPresentation,
	Arabic,
	ArabicPresentationA,
	ArabicPresentationB,
	Syriac,
	Thaana,

	Devanagari,
	Bengali,
	Gurmukhi,
	Gujarati,
	Oriya,
	Tamil,
	Telugu,
	Kannada,
	Malayalam,
	Sinhala,
	Thai,
	Lao,
	Tibetan,
	Myanmar,
	Khmer,

	UnifiedHan,
	Hiragana,
	Katakana,
	Hangul,
	Bopomofo,
	Yi,

	Ethiopic,
	Cherokee,
	CanadianAboriginal,
	Mongolian,

	// this one is only used on X11 to get some char displayed for all of
	// the Han area.
	UnifiedHanX11,

	// To get Latin Extended-A characters from various ISO-8859-* encodings
	// Extended Latin from ISO-8859-2
	LatinExtendedA_2 = LatinExtendedA,
	// Extended Latin from ISO-8859-3
	LatinExtendedA_3 = UnifiedHanX11 + 1,
	// Extended Latin from ISO-8859-4
	LatinExtendedA_4,
	// Extended Latin from ISO-8859-14
	LatinExtendedA_14,
	// Extended Latin from ISO-8859-15
	LatinExtendedA_15,

	Unicode,

	// End
	NScripts,
	AnyScript = NScripts,
	UnknownScript = NScripts,

	NoScript
    };


#ifndef QT_NO_COMPAT

    static QFont defaultFont();
    static void setDefaultFont( const QFont & );

#endif // QT_NO_COMPAT


protected:
    // why protected?
    bool dirty() const;

    QString defaultFamily() const;
    QString lastResortFamily() const;
    QString lastResortFont() const;
    int deciPointSize() const;


private:
    QFont( QFontPrivate *, bool deep = TRUE );

    void detach();


#if defined(Q_WS_MAC)
    void macSetFont(QPaintDevice *);
#endif


#if defined(Q_WS_WIN)
    void *textMetric() const;
    HFONT create( bool *stockFont, HDC hdc = 0, bool VxF = FALSE );
#endif

    friend class QFontMetrics;
    friend class QFontInfo;
    friend class QPainter;
    friend class QPSPrinter;

#ifndef QT_NO_DATASTREAM
    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QFont & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QFont & );
#endif

    QFontPrivate *d;
};


inline bool QFont::bold() const
{ return weight() > Normal; }


inline void QFont::setBold( bool enable )
{ setWeight( enable ? Bold : Normal ); }




/*****************************************************************************
  QFont stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QFont & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QFont & );
#endif


#endif // QFONT_H
