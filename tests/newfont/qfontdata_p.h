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
    QString	family;

    short	pointSize;
    short	lbearing;
    short	rbearing;

    uint	styleHint	: 8;
    uint	styleStrategy	: 8;
    uint	weight		: 8;
    uint	italic		: 1;
    uint	underline	: 1;
    uint	strikeOut	: 1;
    uint	fixedPitch	: 1;
    uint	hintSetByUser	: 1;
    uint	rawMode		: 1;
    uint	dirty		: 1;
};


class QTextCodec;

#ifdef Q_WS_X11

// this is a shared wrapper for XFontStruct (to prevent a font being freed by
// the cache while it's being used)
class QFontStruct : public QShared
{
public:
    QFontStruct(Qt::HANDLE h, QCString n, QTextCodec *c) : handle(h), name(n), codec(c) { ; }
    ~QFontStruct();

    Qt::HANDLE handle;
    QCString name;
    QTextCodec *codec;
};

#endif



// QFontPrivate - holds all data on which a font operates
class QFontPrivate : public QShared
{
public:
    QFontPrivate()
	: // printerHackFont(0),
	exactMatch(FALSE), lineWidth(1)
    {
	request.pointSize = 0;
	request.lbearing = SHRT_MIN;
	request.rbearing = SHRT_MIN;
	request.styleHint = QFont::AnyStyle;
	request.styleStrategy = QFont::PreferDefault;
	request.weight = 0;
	request.italic = FALSE;
	request.underline = FALSE;
	request.strikeOut = FALSE;
	request.fixedPitch = FALSE;
	request.hintSetByUser = FALSE;
	request.rawMode = FALSE;
	request.dirty = TRUE;

	actual.pointSize = 0;
	actual.lbearing = SHRT_MIN;
	actual.rbearing = SHRT_MIN;
	actual.styleHint = QFont::AnyStyle;
	actual.styleStrategy = QFont::PreferDefault;
	actual.weight = 0;
	actual.italic = FALSE;
	actual.underline = FALSE;
	actual.strikeOut = FALSE;
	actual.fixedPitch = FALSE;
	actual.hintSetByUser = FALSE;
	actual.rawMode = FALSE;
	actual.dirty = TRUE;

#ifndef QT_NO_COMPAT
	charset = QFont::AnyCharSet;
#endif

    }

    QFontPrivate(const QFontPrivate &fp)
	: QShared(fp), request(fp.request), actual(fp.actual),
	  // printerHackFont(fp.printerHackFont),
	  exactMatch(fp.exactMatch),
	  lineWidth(1)

#ifdef Q_WS_X11
	, x11data(fp.x11data)
#endif

    {

#ifndef QT_NO_COMPAT
	charset = fp.charset;
#endif

    }

    // requested font
    QFontDef request;
    // actual font
    QFontDef actual;

    // QFont *printerHackFont;
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

    QCString findFont(QFont::CharSet, bool *) const;
    QCString bestFamilyMember(QFont::CharSet, const QString &, const QString &,
			      int *) const;
    QCString bestMatch(const char *, int *) const;
    int fontMatchScore(const char *, QCString &, float *, int *, bool *, bool *) const;
    void computeLineWidth(QFont::CharSet);
    void initFontInfo(QFont::CharSet);
    void load(QFont::CharSet, bool = TRUE);


    struct QFontX11Data {
	QFontX11Data()
	{
	    for (int i = 0; i < QFont::NCharSets; i++) {
		fontstruct[i] = 0;
		// codec[i] = 0;
	    }
	}

	QFontX11Data(const QFontX11Data &xd)
	{
	    for (int i = 0; i < QFont::NCharSets - 1; i++) {
		if (xd.fontstruct[i] &&
		    xd.fontstruct[i] != (QFontStruct *) -1 &&
		    xd.fontstruct[i] != xd.fontstruct[QFont::ISO_10646_1]) {
		    xd.fontstruct[i]->ref();
		}

		fontstruct[i] = xd.fontstruct[i];
		// codec[i] = xd.codec[i];
	    }

	    if (xd.fontstruct[QFont::ISO_10646_1] &&
		xd.fontstruct[QFont::ISO_10646_1] != (QFontStruct *) -1) {
		xd.fontstruct[QFont::ISO_10646_1]->ref();
	    }

	    fontstruct[QFont::ISO_10646_1] = xd.fontstruct[QFont::ISO_10646_1];
	    // codec[QFont::ISO_10646_1] = xd.codec[QFont::ISO_10646_1];
	}

	~QFontX11Data()
	{
	    for (int i = 0; i < QFont::NCharSets - 1; i++) {
		if (fontstruct[i] &&
		    fontstruct[i] != (QFontStruct *) -1 &&
		    fontstruct[i] != fontstruct[QFont::ISO_10646_1]) {
		    fontstruct[i]->deref();
		}
	    }

	    if (fontstruct[QFont::ISO_10646_1] &&
		fontstruct[QFont::ISO_10646_1] != (QFontStruct *) -1) {
		fontstruct[QFont::ISO_10646_1]->deref();
	    }
	}

	// X fontstruct handles for each character set
	QFontStruct *fontstruct[QFont::NCharSets];
	// Unicode -> font encoding
	// QTextCodec *codec[QFont::NCharSets];
    } x11data;


    static QFont::CharSet defaultCharSet;
#endif


    // source compatibility for QFont
#ifndef QT_NO_COMPAT
    QFont::CharSet charset;
#endif
};


// class QFontPrivate;
// class QTextCodec;
/*
  struct QFontData : public QShared {
  QFontData()
  : / * priv(0), * / printerHackFont(0), exactMatch(FALSE)
  {}

  QFontData( const QFontData &d )
  : QShared(d), req(d.req), / * priv(d.priv) * / printerHackFont(0),
  exactMatch(FALSE)
  // Copy the QShared count as well. The count may need to be
  // reset when using the QFontData class, see QFont::QFont(QFontData*)
  {}

  // needed?
  ~QFontData()
  {}

  QFontData &operator=( const QFontData &d )
  {
  req = d.req;
  exactMatch = d.exactMatch;
  // priv = d.priv;
  printerHackFont=d.printerHackFont;

  return *this;
  }

  // requested font
  QFontDef req;
  // private font data
  // QFontPrivate *priv;
  // printer fontmetrics hack font
  QFont *printerHackFont;

  bool exactMatch;

  const QTextCodec *mapper()  const;
  };
*/


#endif // QFONTDATA_P_H
