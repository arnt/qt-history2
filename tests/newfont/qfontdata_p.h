/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdata.h#41 $
**
** Definition of internal QFontData struct
**
** Created : 941229
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

#ifndef QFONTDATA_P_H
#define QFONTDATA_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//


// font description
struct QFontDef {
    QFontDef()
	: pointSize(0), lbearing(SHRT_MIN), rbearing(SHRT_MIN),
	  styleHint(QFont::AnyStyle), styleStrategy(QFont::PreferDefault),
	  weight(0), italic(FALSE), underline(FALSE), strikeOut(FALSE),
	  fixedPitch(FALSE), hintSetByUser(FALSE), rawMode(FALSE), dirty(TRUE)
    { ; }


    QString family;

    short pointSize;
    short lbearing;
    short rbearing;

    uchar styleHint;
    uchar styleStrategy;
    uchar weight;

    bool italic;
    bool underline;
    bool strikeOut;
    bool fixedPitch;
    bool hintSetByUser;
    bool rawMode;

    bool dirty;
};


class QTextCodec;


#ifdef Q_WS_X11

// this is a shared wrapper for XFontStruct (to prevent a font being freed by
// the cache while it's being used)
class QFontStruct : public QShared
{
public:
    QFontStruct(Qt::HANDLE h, QCString n, QTextCodec *c, int a) :
	handle(h), name(n), codec(c), cache_cost(a)
    { ; }

    ~QFontStruct();

    Qt::HANDLE handle;
    QCString name;
    QTextCodec *codec;
    int cache_cost;
};

#endif // Q_WS_X11


// QFontPrivate - holds all data on which a font operates
class QFontPrivate : public QShared
{
public:
    enum Script {
	// Basic Latin with Latin-1 Supplement
	BASICLATIN,

	// To get Latin Extended-A characters from various ISO-8859-* encodings
	EXTLATINA2, // Extended Latin from ISO-8859-2
	EXTLATINA3, // Extended Latin from ISO-8859-3
	EXTLATINA4, // Extended Latin from ISO-8859-4
	EXTLATINA9, // Extended Latin from ISO-8859-9
	EXTLATINA14, // Extended Latin from ISO-8859-14
	EXTLATINA15, // Extended Latin from ISO-8859-15

	// TODO: support for Latin Extended-B characters

	CYRILLIC,
	ARABIC,
	GREEK,
	HEBREW,

	// South/Southeast Asian Scripts
	TAMIL,
	THAI,

	// East Asian Scripts
	HAN,
	HIRAGANA,
	KATAKANA,
	HANGUL,
	BOPOMOFO,

	UNICODE,

	// End
	NScripts,
	AnyScript = NScripts,
	UnknownScript = NScripts,
	
	// No Script
	NoScript
    };

    // stupid stupid egcs - It can't use NScripts below for the x11data.fontstruct
    // array size because it thinks QFontPrivate is still incomplete.  If you change
    // the above enum you *MUST* update this number to be equal to the new NScripts
    // value, lest you suffer firey death at the hand of qFatal().
#define NSCRIPTSEGCSHACK 19

    static Script scriptForChar(const QChar &c);


    QFontPrivate()
	: exactMatch(FALSE), lineWidth(1)
    {

#ifndef QT_NO_COMPAT
	// charset = QFont::AnyCharSet;
#endif

    }

    QFontPrivate(const QFontPrivate &fp)
	: QShared(fp), request(fp.request), actual(fp.actual),
	  exactMatch(fp.exactMatch), lineWidth(1)
    {
	
#ifndef QT_NO_COMPAT
	// charset = fp.charset;
#endif

    }

    // requested font
    QFontDef request;
    // actual font
    QFontDef actual;

    bool exactMatch;
    int lineWidth;

    QString defaultFamily() const;
    QString lastResortFamily() const;
    QString lastResortFont() const;
    QString key() const;


#if defined(Q_WS_X11)
    static char **getXFontNames(const char *, int *);
    static bool fontExists(const QString &);
    static bool parseXFontName(const QCString &, char **);
    static bool fillFontDef(const QCString &, QFontDef *, QCString *);
    static int getFontWeight(const QCString &, bool = FALSE);

    static inline bool isZero(char *x)
    {
	return (x[0] == '0' && x[1] == 0);

    }

    static inline bool isScalable( char **tokens )
    {
	return (isZero(tokens[PixelSize]) &&
		isZero(tokens[PointSize]) &&
		isZero(tokens[AverageWidth]));
    }

    static inline bool isSmoothlyScalable( char **tokens )
    {
	return (isZero(tokens[ResolutionX]) && isZero(tokens[ResolutionY]));
    }

    // XLFD fields
    enum FontFieldNames {
	Foundry,
	Family,
	Weight,
	Slant,
	Width,
	AddStyle,
	PixelSize,
	PointSize,
	ResolutionX,
	ResolutionY,
	Spacing,
	AverageWidth,
	CharsetRegistry,
	CharsetEncoding,
	NFontFields
    };

    QCString findFont(QFontPrivate::Script, bool *) const;
    QCString bestFamilyMember(QFontPrivate::Script, const QString &, const QString &,
			      int *) const;
    QCString bestMatch(const char *, int *) const;
    int fontMatchScore(const char *, QCString &, float *, int *, bool *, bool *) const;
    void computeLineWidth(QFontPrivate::Script);
    void initFontInfo(QFontPrivate::Script);
    void load(QFontPrivate::Script, bool = TRUE);

    class QFontX11Data {
    public:
	// X fontstruct handles for each character set
	QFontStruct *fontstruct[NSCRIPTSEGCSHACK];

	QFontX11Data()
	{
	    for (int i = 0; i < QFontPrivate::NScripts; i++) {
		fontstruct[i] = 0;
	    }
	}

	~QFontX11Data()
	{
	    QFontStruct *qfs;

	    for (int i = 0; i < QFontPrivate::NScripts; i++) {
		qfs = fontstruct[i];
		fontstruct[i] = 0;

		if (qfs && qfs != (QFontStruct *) -1) {
		    qfs->deref();
		}
	    }
	}
    } x11data;

    static QFontPrivate::Script defaultScript;

#endif // Q_WS_X11


#ifndef QT_NO_COMPAT
    // source compatibility for QFont
    // QFont::CharSet charsetcompat;
#endif // QT_NO_COMPAT

};


#endif // QFONTDATA_P_H
