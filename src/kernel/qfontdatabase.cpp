/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdatabase.cpp#24 $
**
** Implementation of font database class.
**
** Created : 990603
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
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

#include "qfontdatabase.h"

#ifndef QT_NO_FONTDATABASE

#include "qfontdata_p.h"

#ifdef Q_WS_QWS
#include "qfontmanager_qws.h"
#endif // Q_WS_QWS
#if defined( Q_WS_MAC )
#include "qt_mac.h"
#endif

#include "qmap.h"
#include "qdict.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qapplication.h"
#include "../../src/kernel/qapplication_p.h"
#include "qpainter.h"
#include <stdlib.h>
#include <ctype.h>


// NOT REVISED
#if defined( Q_WS_MAC )

extern int qFontGetWeight( const QCString &weightString, bool adjustScore=FALSE );

#endif // Q_WS_MAC








#ifdef Q_WS_X11

#include "qt_x11.h"

static QString getStyleName( char ** tokens, bool *italic, bool *lesserItalic );

#endif // Q_WS_X11








#ifdef Q_WS_WIN

#include "qt_windows.h"

static void populate_database(const QString& fam);

static void newWinFont( void * p );
static void add_style( QtFontFamily *family, const QString& styleName,
		       bool italic, bool lesserItalic, int weight );

#endif // Q_WS_WIN
#if defined( Q_WS_MAC )
static void add_style( QtFontFamily *family, const QString& styleName,
		bool italic, bool lesserItalic, int weight );
#endif









class QtFontFamily;
class QtFontFoundry;


class QtFontStyle
{
public:
    QtFontStyle( QtFontFamily *prnt, const QString &n )
    {
	p                = prnt;
	nm               = n;
	bitmapScalable   = FALSE;
	smoothlyScalable = FALSE;
	fixedPitch       = FALSE;
	weightDirty      = TRUE;
	ital             = FALSE;
	lesserItal       = FALSE;
	weightVal        = 0;
	weightDirty      = TRUE;
	sizesDirty       = TRUE;
    }

    QFont font( const QString &family, int pointSize ) const;  // ### fttb
    QFont font( int pointSize ) const;

    const QString &name() const { return nm; }
    const QtFontFamily *parent() const { return p; }

    const QValueList<int> &pointSizes() const;
    const QValueList<int> &smoothSizes() const;
    static const QValueList<int> &standardSizes();

    int weight() const;
    bool italic() const { return ital || lesserItal; }
    bool lesserItalic() const { return lesserItal; }

    bool isBitmapScalable() const { return bitmapScalable; }
    bool isSmoothlyScalable() const { return smoothlyScalable; }
    bool isFixedPitch() const { return fixedPitch; }


private:
    const QValueList<int> &storedSizes() const;

    void addPointSize( int size );
    void setSmoothlyScalable();
    void setBitmapScalable();
    void setFixedPitch();

    QtFontFamily *p;
    QString nm;

    bool bitmapScalable;
    bool smoothlyScalable;
    bool fixedPitch;

    bool ital;
    bool lesserItal;
    QCString weightString;
    int  weightVal;
    bool weightDirty;
    bool sizesDirty;

    QMap<int, int> sizeMap;
    QValueList<int> sizeList;

    friend void QFontDatabase::createDatabase();

#ifdef Q_WS_WIN

    friend void newWinFont( void * p );
    friend void add_style( QtFontFamily *family, const QString& styleName,
			   bool italic, bool lesserItalic, int weight );

#endif // Q_WS_WIN

#if defined( Q_WS_MAC )
    friend void add_style( QtFontFamily *family, const QString& styleName,
		bool italic, bool lesserItalic, int weight );
#endif


};


class QtFontFamily
{
public:
    QtFontFamily( QtFontFoundry *prnt, const QString &n )
    {
	p                = prnt;
	nm               = n;
	dirty            = TRUE;
	namesDirty       = TRUE;
	bitmapScalable   = FALSE;
	smoothlyScalable = FALSE;
	fixedPitch       = FALSE;
	scalableDirty    = TRUE;
    }

    const QString &name() const { return nm; }

    const QtFontFoundry *parent() { return p; }

    const QStringList &styles() const;
    const QtFontStyle *style(const QString &) const;

    bool isBitmapScalable() const;
    bool isSmoothlyScalable() const;
    bool isFixedPitch() const;


private:
    void refresh() const;
    void addStyle(QtFontStyle *s) { styleDict.insert( s->name(), s ); }

    QString nm;
    QtFontFoundry *p;

    QtFontStyle *normalStyle; // Only makes sense if the font is scalable
    QtFontStyle *italicStyle; // Gives information about which
    QtFontStyle *boldStyle;   //  combinations of these are available.
    QtFontStyle *italicBoldStyle;

    QStringList styleNames;
    QDict<QtFontStyle> styleDict;

    bool dirty;
    bool namesDirty;
    bool scalableDirty;
    bool fixedPitch;

    bool bitmapScalable;
    bool smoothlyScalable;

    friend void QFontDatabase::createDatabase();

#ifdef Q_WS_WIN
    friend void newWinFont( void * p );
    friend void add_style( QtFontFamily *family, const QString& styleName,
			   bool italic, bool lesserItalic, int weight );
#endif
#if defined( Q_WS_MAC )
    friend void add_style( QtFontFamily *family, const QString& styleName,
		bool italic, bool lesserItalic, int weight );
#endif


};


class QtFontFoundry
{
public:
    QtFontFoundry( const QString &n ) { nm = n; namesDirty = TRUE; }

    QString name() const { return nm; }

    const QStringList &families() const;
    const QtFontFamily *family( const QString &name ) const;

private:
    QString nm;

    QStringList familyNames;
    QDict<QtFontFamily> familyDict;

    bool namesDirty;

    void addFamily( QtFontFamily *f )
	{ familyDict.insert( f->name(), f ); }

    friend void QFontDatabase::createDatabase();

#ifdef Q_WS_WIN
    friend void newWinFont( void * p );
#endif
};


class QFontDatabase::Private {
public:
    Private(){
	namesDirty          = TRUE;
	familiesDirty       = TRUE;
	foundryDict.setAutoDelete( TRUE );
    }

    const QStringList &families() const;
    const QtFontFamily *family( const QString &name ) const;

    const QStringList &foundries() const;
    const QtFontFoundry *foundry( const QString foundryName ) const;

private:
    QStringList foundryNames;
    QDict<QtFontFoundry> foundryDict;

    QStringList familyNames;
    QDict<QtFontFamily> bestFamilyDict;

    bool namesDirty;
    bool familiesDirty;

    void addFoundry( QtFontFoundry *f )
	{ foundryDict.insert( f->name(), f ); }

    friend void QFontDatabase::createDatabase();

#ifdef Q_WS_WIN
    friend void newWinFont( void * p );
#endif
};


QFont QtFontStyle::font( const QString & family, int pointSize ) const
{
    QFont f( family, pointSize, weight(), italic() );
    f.setFixedPitch(isFixedPitch());
    return f;
}


