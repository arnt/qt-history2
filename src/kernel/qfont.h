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


class QStringList;
class QFontPrivate;


class Q_EXPORT QFont					// font class
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

    // we will need some central way to convert a charset to an encoding name, but
    // probably not in QFont
    // static QString encodingName( CharSet );

    static QFont defaultFont();
    static void setDefaultFont( const QFont & );

    static QString substitute(const QString &);
    static QStringList substitutes(const QString &);
    static QStringList substitutions();
    static void insertSubstitution(const QString&, const QString &);
    static void removeSubstitution(const QString &);

    static void initialize();
    static void cleanup();
    static void cacheStatistics();


#if defined(Q_WS_QWS)
    void qwsRenderToDisk(bool all=TRUE);
#endif


#ifndef QT_NO_COMPAT

    enum CharSet {
	ISO_8859_1,  Latin1 = ISO_8859_1, AnyCharSet,
	ISO_8859_2,  Latin2 = ISO_8859_2,
	ISO_8859_3,  Latin3 = ISO_8859_3,
	ISO_8859_4,  Latin4 = ISO_8859_4,
	ISO_8859_5,
	ISO_8859_6,
	ISO_8859_7,
	ISO_8859_8,
	ISO_8859_9,  Latin5 = ISO_8859_9,
	ISO_8859_10, Latin6 = ISO_8859_10,
	ISO_8859_11, TIS620 = ISO_8859_11,
	ISO_8859_12,
	ISO_8859_13, Latin7 = ISO_8859_13,
	ISO_8859_14, Latin8 = ISO_8859_14,
	ISO_8859_15, Latin9 = ISO_8859_15,
	KOI8R,
	Set_Ja, Set_1 = Set_Ja,
	Set_Ko,
	Set_Th_TH,
	Set_Zh,
	Set_Zh_TW,
	Set_GBK,
	Set_Big5,
	Set_N = Set_Zh_TW,

	/*
	  The following are font-specific encodings that
	  we shouldn't need in a perfect world.
	*/
	// 8-bit fonts
	JIS_X_0201 = 0xa0,
	// 16-bit fonts
	JIS_X_0208 = 0xc0, Enc16 = JIS_X_0208,
	KSC_5601,
	GB_2312,
	Big5,
	TSCII,
	KOI8U,

	Unicode
    };

    QFont(const QString &, int, int, bool, CharSet);

    CharSet charSet() const;
    void setCharSet(CharSet);
    static CharSet charSetForLocale();
    QString encodingName(CharSet);

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


#ifndef QT_NO_DATASTREAM
    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QFont & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QFont & );
#endif

    // private data
    QFontPrivate *d;
};


#ifndef QT_NO_COMPAT


#endif

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
