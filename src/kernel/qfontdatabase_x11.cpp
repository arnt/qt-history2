/****************************************************************************
** $Id$
**
** Implementation of platform specific QFontDatabase
**
** Created : 970521
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#include "qt_x11.h"

// #define QFONTDATABASE_DEBUG

static QString getStyleName( char ** tokens, bool *italic, bool *lesserItalic )
{
    char slant0 = tolower( (uchar) tokens[QFontPrivate::Slant][0] );
    *italic      = FALSE;
    *lesserItalic = FALSE;

    QString nm;
    if ( qstrcmp( tokens[QFontPrivate::Weight], "medium" ) != 0 )
	nm = QString::fromLatin1(tokens[QFontPrivate::Weight]);

    if ( nm.length() > 0 )
        nm.replace( 0, 1, nm[0].upper() );

    if ( slant0 == 'r' ) {
        if ( tokens[QFontPrivate::Slant][1]) {
            char slant1 = tolower( (uchar) tokens[QFontPrivate::Slant][1] );

            if ( slant1 == 'o' ) {
                nm += " Reverse Oblique";
                *italic       = TRUE;
                *lesserItalic = TRUE;
            } else if ( slant0 == 'i' ) {
                nm += " Reverse Italic";
                *italic       = TRUE;
                *lesserItalic = TRUE;
            }
        } else {
            // Normal
        }
    } else if ( slant0 == 'o' ) {
        if ( tokens[QFontPrivate::Slant][1] ) {
            nm += " Other";
        } else {
            nm += " Oblique";
            *italic = TRUE;
        }
    } else if ( slant0 == 'i' ) {
        nm += " Italic";
        *italic = TRUE;
    }

    if ( nm.isEmpty() ) {
        nm = "Normal";
    } else if ( nm[0] == ' ' ) {
        nm = nm.remove( 0, 1 );
    }

    return nm;
}


#ifndef QT_NO_XFTFREETYPE
static const char *getXftWeightString(int xftweight)
{
    int qtweight = QFont::Black;
    if (xftweight <= (XFT_WEIGHT_LIGHT + XFT_WEIGHT_MEDIUM) / 2)
	qtweight = QFont::Light;
    else if (xftweight <= (XFT_WEIGHT_MEDIUM + XFT_WEIGHT_DEMIBOLD) / 2)
	qtweight = QFont::Normal;
    else if (xftweight <= (XFT_WEIGHT_DEMIBOLD + XFT_WEIGHT_BOLD) / 2)
	qtweight = QFont::DemiBold;
    else if (xftweight <= (XFT_WEIGHT_BOLD + XFT_WEIGHT_BLACK) / 2)
	qtweight = QFont::Bold;

    if (qtweight <= (QFont::Light + QFont::Normal) / 2)
	return "Light";
    if (qtweight <= (QFont::Normal + QFont::DemiBold) / 2)
	return "Normal";
    if (qtweight <= (QFont::DemiBold + QFont::Bold) / 2)
	return "DemiBold";
    if (qtweight <= (QFont::Bold + QFont::Black) / 2)
	return "Bold";
    return "Black";
}
#endif // QT_NO_XFTFREETYPE


extern bool qt_has_xft; // defined in qfont_x11.cpp


void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;

    int fontCount;
    // force the X server to give us XLFDs
    char **fontList = XListFonts( QPaintDevice::x11AppDisplay(),
				  "-*-*-*-*-*-*-*-*-*-*-*-*-*-*",
				  32767, &fontCount );

    if ( fontCount >= 32767 ) {
	XFreeFontNames( fontList );
	fontList = XListFonts( qt_xdisplay(), "-*-*-*-*-*-*-*-*-*-*-*-*-*-*",
			       fontCount + 1, &fontCount );
    }

    char *tokens[QFontPrivate::NFontFields];

    QtFontFoundry *current_foundry = 0;
    QtFontFamily *current_family = 0;
    QtFontStyle *current_style = 0;
    char *current_foundryname = 0;
    char *current_familyname = 0;
    char *current_slantstr = 0, *current_weightstr = 0;

    for( int i = 0 ; i < fontCount ; i++ ) {
	if ( QFontPrivate::parseXFontName( fontList[i], tokens ) ) {
	    if ( ! current_foundry || ! current_foundryname ||
		 qstrcmp( current_foundryname,
			  tokens[QFontPrivate::Foundry] ) != 0 ) {
		// get foundry and insert it into the database if not present
		QString foundryName = tokens[QFontPrivate::Foundry];
		if ( foundryName.isEmpty() )
		    foundryName = "x11";
		current_foundry = db->foundryDict.find( foundryName );
		if ( ! current_foundry ) {
		    current_foundry = new QtFontFoundry( foundryName );
		    db->addFoundry( current_foundry );
		}

		// since we changed foundry, we need to force a family change
		current_family = 0;
	    }
	    current_foundryname = tokens[QFontPrivate::Foundry];

	    if ( ! current_family || ! current_familyname ||
		 qstrcmp( current_familyname,
			  tokens[QFontPrivate::Family] ) != 0 ) {
		// get family and insert it into the database if not present
		QString familyName = tokens[QFontPrivate::Family];
		current_family = current_foundry->familyDict.find( familyName );
		if ( ! current_family ) {
		    current_family = new QtFontFamily( current_foundry, familyName );
		    current_foundry->addFamily( current_family );
		}

		// since we changed family, we need for force a style change
		current_style = 0;
	    }
	    current_familyname = tokens[QFontPrivate::Family];

	    if ( ! current_style || ! current_slantstr || ! current_weightstr ||
		 qstrcmp( current_slantstr, tokens[QFontPrivate::Slant] ) != 0 ||
		 qstrcmp( current_weightstr, tokens[QFontPrivate::Weight] ) != 0 ) {
		// get style
		bool italic;
		bool lesserItalic;
		QString styleName = getStyleName( tokens, &italic, &lesserItalic );
		current_style = current_family->styleDict.find( styleName );
		if ( ! current_style ) {
		    current_style = new QtFontStyle( current_family, styleName );
		    current_style->ital         = italic;
		    current_style->lesserItal   = lesserItalic;
		    current_style->weightString = tokens[QFontPrivate::Weight];

		    current_family->addStyle( current_style );
		}
	    }
	    current_slantstr = tokens[QFontPrivate::Slant];
	    current_weightstr = tokens[QFontPrivate::Weight];

	    if ( QFontPrivate::isScalable(tokens) ) {
		if ( QFontPrivate::isSmoothlyScalable( tokens ) )
		    current_style->setSmoothlyScalable();
		else
		    current_style->setBitmapScalable();
	    } else {
		QCString ps = tokens[QFontPrivate::PointSize];
		int pSize = ps.toInt()/10;
		int r = atoi(tokens[QFontPrivate::ResolutionY]);
		if ( r && QPaintDevice::x11AppDpiY() &&
		     r != QPaintDevice::x11AppDpiY() ) {
		    // not "0" or "*", or required DPI
		    // calculate actual pointsize for display DPI
		    pSize = ( 2*pSize*r + QPaintDevice::x11AppDpiY() ) /
			    (QPaintDevice::x11AppDpiY() * 2);
		}

		if ( pSize != 0 )
		    current_style->addPointSize( pSize );
	    }

	    if (QFontPrivate::isFixedPitch(tokens))
		current_style->setFixedPitch();
	}
    }

    XFreeFontNames( fontList );

#ifndef QT_NO_XFTFREETYPE

    if (qt_has_xft) {
	XftFontSet  *foundries;
	XftFontSet  *families;
	XftFontSet  *styles;

	QCString foundryName, familyName;
	bool hasFoundries;
    	char *value;
	int weight_value;
	int slant_value;
	int spacing_value;
	bool created_foundry = FALSE;
	bool created_family = FALSE;

	foundries = XftListFonts (QPaintDevice::x11AppDisplay(),
				  QPaintDevice::x11AppScreen(),
				  (const char *)0,
				  XFT_FOUNDRY,
				  (const char *)0);

	for (int d = 0; d < foundries->nfont; d++) {
	    created_foundry = FALSE;

	    if (XftPatternGetString(foundries->fonts[d],
				    XFT_FOUNDRY, 0, &value) == XftResultMatch) {
		foundryName = value;
		hasFoundries = TRUE;
	    } else {
		hasFoundries = FALSE;
		foundryName = "xft";
	    }

	    QtFontFoundry *foundry = db->foundryDict.find( foundryName.lower() );
	    if ( ! foundry ) {
		foundry = new QtFontFoundry(foundryName.lower());
		Q_CHECK_PTR(foundry);
		created_foundry = TRUE;
	    }

	    if (hasFoundries)
		families =
		    XftListFonts(QPaintDevice::x11AppDisplay(),
				 QPaintDevice::x11AppScreen(),
				 XFT_FOUNDRY, XftTypeString, foundryName.data(), (const char *)0,
				 XFT_FAMILY, (const char *)0);
	    else
		families =
		    XftListFonts(QPaintDevice::x11AppDisplay(),
				 QPaintDevice::x11AppScreen(),
				 (const char *)0,
				 XFT_FAMILY, (const char *)0);

	    for (int f = 0; f < families->nfont; f++) {
		created_family = FALSE;

		if (XftPatternGetString(families->fonts[f],
					XFT_FAMILY, 0, &value) == XftResultMatch) {
		    familyName = value;

		    QtFontFamily *family =
			foundry->familyDict.find( familyName.lower() );
		    if ( ! family ) {
			family = new QtFontFamily ( foundry, familyName.lower() );
			Q_CHECK_PTR (family);
			created_family = TRUE;
		    }

		    if (hasFoundries)
			styles =
			    XftListFonts (QPaintDevice::x11AppDisplay(),
					  QPaintDevice::x11AppScreen(),
					  XFT_FOUNDRY, XftTypeString, foundryName.data(),
					  XFT_FAMILY, XftTypeString, familyName.data(),
					  (const char *)0,
					  XFT_STYLE, XFT_WEIGHT, XFT_SLANT, XFT_SPACING,
					  (const char *)0);
		    else
			styles =
			    XftListFonts (QPaintDevice::x11AppDisplay(),
					  QPaintDevice::x11AppScreen(),
					  XFT_FAMILY, XftTypeString, familyName.data(),
					  (const char *)0,
					  XFT_STYLE, XFT_WEIGHT, XFT_SLANT, XFT_SPACING,
					  (const char *)0);

		    for (int s = 0; s < styles->nfont; s++) {
			if (XftPatternGetString (styles->fonts[s],
						 XFT_STYLE, 0, &value) ==
			    XftResultMatch) {
			    QString styleName = QString::fromLatin1( value );
			    if (styleName.lower() == "regular")
				styleName = "Normal";
			    QtFontStyle *style = new QtFontStyle (family, styleName);
			    Q_CHECK_PTR (style);

			    slant_value = XFT_SLANT_ROMAN;
			    weight_value = XFT_WEIGHT_MEDIUM;
			    spacing_value = XFT_PROPORTIONAL;
			    XftPatternGetInteger (styles->fonts[s],
						  XFT_SLANT, 0, &slant_value);
			    XftPatternGetInteger (styles->fonts[s],
						  XFT_WEIGHT, 0, &weight_value);
			    XftPatternGetInteger (styles->fonts[s],
						  XFT_SPACING, 0, &spacing_value);
			    style->ital = slant_value != XFT_SLANT_ROMAN;
			    style->lesserItal = FALSE;
			    style->weightString = getXftWeightString(weight_value);
			    if (spacing_value >= XFT_MONO)
				style->setFixedPitch();
			    style->setSmoothlyScalable();
			    family->addStyle (style);
			}
		    }

		    if ( created_family ) {
			if ( created_foundry ) {
			    if ( ! db->family( familyName ) )
				foundry->addFamily( family );
			    else
				delete family;
			} else {
			    foundry->addFamily( family );
			}
		    }

		    XftFontSetDestroy (styles);
		}
	    }


	    if ( created_foundry ) {
		if ( foundry->familyDict.count() )
		    db->addFoundry( foundry );
		else
		    delete foundry;
	    }

	    XftFontSetDestroy(families);
	}

	XftFontSetDestroy (foundries);
    }
#endif

#ifdef QFONTDATABASE_DEBUG
    // print the database
    QDictIterator<QtFontFoundry> fndit(db->foundryDict);
    QtFontFoundry *fnd;
    while ((fnd = fndit.current()) != 0) {
	++fndit;

	qDebug("foundry %s", fnd->name().latin1());

	QDictIterator<QtFontFamily> famit(fnd->familyDict);
	QtFontFamily *fam;
	while ((fam = famit.current()) != 0) {
	    ++famit;

	    qDebug("    family %s", fam->name().latin1());

	    QDictIterator<QtFontStyle> styit(fam->styleDict);
	    QtFontStyle *sty;
	    while ((sty = styit.current()) != 0) {
		++styit;

		qDebug("        style %s", sty->name().latin1());
	    }
	}
    }
#endif // QFONTDATABASE_DEBUG

}