QFont QtFontStyle::font( int pointSize ) const
{
    QFont f(parent()->name(), pointSize);
    f.setFixedPitch(isFixedPitch());
    return f;
}


const QValueList<int> &QtFontStyle::pointSizes() const
{
    if ( smoothlyScalable || bitmapScalable )
	return standardSizes();

    return storedSizes();
}


const QValueList<int> &QtFontStyle::smoothSizes() const
{
    if ( smoothlyScalable )
	return standardSizes();

    return storedSizes();
}


int QtFontStyle::weight() const
{
    if ( weightDirty ) {
	QtFontStyle *that = (QtFontStyle*) this; // mutable function
	that->weightVal = QFontPrivate::getFontWeight( weightString, TRUE );
	that->weightDirty = FALSE;
    }

    return weightVal;
}


const QValueList<int> &QtFontStyle::storedSizes() const
{
    if ( sizesDirty ) {
	QtFontStyle *that = (QtFontStyle*) this;  // Mutable function
	QMap<int, int>::ConstIterator it = sizeMap.begin();

	for( ; it != sizeMap.end() ; ++it )
	    that->sizeList.append( *it );

	that->sizesDirty = FALSE;
    }

    return sizeList;
}


const QValueList<int> &QtFontStyle::standardSizes()
{
    static int s[]={ 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28,
		     36, 48, 72, 0 };
    static bool first = TRUE;
    static QValueList<int> sList;

    if ( first ) {
	first = FALSE;
	int i = 0;

	while( s[i] )
	    sList.append( s[i++] );
    }

    return sList;
}


void QtFontStyle::addPointSize( int pointSize )
{
    if ( smoothlyScalable )
	return;

    sizeMap.insert( pointSize, pointSize );
}


void QtFontStyle::setSmoothlyScalable()
{
    smoothlyScalable = TRUE;
    sizeMap.clear();
}


void QtFontStyle::setBitmapScalable()
{
    bitmapScalable = TRUE;
}


void QtFontStyle::setFixedPitch()
{
    fixedPitch = TRUE;
}


static int styleSortValue( QtFontStyle *style )
{
    int score = 100000; // so lexical ordering is ok
    score += style->weight() * 100;

    if ( style->italic() ) {
	score += 10;

	if ( style->lesserItalic() )
	    score += 20000;
    }

    return score;
}


const QStringList &QtFontFamily::styles() const
{
    if ( namesDirty ) {
	QtFontFamily *that = (QtFontFamily *) this; // Mutable function

#ifdef Q_WS_WIN
	// Lazy evaluation
	populate_database(that->parent()->name());
#endif // Q_WS_WIN

	QMap<QString, QString> styleMap;
	QDictIterator<QtFontStyle> iter( styleDict );
	QtFontStyle *tmp;

	for( ; (tmp = iter.current()) ; ++iter ) {
	    styleMap.insert( QString().setNum(styleSortValue( tmp ))+
			     tmp->name(), tmp->name() );
	}

	QMap<QString,QString>::Iterator it = styleMap.begin();
	for ( ; it != styleMap.end(); ++it ) {
	    that->styleNames.append( *it );
	}

	that->namesDirty = FALSE;
    }

    return styleNames;
}


const QtFontStyle *QtFontFamily::style(const QString &name) const
{
    return styleDict.find(name);
}


bool QtFontFamily::isFixedPitch() const
{
    refresh();
    return fixedPitch;
}


bool QtFontFamily::isBitmapScalable() const
{
    refresh();
    return bitmapScalable;
}


bool QtFontFamily::isSmoothlyScalable() const
{
    refresh();
    return smoothlyScalable;
}


void QtFontFamily::refresh() const
{
    if ( !scalableDirty )
	return;

    bool smooth = TRUE;
    bool bitmap = TRUE;
    bool fixed  = TRUE;
    QtFontStyle *tmp;
    QDictIterator<QtFontStyle> iter(styleDict);

    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( !tmp->isSmoothlyScalable() ) {
	    smooth = FALSE;
	    if ( !tmp->isBitmapScalable() )
		bitmap = FALSE;
	}
	if ( !tmp->isFixedPitch() )
	    fixed = FALSE;
    }

    QtFontFamily *that = (QtFontFamily*) this;   // Mutable function
    that->scalableDirty    = FALSE;
    that->smoothlyScalable = smooth;
    that->bitmapScalable   = bitmap;
    that->fixedPitch       = fixed;
}


const QStringList &QtFontFoundry::families() const
{
    if ( namesDirty ) {
	QtFontFoundry *that = (QtFontFoundry*) this;   // Mutable function
	QDictIterator<QtFontFamily> iter( familyDict );
	QtFontFamily *tmp;
	for( ; (tmp = iter.current()) ; ++iter )
	    that->familyNames.append( tmp->name() );
	that->familyNames.sort();
	that->namesDirty = FALSE;
    }
    return familyNames;
}


const QtFontFamily *QtFontFoundry::family( const QString &n ) const
{
    return familyDict.find( n );
}


const QStringList & QFontDatabase::Private::families() const
{
    Private *that = (Private*) this; // Mutable

    if ( familiesDirty ) {
	QDict<QtFontFoundry> firstFoundryForFamily;
	QDict<int> doubles;
	QtFontFoundry *foundry;
	QString s;
	QDictIterator<QtFontFoundry> iter( foundryDict );

	for( ; (foundry = iter.current()) ; ++iter ) {
	    QStringList l = foundry->families();

	    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
		if ( !firstFoundryForFamily.find( *it ) ) {
		    that->familyNames.append( *it );
		    firstFoundryForFamily.insert( *it, foundry );
		} else {
		    QString s;

		    if ( !doubles.find(*it) ) { // 2nd foundry for family?
			doubles.insert( *it, (int*)1 );
			QtFontFoundry *tmp = firstFoundryForFamily.find(*it);
			QString firstFoundryName;

			if ( tmp )
			    firstFoundryName = tmp->name();
			else
			    qWarning( "QFontDatabase::Private::families:"
				      "Internal error, Cannot find first foundry");

			that->familyNames.remove( *it );
			s = firstFoundryName + "-" + *it;
			that->familyNames.append( s );
		    }

		    s = foundry->name() + "-" + *it;
		    that->familyNames.append( s );
		}
	    }
	}

	that->familyNames.sort();
	that->familiesDirty = FALSE;
    }

    return familyNames;
}


const QtFontFamily * QFontDatabase::Private::family( const QString &name ) const
{
    if ( name.isEmpty() )
	return 0;

    Private *that = (Private*) this; // Mutable
    const QtFontFamily *result = bestFamilyDict.find(name);

    if ( !result ) {
	const QtFontFoundry *fndry;
	const QtFontFamily *fam;

	if ( name.contains('-') ) {
	    int i = name.find('-');
	    QString foundryName = name.left( i );
	    QString familyName = name.right( name.length() - i - 1 );
	    fndry = foundry( foundryName );

	    if ( fndry ) {
		fam = fndry->family( familyName );

		if ( fam ) {
		    that->bestFamilyDict.insert( name, fam );
		    return fam;
		}
	    }
	}

	QDictIterator<QtFontFoundry> iter( foundryDict );
	const QtFontFamily *nonScalable    = 0;
	const QtFontFamily *bitmapScalable = 0;

	for( ; (fndry = iter.current()) ; ++iter ) {
	    fam = fndry->family( name );

	    if ( fam ) {
		if ( fam->isSmoothlyScalable() ) {
		    result = fam;
		    break;
		}

		if ( fam->isBitmapScalable() )
		    bitmapScalable = fam;
		else
		    nonScalable    = fam;
	    }
	}

	if ( !result )
	    result = bitmapScalable ? bitmapScalable : nonScalable;

	if ( result )
	    that->bestFamilyDict.insert( name, result );
    }

    return result;
}


const QStringList & QFontDatabase::Private::foundries() const
{
    if ( namesDirty ) {
	Private *that = (Private*) this;  // Mutable
	QDictIterator<QtFontFoundry> iter( foundryDict );
	QtFontFoundry *tmp;

	for( ; (tmp = iter.current()) ; ++iter )
	    that->foundryNames.append( tmp->name() );

	that->foundryNames.sort();
	that->namesDirty = FALSE;
    }

    return foundryNames;

}


const QtFontFoundry * QFontDatabase::Private::foundry( const QString foundryName ) const
{
    return foundryDict.find( foundryName );
}


static QFontDatabase::Private *db=0;


#ifdef Q_WS_X11


#ifndef QT_NO_XFTFREETYPE
static const char *getXftWeightString(int xftweight)
{
    int qtweight = QFont::Black;
    if (xftweight <= (XFT_WEIGHT_LIGHT + XFT_WEIGHT_MEDIUM) / 2)
	qtweight = QFont::Light;
    if (xftweight <= (XFT_WEIGHT_MEDIUM + XFT_WEIGHT_DEMIBOLD) / 2)
	qtweight = QFont::Normal;
    if (xftweight <= (XFT_WEIGHT_DEMIBOLD + XFT_WEIGHT_BOLD) / 2)
	qtweight = QFont::DemiBold;
    if (xftweight <= (XFT_WEIGHT_BOLD + XFT_WEIGHT_BLACK) / 2)
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

    db = new Private;

#ifndef QT_NO_XFTFREETYPE

    // #define QFDB_DEBUG

    if (qt_has_xft) {
	XftFontSet  *foundries;
	XftFontSet  *families;
	XftFontSet  *styles;
	char	    *foundry_name, *qt_foundry_name;
	char	    *family_name;
	char	    *style_value;
	int	    weight_value;
	int	    slant_value;
	int	    foundry_i;
	int	    family_i;
	int	    style_i;
	static char default_foundry[] = "Unknown";

	foundries = XftListFonts (qt_xdisplay (),
				  qt_xscreen (),
				  0,
				  XFT_FOUNDRY,
				  0);

#ifdef QFDB_DEBUG
	printf ("Foundries ");
	XftFontSetPrint (foundries);
#endif
	for (foundry_i = 0; foundry_i < foundries->nfont; foundry_i++) {
	    if (XftPatternGetString(foundries->fonts[foundry_i],
				    XFT_FOUNDRY, 0, &foundry_name) == XftResultMatch) {
		qt_foundry_name = foundry_name;
	    } else {
		foundry_name = 0;
		qt_foundry_name = default_foundry;
	    }

	    QtFontFoundry *foundry = new QtFontFoundry(qt_foundry_name);
	    Q_CHECK_PTR(foundry);
	    db->addFoundry(foundry);

	    if (foundry_name)
		families = XftListFonts(qt_xdisplay (), qt_xscreen(),
					XFT_FOUNDRY, XftTypeString, foundry_name, 0,
					XFT_FAMILY, 0);
	    else
		families = XftListFonts(qt_xdisplay (),
					qt_xscreen(),
					0,
					XFT_FAMILY, 0);
#ifdef QFDB_DEBUG
	    printf ("Families ");
	    XftFontSetPrint (families);
#endif
	    for (family_i = 0; family_i < families->nfont; family_i++) {
		if (XftPatternGetString(families->fonts[family_i],
					XFT_FAMILY, 0, &family_name) == XftResultMatch) {
		    QtFontFamily *family = new QtFontFamily ( foundry, family_name);
		    Q_CHECK_PTR (family);
		    foundry->addFamily (family);

		    if (foundry_name) {
			styles = XftListFonts (qt_xdisplay (),
					       qt_xscreen(),
					       XFT_FOUNDRY, XftTypeString, foundry_name,
					       XFT_FAMILY, XftTypeString, family_name,
					       0,
					       XFT_STYLE,
					       XFT_WEIGHT,
					       XFT_SLANT,
					       0);
		    } else {
			styles = XftListFonts (qt_xdisplay (),
					       qt_xscreen(),
					       XFT_FAMILY, XftTypeString, family_name,
					       0,
					       XFT_STYLE,
					       XFT_WEIGHT,
					       XFT_SLANT,
					       0);
		    }

#ifdef QFDB_DEBUG
		    printf ("Styles ");
		    XftFontSetPrint (styles);
#endif
		    for (style_i = 0; style_i < styles->nfont; style_i++) {
			if (XftPatternGetString (styles->fonts[style_i],
						 XFT_STYLE, 0, &style_value) ==
			    XftResultMatch) {
			    QtFontStyle *style = new QtFontStyle (family, style_value);
			    Q_CHECK_PTR (style);

			    slant_value = XFT_SLANT_ROMAN;
			    weight_value = XFT_WEIGHT_MEDIUM;
			    XftPatternGetInteger (styles->fonts[style_i],
						  XFT_SLANT, 0, &slant_value);
			    XftPatternGetInteger (styles->fonts[style_i],
						  XFT_WEIGHT, 0, &weight_value);
			    style->ital = slant_value != XFT_SLANT_ROMAN;
			    style->lesserItal = FALSE;
			    style->weightString = getXftWeightString(weight_value);
			    style->setSmoothlyScalable();
			    family->addStyle (style);
			}
		    }
		    XftFontSetDestroy (styles);
		}
	    }
	    XftFontSetDestroy(families);
	    if (qt_foundry_name == default_foundry)
		break;
	}
	XftFontSetDestroy (foundries);
    }
#endif

    int fontCount;
    // force the X server to give us XLFDs
    char **fontList = XListFonts( qt_xdisplay(), "-*-*-*-*-*-*-*-*-*-*-*-*-*-*",
				  32767, &fontCount );

    if ( fontCount >= 32767 )
	qWarning( "More than 32k fonts, please notify qt-bugs@trolltech.com" );

    char *tokens[QFontPrivate::NFontFields];

    for( int i = 0 ; i < fontCount ; i++ ) {

	QCString fontName = fontList[i];

	if ( QFontPrivate::parseXFontName( fontName, tokens ) ) {
	    // get foundry and insert it into the database if not present
	    QString foundryName = tokens[QFontPrivate::Foundry];
	    QtFontFoundry *foundry = db->foundryDict.find( foundryName );
	    if ( !foundry ) {
		// qDebug( "New font foundry [%s]", foundryName.latin1() );

		foundry = new QtFontFoundry( foundryName );
		Q_CHECK_PTR(foundry);
		db->addFoundry( foundry );
	    }

	    // get family and insert it into the database if not present
	    QString familyName = tokens[QFontPrivate::Family];
	    QtFontFamily *family = foundry->familyDict.find( familyName );
	    if ( !family ) {
		// qDebug( "New font family [%s][%s]", familyName.latin1(),
		// foundryName.latin1());

		family = new QtFontFamily( foundry, familyName );
		Q_CHECK_PTR(family);
		foundry->addFamily( family );
	    }

	    // get style
	    bool italic;
	    bool lesserItalic;
	    QString styleName = getStyleName( tokens, &italic, &lesserItalic );
	    QtFontStyle *style = family->styleDict.find( styleName );
	    if ( !style ) {
		// qDebug( "New style[%s] for [%s][%s]", styleName.latin1(),
		// familyName.latin1(), foundryName.latin1() );

		style = new QtFontStyle( family, styleName );
		Q_CHECK_PTR( style );
		style->ital         = italic;
		style->lesserItal   = lesserItalic;
		style->weightString = tokens[QFontPrivate::Weight];

		family->addStyle(style);
	    }

	    if ( QFontPrivate::isScalable(tokens) ) {
		if ( QFontPrivate::isSmoothlyScalable( tokens ) ) {
		    style->setSmoothlyScalable();
		    // qDebug( "Smooth [%s][%s][%s]", styleName.latin1(),
		    // familyName.latin1(), tokens[QFontPrivate::Family] );
		} else {
		    style->setBitmapScalable();
		    // qDebug( "Scalable [%s][%s][%s]", styleName.latin1(),
		    // familyName.latin1(), tokens[QFontPrivate::Family] );
		}
	    } else {
		QCString ps = tokens[QFontPrivate::PointSize];
		int pSize = ps.toInt()/10;

		if ( pSize != 0 ) {
		    style->addPointSize( pSize );
		}
	    }

	    if (QFontPrivate::isFixedPitch(tokens))
		style->setFixedPitch();
	}
    }

    XFreeFontNames( fontList );
}

#endif // Q_WS_X11


#ifdef Q_WS_WIN

static
int CALLBACK
storeFont( ENUMLOGFONTEX* f, TEXTMETRIC*, int /*type*/, LPARAM /*p*/ )
{
    // Private* d = (Private*)p;
    newWinFont( (void*) f );
    return 1; // Keep enumerating.
}

static
void add_style( QtFontFamily *family, const QString& styleName,
		bool italic, bool lesserItalic, int weight )
{
    QString weightString;
    if ( weight <= QFont::Light ) {
	weight = QFont::Light;
	weightString = "Light";
    } else if ( weight <= QFont::Normal ) {
	weight = QFont::Normal;
	weightString = "Regular";
    } else if ( weight <= QFont::DemiBold ) {
	weight = QFont::DemiBold;
	weightString = "DemiBold";
    } else if ( weight <= QFont::Bold ) {
	weight = QFont::Bold;
	weightString = "Bold";
    } else {
	weight = QFont::Black;
	weightString = "Black";
    }

    QString sn = styleName;
    if ( sn.isEmpty() ) {
	// Not TTF, we make the name
	if ( weight != QFont::Normal || !italic && !lesserItalic ) {
	    sn += weightString;
	    sn += " ";
	}
	if ( italic )
	    sn += "Italic ";
	if ( lesserItalic ) {
	    // Windows doesn't tell the user, so we don't either
	    //  sn += "Oblique ";
	    sn += "Italic ";
	}
	sn = sn.left(sn.length()-1); // chomp " "
    }
    QtFontStyle *style = family->styleDict.find( sn );
    if ( !style ) {
	// qWarning( "New style[%s] for [%s][%s][%s]",
	// (const char*)styleName, (const char*)charSetName,
	// (const char*)familyName, (const char *)foundryName );
	style = new QtFontStyle( family, sn );
	Q_CHECK_PTR( style );
	style->ital         = italic;
	style->lesserItal   = lesserItalic;
	style->weightString = weightString;
	style->weightVal    = weight;
	style->weightDirty  = FALSE;
	family->addStyle( style );
    }

    //#### eiriken?
#if 0
else
qDebug("Already got it");
#endif

    style->setSmoothlyScalable();  // cowabunga
}


static
void newWinFont( void * p )
{
    ENUMLOGFONTEX* f = (ENUMLOGFONTEX*)p;

    static QtFontFoundry *foundry = 0;

    if ( !foundry ) {
	foundry = new QtFontFoundry( "MS" ); // One foundry on Windows
	// (and only one db)
	db->addFoundry(foundry);
    }

    const TCHAR* tc = f->elfLogFont.lfFaceName;

    QString familyName;
    if ( qt_winver & Qt::WV_NT_based ) {
	familyName = qt_winQString((void*)tc);
    } else {
	familyName = QString::fromLocal8Bit((const char*)tc);
    }

    QtFontFamily *family = foundry->familyDict.find( familyName );
    if ( !family ) {
	//qWarning( "New font family [%s][%s]",
	// (const char*) familyName, (const char*) foundryName );
	family = new QtFontFamily( foundry, familyName );
	Q_CHECK_PTR(family);
	foundry->addFamily( family );
    }
    bool italic = f->elfLogFont.lfItalic;
    int weight = f->elfLogFont.lfWeight/10;

    tc = (TCHAR*)f->elfStyle;

    QString styleName;
    if ( qt_winver & Qt::WV_NT_based ) {
	styleName = qt_winQString((void*)tc);
    } else {
	styleName = QString::fromLocal8Bit((const char*)tc);
    }

    if ( styleName.isEmpty() ) {
	// Not TTF, we enumerate the
	// transformed fonts that Windows can generate.

#if 0
qDebug("%s with quality %x",familyName.latin1(),f->elfLogFont.lfQuality);
#endif

	add_style( family, styleName, FALSE, FALSE, weight );
	add_style( family, styleName, FALSE, TRUE, weight );

	if ( weight < QFont::DemiBold ) {
	    // Can make bolder
	    add_style( family, styleName, FALSE, FALSE, QFont::Bold );
	    add_style( family, styleName, FALSE, TRUE, QFont::Bold );
	}
    } else {
	if ( italic ) {
	    add_style( family, styleName, italic, FALSE, weight );
	} else {
	    add_style( family, styleName, italic, FALSE, weight );
	    add_style( family, QString::null, italic, TRUE, weight );
	}
	if ( weight < QFont::DemiBold ) {
	    // Can make bolder
	    if ( italic )
		add_style( family, QString::null, italic, FALSE, QFont::Bold );
	    else {
		add_style( family, QString::null, FALSE, FALSE, QFont::Bold );
		add_style( family, QString::null, FALSE, TRUE, QFont::Bold );
	    }
	}
    }
}

static
void populate_database(const QString& fam)
{
    QWidget dummy;
    QPainter p( &dummy );

    if ( qt_winver & Qt::WV_NT_based ) {
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	if ( fam.isNull() ) {
	    lf.lfFaceName[0] = 0;
	} else {
	    memcpy(lf.lfFaceName,qt_winTchar( fam, TRUE ),
		sizeof(TCHAR)*QMIN(fam.length()+1,32));  // 32 = Windows hard-coded
	}
	lf.lfPitchAndFamily = 0;

#if 0
	EnumFontFamiliesEx( dummy.handle(), &lf,
	    (FONTENUMPROC)storeFont, (LPARAM)db, 0 );
#else
	EnumFontFamilies( dummy.handle(), (LPTSTR) 0,
	    (FONTENUMPROC)storeFont, (LPARAM)db );
#endif

    } else {
	LOGFONTA lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	if ( fam.isNull() ) {
	    lf.lfFaceName[0] = 0;
	} else {
	    QCString lname = fam.local8Bit();
	    memcpy(lf.lfFaceName,lname.data(),
		QMIN(lname.length()+1,32));  // 32 = Windows hard-coded
	}
	lf.lfPitchAndFamily = 0;
	EnumFontFamiliesExA( dummy.handle(), &lf,
	    (FONTENUMPROCA)storeFont, (LPARAM)db, 0 );
    }

    // ##### Should add Italic if none already
    // ##### Should add Bold and Bold Italic if any less-than-bold exists
}

void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new Private;
    populate_database(0);
}

#endif // Q_WS_WIN


#if defined( Q_WS_MAC )

static
void add_style( QtFontFamily *family, const QString& styleName,
		bool italic, bool lesserItalic, int weight )
{
    QString weightString;
    if ( weight <= QFont::Light ) {
	weight = QFont::Light;
	weightString = "Light";
    } else if ( weight <= QFont::Normal ) {
	weight = QFont::Normal;
	weightString = "Regular";
    } else if ( weight <= QFont::DemiBold ) {
	weight = QFont::DemiBold;
	weightString = "DemiBold";
    } else if ( weight <= QFont::Bold ) {
	weight = QFont::Bold;
	weightString = "Bold";
    } else {
	weight = QFont::Black;
	weightString = "Black";
    }

    QString sn = styleName;
    if ( sn.isEmpty() ) {
	// Not TTF, we make the name
	if ( weight != QFont::Normal || !italic && !lesserItalic ) {
	    sn += weightString;
	    sn += " ";
	}
	if ( italic )
	    sn += "Italic ";
	if ( lesserItalic ) {
	    // Windows doesn't tell the user, so we don't either
	    //  sn += "Oblique ";
	    sn += "Italic ";
	}
	sn = sn.left(sn.length()-1); // chomp " "
    }
    QtFontStyle *style = family->styleDict.find( sn );
    if ( !style ) {
	style = new QtFontStyle( family, sn );
	Q_CHECK_PTR( style );
	style->ital         = italic;
	style->lesserItal   = lesserItalic;
	style->weightString = weightString;
	style->weightVal    = weight;
	style->weightDirty  = FALSE;
	family->addStyle( style );
    }
    style->setSmoothlyScalable();  // cowabunga
}

void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new Private;

    QtFontFoundry *foundry = NULL;
    if ( !foundry ) {
	foundry = new QtFontFoundry( "Mac" ); // One foundry on Macintosh
	db->addFoundry(foundry);        // (and only one db)
    }

    FMFontFamilyIterator it;
    if(!FMCreateFontFamilyIterator(NULL, NULL, kFMUseGlobalScopeOption, &it)) {
	FMFontFamily fam;
	QString fam_name;
	while(!FMGetNextFontFamily(&it, &fam)) {

	    static Str255 n;
	    if(FMGetFontFamilyName(fam, n))
		qDebug("Whoa! %s %d", __FILE__, __LINE__);

	    TextEncoding encoding;
	    FMGetFontFamilyTextEncoding( fam, &encoding);
	    TextToUnicodeInfo uni_info;
	    CreateTextToUnicodeInfoByEncoding( encoding, &uni_info);

	    unsigned long len = n[0] * 2;
	    unsigned char *buff = (unsigned char *)malloc(len);
	    ConvertFromPStringToUnicode(uni_info, n, len, &len, (UniCharArrayPtr)buff);
	    fam_name = "";
	    for(unsigned long x = 0; x < len; x+=2)
		fam_name += QChar(buff[x+1], buff[x]);

	    QtFontFamily *family = foundry->familyDict.find( fam_name );
	    if ( !family ) {
		family = new QtFontFamily( foundry, fam_name );
		Q_CHECK_PTR(family);
		foundry->addFamily( family );
	    }

	    FMFontFamilyInstanceIterator fit;
	    if(!FMCreateFontFamilyInstanceIterator(fam, &fit)) {
		FMFont font;
		FMFontStyle font_style;
		FMFontSize font_size;

		while(!FMGetNextFontFamilyInstance(&fit, &font, &font_style, &font_size)) {
		    bool italic = (bool)(font_style & ::italic);
		    int weight = ((font_style & ::bold) ? QFont::Bold : QFont::Normal);

		    if ( italic ) {
			add_style( family, QString::null, italic, FALSE, weight );
		    } else {
			add_style( family, QString::null, italic, FALSE, weight );
			add_style( family, QString::null, italic, TRUE, weight );
		    }
		    if ( weight < QFont::DemiBold ) {
			// Can make bolder
			if ( italic )
			    add_style( family, QString::null, italic, FALSE, QFont::Bold );
			else {
			    add_style( family, QString::null, FALSE, FALSE, QFont::Bold );
			    add_style( family, QString::null, FALSE, TRUE, QFont::Bold );
			}
		    }
		}
		FMDisposeFontFamilyInstanceIterator(&fit);
	    }
	}
	FMDisposeFontFamilyIterator(&it);
    }
}

#endif // Q_WS_MAC


/*!
  Returns a string that describes the style of the font \a f. This is something like
  "Bold Italic".
*/
QString QFontDatabase::styleString( const QFont &f )
{
    QString result;
    if ( f.weight() >= QFont::Bold ) {
	if ( f.italic() )
	    result = "Bold Italic";
	else
	    result = "Bold";
    } else {
	if ( f.italic() )
	    result = "Italic";
	else
	    result = "Normal";
    }
    return result;
}


#ifdef Q_WS_X11

static QString getStyleName( char ** tokens, bool *italic, bool *lesserItalic )
{
    char slant0 = tolower( tokens[QFontPrivate::Slant][0] );
    *italic      = FALSE;
    *lesserItalic = FALSE;

    QString nm = QString::fromLatin1(tokens[QFontPrivate::Weight]);

    if ( nm == QString::fromLatin1("medium") )
	nm = QString::fromLatin1("");

    if ( nm.length() > 0 )
	nm.replace( 0, 1, QString(nm[0]).upper());

    if ( slant0 == 'r' ) {
	if ( tokens[QFontPrivate::Slant][1]) {
	    char slant1 = tolower( tokens[QFontPrivate::Slant][1] );

	    if ( slant1 == 'o' ) {
		nm += ' ';
		nm += qApp->translate("QFont","Reverse Oblique");
		*italic       = TRUE;
		*lesserItalic = TRUE;
	    } else if ( slant0 == 'i' ) {
		nm += ' ';
		nm += qApp->translate("QFont","Reverse Italic");
		*italic       = TRUE;
		*lesserItalic = TRUE;
	    }
	} else {
	    // Normal
	}
    } else if ( slant0 == 'o' ) {
	nm += ' ';

	if ( tokens[QFontPrivate::Slant][1] ) {
	    nm += qApp->translate("QFont","Other");
	} else {
	    nm += qApp->translate("QFont","Oblique");
	    *italic = TRUE;
	}
    } else if ( slant0 == 'i' ) {
	nm += ' ';
	nm += qApp->translate("QFont","Italic");
	*italic = TRUE;
    }

    if ( nm.isEmpty() ) {
	nm = qApp->translate("QFont","Normal");
    } else if ( nm[0] == ' ' ) {
	nm = nm.remove( 0, 1 );
    }

    return nm;
}

#endif // Q_WS_X11


static QStringList emptyList;


/*!
  \class QFontDatabase qfontdatabase.h
  \brief Provides information about available fonts.
  \ingroup environment

  QFontDatabase provides information about the available fonts of the
  underlying window system.

  Most often you will simply want to query the database for all font
  families(), and their respective pointSizes(), styles() and
  charSets().
*/


/*!
  Creates a font database object.
*/
QFontDatabase::QFontDatabase()
{
    createDatabase();

    d = db;
}


/*!
  Returns a list of names of all available font families.

  If a family exists in several foundries, the returned name for that font
  is "foundry-family".
*/
QStringList QFontDatabase::families() const
{
    return d->families();
}


/*!
  Returns all available styles of the font \a family.
*/
QStringList QFontDatabase::styles( const QString &family) const
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return emptyList;

    return fam->styles();
}


/*!
  Returns TRUE if the font that matches \a family and \a style
  is fixed pitch.
*/

bool QFontDatabase::isFixedPitch(const QString &family,
				 const QString &style) const
{
    const QtFontFamily *fam = d->family(family);
    if (! fam)
	return FALSE;

    if (style.isEmpty())
	return fam->isFixedPitch();

    const QtFontStyle *sty = fam->style(style);
    return sty && sty->isFixedPitch();
}


/*!
  Returns whether the font that matches \a family and \a style is a
  scalable bitmap font. Scaling a bitmap font produces a bad, often hardly
  readable, result because the pixels of the font are scaled. It's better
  to scale such a font only to the available fixed sizes (which you can
  get with smoothSizes()).

  \sa isScalable(), isSmoothlyScalable()
*/
bool  QFontDatabase::isBitmapScalable( const QString &family,
				       const QString &style) const
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return FALSE;

    if ( style.isEmpty() )
	return fam->isBitmapScalable();

    const QtFontStyle *sty = fam->style( style );
    return sty && sty->isBitmapScalable();
}


/*!
  Returns whether the font that matches \a family and \a style is smoothly
  scalable. If this function returns TRUE, it's safe to scale this font to
  every size. The result will always look decent.

  \sa isScalable(), isBitmapScalable()
*/
bool  QFontDatabase::isSmoothlyScalable( const QString &family,
					 const QString &style) const
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return FALSE;

    if ( style.isEmpty() )
	return fam->isSmoothlyScalable();

    const QtFontStyle *sty = fam->style( style );
    return sty && sty->isSmoothlyScalable();
}


/*!
  Returns TRUE if the font that matches the settings \a family and \a style
  is scalable.

  \sa isBitmapScalable(), isSmoothlyScalable()
*/
bool  QFontDatabase::isScalable( const QString &family,
				 const QString &style) const
{
    if ( isSmoothlyScalable( family, style) )
	return TRUE;

    return isBitmapScalable( family, style);
}


static const QtFontStyle * getStyle( QFontDatabase::Private *d,
				     const QString &family,
				     const QString &style)
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return 0;

    return fam->style( style );
}


static QValueList<int> emptySizeList;


/*!
  Returns a list of all available sizes of the font \a family in the
  style \a style.

  \sa smoothSizes(), standardSizes()
*/
QValueList<int> QFontDatabase::pointSizes( const QString &family,
					   const QString &style)
{
    QString s( style );
    if ( style.isEmpty() ) {
	QStringList lst = styles( family );
	s = lst.first();
    }

    const QtFontStyle *sty = getStyle( d, family, s);

    return sty ? sty->pointSizes() : emptySizeList;
}


/*!
  Returns a QFont object that matches the settings of \a family, \a style
  and \a pointsize. If no matching font could be created, an empty QFont
  object is returned.
*/
QFont QFontDatabase::font( const QString &family, const QString &style,
			   int pointSize)
{
    const QtFontStyle *sty = 0;

    sty =  getStyle( d, family, style);
    if ( !sty ) {
	qWarning( "QFontDatabase::font: Style not found for %s, %s, %d",
		  family.latin1(), style.latin1(), pointSize);
	return QFont();
    }

    return sty->font( family, pointSize );
}


/*!
  Returns the point sizes of a font that matches \a family and \a style
  that is guaranteed to look good. For non-scalable fonts and smoothly
  scalable fonts, this function is equivalent to pointSizes().

  \sa pointSizes(), standardSizes()
*/
QValueList<int> QFontDatabase::smoothSizes( const QString &family,
					    const QString &style)
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty ? sty->smoothSizes() : emptySizeList;
}


/*!
  Returns a list of standard font sizes.

  \sa smoothSizes(), pointSizes()
*/
QValueList<int> QFontDatabase::standardSizes()
{
    return QtFontStyle::standardSizes();
}


/*!
  Returns if the font that matches the settings \a family and \a style is
  italic or not.

  \sa weight(), bold()
*/
bool QFontDatabase::italic( const QString &family,
			    const QString &style) const
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty && sty->italic();
}


/*!  Returns whether the font that matches the settings \a family and \a
  style is bold or not.

  \sa italic(), weight()
*/
bool QFontDatabase::bold( const QString &family,
			  const QString &style) const
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty && sty->weight() >= QFont::Bold;
}


/*!
  Returns the weight of the font that matches the settings \a family and
  \a style.

  \sa italic(), bold()
*/
int QFontDatabase::weight( const QString &family,
			   const QString &style) const
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty ? sty->weight() : -1;
}

/*! \obsolete
  In Qt 3.0 and higher, this returns a QStringList containing only "Unicode".

  Previous versions returned a list of all character sets in which the
  font \a family was available.
*/
QStringList QFontDatabase::charSets( const QString & ) const
{
    static QStringList charSetsList;

    if (! charSetsList.count()) {
	charSetsList.append("Unicode");
    }

    return charSetsList;
}


/*!
  Returns a string that gives a default description of the \a script
  (e.g. for display in a dialog for the user).  The name matches the
  name of the script as indicated by Unicode 3.0.
*/
QString QFontDatabase::scriptName(QFont::Script script)
{
    const char *name = 0;

    switch (script) {
    case QFont::BasicLatin:
	name = "Basic Latin";
	break;
    case QFont::LatinExtendedA:
	name = "Latin Extended-A";
	break;
    case QFont::LatinExtendedB:
	name = "Latin Extended-B";
	break;
    case QFont::IPAExtensions:
	name = "IPA Extentions";
	break;
    case QFont::LatinExtendedAdditional:
	name = "Latin Extended Additional";
	break;
    case QFont::Greek:
	name = "Greek";
	break;
    case QFont::GreekExtended:
	name = "Greek Extended";
	break;
    case QFont::Cyrillic:
	name = "Cyrillic";
	break;
    case QFont::CyrillicHistoric:
	name = "Cyrillic Historic";
	break;
    case QFont::CyrillicExtended:
	name = "Cyrillic Extended";
	break;
    case QFont::Armenian:
	name = "Armenian";
	break;
    case QFont::Georgian:
	name = "Georgian";
	break;
    case QFont::Runic:
	name = "Runic";
	break;
    case QFont::Ogham:
	name = "Ogham";
	break;
    case QFont::Hebrew:
	name = "Hebrew";
	break;
	break;
    case QFont::Arabic:
	name = "Arabic";
	break;
    case QFont::Syriac:
	name = "Syriac";
	break;
    case QFont::Thaana:
	name = "Thaana";
	break;
    case QFont::Devanagari:
	name = "Devanagari";
	break;
    case QFont::Bengali:
	name = "Bengali";
	break;
    case QFont::Gurmukhi:
	name = "Gurmukhi";
	break;
    case QFont::Gujarati:
	name = "Gujarati";
	break;
    case QFont::Oriya:
	name = "Oriya";
	break;
    case QFont::Tamil:
	name = "Tamil";
	break;
    case QFont::Telugu:
	name = "Telugu";
	break;
    case QFont::Kannada:
	name = "Kannada";
	break;
    case QFont::Malayalam:
	name = "Malayalam";
	break;
    case QFont::Sinhala:
	name = "Sinhala";
	break;
    case QFont::Thai:
	name = "Thai";
	break;
    case QFont::Lao:
	name = "Lao";
	break;
    case QFont::Tibetan:
	name = "Tibetan";
	break;
    case QFont::Myanmar:
	name = "Myanmar";
	break;
    case QFont::Khmer:
	name = "Khmer";
	break;
    case QFont::UnifiedHan:
	name = "Unified Han";
	break;
    case QFont::Hiragana:
	name = "Hiragana";
	break;
    case QFont::Katakana:
	name = "Katakana";
	break;
    case QFont::Hangul:
	name = "Hangul";
	break;
    case QFont::Bopomofo:
	name = "Bopomofo";
	break;
    case QFont::Yi:
	name = "Yi";
	break;
    case QFont::Ethiopic:
	name = "Ethiopic";
	break;
    case QFont::Cherokee:
	name = "Cherokee";
	break;
    case QFont::CanadianAboriginal:
	name = "Canadian Aboriginal";
	break;
    case QFont::Mongolian:
	name = "Mongolian";
	break;
    case QFont::Unicode:
	name = "Unicode";
	break;
    default:
	name = "";
	break;
    }

    return qApp ? qApp->translate("QFont", name) : QString::fromLatin1(name);
}


/*!
  Returns a string with sample characters from \a script.
*/
QString QFontDatabase::scriptSample(QFont::Script script)
{
    QString sample = "AaBb";

    switch (script) {
    case QFont::BasicLatin:
	sample += QChar(0x00C3);
	sample += QChar(0x00E1);
	sample += "Zz";
	break;
    case QFont::LatinExtendedA:
	sample += QChar(0x0102);
	sample += QChar(0x011c);
	sample += QChar(0x0174);
	sample += QChar(0x0152);
	break;
    case QFont::LatinExtendedB:
	sample += QChar(0x018f);
	sample += QChar(0x019b);
	sample += QChar(0x020f);
	sample += QChar(0x0233);
	break;
    case QFont::IPAExtensions:
	sample += QChar(0x025f);
	sample += QChar(0x026f);
	sample += QChar(0x027f);
	sample += QChar(0x028f);
	break;
    case QFont::LatinExtendedAdditional:
	sample += QChar(0x1e0f);
	sample += QChar(0x1e4f);
	sample += QChar(0x1e8f);
	sample += QChar(0x1ecf);
	break;
    case QFont::Greek:
	sample += QChar(0x0393);
	sample += QChar(0x03B1);
	sample += QChar(0x03A9);
	sample += QChar(0x03C9);
	break;
    case QFont::GreekExtended:
	sample += QChar(0x1f00);
	sample += QChar(0x1f10);
	sample += QChar(0x1f20);
	sample += QChar(0x1f30);
	break;
    case QFont::Cyrillic:
	sample += QChar(0x0414);
	sample += QChar(0x0434);
	sample += QChar(0x0436);
	sample += QChar(0x0402);
	break;
    case QFont::CyrillicHistoric:
	sample += QChar(0x0464);
	sample += QChar(0x0474);
	sample += QChar(0x0466);
	sample += QChar(0x0472);
	break;
    case QFont::CyrillicExtended:
	sample += QChar(0x04a0);
	sample += QChar(0x04b0);
	sample += QChar(0x04c0);
	sample += QChar(0x04d0);
	break;
    case QFont::Armenian:
	sample += QChar(0x053f);
	sample += QChar(0x054f);
	sample += QChar(0x056f);
	sample += QChar(0x057f);
	break;
    case QFont::Georgian:
	sample += QChar(0x10a0);
	sample += QChar(0x10b0);
	sample += QChar(0x10c0);
	sample += QChar(0x10d0);
	break;
    case QFont::Runic:
	sample += QChar(0x16a0);
	sample += QChar(0x16b0);
	sample += QChar(0x16c0);
	sample += QChar(0x16d0);
	break;
    case QFont::Ogham:
	sample += QChar(0x1681);
	sample += QChar(0x1687);
	sample += QChar(0x1693);
	sample += QChar(0x168d);
	break;
    case QFont::Hebrew:
	sample += QChar(0x05D0);
	sample += QChar(0x05D1);
	sample += QChar(0x05D2);
	sample += QChar(0x05D3);
	break;
    case QFont::Arabic:
	sample += QChar(0x0628);
	sample += QChar(0x0629);
	sample += QChar(0x062A);
	sample += QChar(0x063A);
	break;
    case QFont::Syriac:
	sample += QChar(0x0715);
	sample += QChar(0x0725);
	sample += QChar(0x0716);
	sample += QChar(0x0726);
	break;
    case QFont::Thaana:
	sample += QChar(0x0784);
	sample += QChar(0x0794);
	sample += QChar(0x078c);
	sample += QChar(0x078d);
	break;
    case QFont::Devanagari:
	sample += QChar(0x0905);
	sample += QChar(0x0915);
	sample += QChar(0x0925);
	sample += QChar(0x0935);
	break;
    case QFont::Bengali:
	sample += QChar(0x0986);
	sample += QChar(0x0996);
	sample += QChar(0x09a6);
	sample += QChar(0x09b6);
	break;
    case QFont::Gurmukhi:
	sample += QChar(0x0a05);
	sample += QChar(0x0a15);
	sample += QChar(0x0a25);
	sample += QChar(0x0a35);
	break;
    case QFont::Gujarati:
	sample += QChar(0x0a85);
	sample += QChar(0x0a95);
	sample += QChar(0x0aa5);
	sample += QChar(0x0ab5);
	break;
    case QFont::Oriya:
	sample += QChar(0x0b06);
	sample += QChar(0x0b16);
	sample += QChar(0x0b2b);
	sample += QChar(0x0b36);
	break;
    case QFont::Tamil:
	sample += QChar(0x0b89);
	sample += QChar(0x0b99);
	sample += QChar(0x0ba9);
	sample += QChar(0x0bb9);
	break;
    case QFont::Telugu:
	sample += QChar(0x0c05);
	sample += QChar(0x0c15);
	sample += QChar(0x0c25);
	sample += QChar(0x0c35);
	break;
    case QFont::Kannada:
	sample += QChar(0x0c85);
	sample += QChar(0x0c95);
	sample += QChar(0x0ca5);
	sample += QChar(0x0cb5);
	break;
    case QFont::Malayalam:
	sample += QChar(0x0d05);
	sample += QChar(0x0d15);
	sample += QChar(0x0d25);
	sample += QChar(0x0d35);
	break;
    case QFont::Sinhala:
	sample += QChar(0x0d90);
	sample += QChar(0x0da0);
	sample += QChar(0x0db0);
	sample += QChar(0x0dc0);
	break;
    case QFont::Thai:
	sample += QChar(0x0e02);
	sample += QChar(0x0e12);
	sample += QChar(0x0e22);
	sample += QChar(0x0e32);
	break;
    case QFont::Lao:
	sample += QChar(0x0e8d);
	sample += QChar(0x0e9d);
	sample += QChar(0x0ead);
	sample += QChar(0x0ebd);
	break;
    case QFont::Tibetan:
	sample += QChar(0x0f00);
	sample += QChar(0x0f01);
	sample += QChar(0x0f02);
	sample += QChar(0x0f03);
	break;
    case QFont::Myanmar:
	sample += QChar(0x1000);
	sample += QChar(0x1001);
	sample += QChar(0x1002);
	sample += QChar(0x1003);
	break;
    case QFont::Khmer:
	sample += QChar(0x1780);
	sample += QChar(0x1790);
	sample += QChar(0x17b0);
	sample += QChar(0x17c0);
	break;
    case QFont::UnifiedHan:
	sample += QChar(0x6f84);
	sample += QChar(0x820a);
	sample += QChar(0x61a9);
	sample += QChar(0x9781);
	break;
    case QFont::Hiragana:
	sample += QChar(0x3050);
	sample += QChar(0x3060);
	sample += QChar(0x3070);
	sample += QChar(0x3080);
	break;
    case QFont::Katakana:
	sample += QChar(0x30b0);
	sample += QChar(0x30c0);
	sample += QChar(0x30d0);
	sample += QChar(0x30e0);
	break;
    case QFont::Hangul:
	sample += QChar(0xac00);
	sample += QChar(0xac11);
	sample += QChar(0xac1a);
	sample += QChar(0xac2f);
	break;
    case QFont::Bopomofo:
	sample += QChar(0x3105);
	sample += QChar(0x3115);
	sample += QChar(0x3125);
	sample += QChar(0x3129);
	break;
    case QFont::Yi:
	sample += QChar(0xa1a8);
	sample += QChar(0xa1a6);
	sample += QChar(0xa200);
	sample += QChar(0xa280);
	break;
    case QFont::Ethiopic:
	sample += QChar(0x1200);
	sample += QChar(0x1240);
	sample += QChar(0x1280);
	sample += QChar(0x12c0);
	break;
    case QFont::Cherokee:
	sample += QChar(0x13a0);
	sample += QChar(0x13b0);
	sample += QChar(0x13c0);
	sample += QChar(0x13d0);
	break;
    case QFont::CanadianAboriginal:
	sample += QChar(0x1410);
	sample += QChar(0x1500);
	sample += QChar(0x15f0);
	sample += QChar(0x1650);
	break;
    case QFont::Mongolian:
	sample += QChar(0x1820);
	sample += QChar(0x1840);
	sample += QChar(0x1860);
	sample += QChar(0x1880);
	break;
    case QFont::Unicode:
	sample += QChar(0x0174);
	sample += QChar(0x0628);
	sample += QChar(0x0e02);
	sample += QChar(0x263A);
	sample += QChar(0x3129);
	sample += QChar(0x61a9);
	sample += QChar(0xac2f);
	break;
    default:
	sample += QChar(0xfffd);
	sample += QChar(0xfffd);
	sample += QChar(0xfffd);
	sample += QChar(0xfffd);
	break;
    }

    return sample;
}


#ifdef Q_WS_QWS

void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new Private;

    QDiskFont * qdf;
    QString fname="Truetype";
    QtFontFoundry * foundry=new QtFontFoundry(fname);
    db->addFoundry(foundry);

    if(!qt_fontmanager)
	qt_fontmanager=new QFontManager();

    for(qdf=qt_fontmanager->diskfonts.first();qdf!=0;
	qdf=qt_fontmanager->diskfonts.next()) {
	QString familyname=qdf->name;
	QtFontFamily * family=foundry->familyDict.find(familyname);
	if(!family) {
	    family=new QtFontFamily(foundry,familyname);
	    foundry->addFamily(family);
	}
	QString weightString;
	int weight=qdf->weight;
	if ( weight <= QFont::Light ) {
	    weight = QFont::Light;
	    weightString = "Light";
	} else if ( weight <= QFont::Normal ) {
	    weight = QFont::Normal;
	    weightString = "Regular";
	} else if ( weight <= QFont::DemiBold ) {
	    weight = QFont::DemiBold;
	    weightString = "DemiBold";
	} else if ( weight <= QFont::Bold ) {
	    weight = QFont::Bold;
	    weightString = "Bold";
	} else {
	    weight = QFont::Black;
	    weightString = "Black";
	}
	QString style;
	if(qdf->italic) {
	    style=weightString+" Italic";
	} else {
	    style=weightString;
	}
	QtFontStyle * mystyle=family->styleDict.find(style);
	if(!mystyle) {
	    mystyle=new QtFontStyle(family,style);
	    mystyle->ital=qdf->italic;
	    mystyle->lesserItal=false;
	    mystyle->weightString=weightString;
	    mystyle->weightVal=weight;
	    mystyle->weightDirty=false;
	    family->addStyle(mystyle);
	}
	mystyle->setSmoothlyScalable();
    }
}

#endif // Q_WS_QWS

#endif // QT_NO_FONTDATABASE
