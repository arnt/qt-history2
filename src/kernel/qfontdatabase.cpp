/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdatabase.cpp#24 $
**
** Implementation of font database class.
**
** Created : 990603
**
** Copyright (C) 1999-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qfontdatabase.h"

#ifndef QT_NO_FONTDATABASE

#ifdef _WS_QWS_
#include "qfontmanager_qws.h"
#endif
#include "qmap.h"
#include "qdict.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qapplication.h"
#include "qpainter.h"
#include <stdlib.h>
#include <ctype.h>

// NOT REVISED
#ifdef _WS_MAC_
extern int qFontGetWeight( const QCString &weightString, bool adjustScore=FALSE );
#endif

#ifdef _WS_X11_
#include "qt_x11.h"


static const int fontFields = 14;

enum FontFieldNames {				// X LFD fields
    Foundry,
    Family,
    Weight_,
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
    CharsetEncoding };

#undef	IS_ZERO
#define IS_ZERO(X) (X[0] == '0' && X[1] == 0)

static inline bool isScalable( char **tokens )
{
    return ( IS_ZERO(tokens[PixelSize]) &&
	     IS_ZERO(tokens[PointSize]) &&
	     IS_ZERO(tokens[AverageWidth]) );
}

static inline bool isSmoothlyScalable( char **tokens )
{
    return ( IS_ZERO( tokens[ResolutionX] ) &&
	     IS_ZERO( tokens[ResolutionY] ) );
}

extern int qFontGetWeight( const QCString &weightString, bool adjustScore=FALSE );

extern bool qParseXFontName( QCString &fontName, char **tokens );

static char **xFontList = 0;
static int xFontCount = 0;

#if 0
static QFont::CharSet getCharSet( const char * registry, const char *encoding);
#endif
static QString getStyleName( char ** tokens, bool *italic,bool *lesserItalic );
static QString getCharSetName( const char * registry, const char *encoding );
static QString getCharSetName( QFont::CharSet cs );
#endif

#ifdef _WS_WIN_
#include "qt_windows.h"

extern int qFontGetWeight( const QCString &/*weightString*/,
			   bool /*adjustscore*/ = FALSE )
{
    // #####eiriken? Not used currently
    return 0;
}

static void populate_database(const QString& fam);

#endif

#ifdef _WS_QWS_
extern int qFontGetWeight( const QCString &/*weightString*/,
			   bool /*adjustscore*/ = FALSE )
{
    // dummy
    return 0;
}
#endif

static QFont::CharSet getCharSet( const QString &name );
class QtFontCharSet;
#ifdef _WS_WIN_
static void newWinFont( void * p );
static void add_style( QtFontCharSet *charSet, const QString& styleName,
		bool italic, bool lesserItalic, int weight );
#endif

class QtFontCharSet;
class QtFontFamily;
class QtFontFoundry;

class QtFontStyle
{
public:
    QtFontStyle( QtFontCharSet *prnt, const QString &n )
		       { p                = prnt;
			 nm		  = n;
			 bitmapScalable   = FALSE;
			 smoothlyScalable = FALSE;
			 weightDirty      = TRUE;
			 ital             = FALSE;
			 lesserItal       = FALSE;
			 weightVal        = 0;
			 weightDirty      = TRUE;
			 sizesDirty       = TRUE; }

    QFont font( const QString &family, int pointSize ) const;  // ### fttb
    QFont font( int pointSize ) const;

    const QString &name() const { return nm; }
    const QtFontCharSet *parent() const { return p; }

    const QValueList<int> &pointSizes() const;
    const QValueList<int> &smoothSizes() const;
    static const QValueList<int> &standardSizes();

    int weight() const;
    bool italic() const { return ital || lesserItal; }
    bool lesserItalic() const { return lesserItal; }

    bool isBitmapScalable() const { return bitmapScalable; }
    bool isSmoothlyScalable() const { return smoothlyScalable; }


private:
    const QValueList<int> &storedSizes() const;

    void addPointSize( int size );
    void setSmoothlyScalable();
    void setBitmapScalable();


    QtFontCharSet *p;
    QString nm;

    bool bitmapScalable;
    bool smoothlyScalable;

    bool ital;
    bool lesserItal;
    QCString weightString;
    int  weightVal;
    bool weightDirty;
    bool sizesDirty;

    QMap<int, int> sizeMap;
    QValueList<int> sizeList;

    friend void QFontDatabase::createDatabase();
#ifdef _WS_WIN_
    friend void newWinFont( void * p );
    friend void add_style( QtFontCharSet *charSet, const QString& styleName,
		bool italic, bool lesserItalic, int weight );
#endif
};

class QtFontCharSet {
public:
    QtFontCharSet( QtFontFamily *prnt, const QString n )
			  { p                = prnt;
			    nm               = n;
			    // charSet          = QFont::AnyCharSet;
			    dirty            = TRUE;
			    namesDirty       = TRUE;
			    bitmapScalable   = FALSE;
			    smoothlyScalable = FALSE;
			    normalStyle      = 0;
			    italicStyle      = 0;
			    boldStyle        = 0;
			    italicBoldStyle  = 0;
			    chSetDirty	     = TRUE;
			    chSet	     = QFont::AnyCharSet;
			  }


    const QString &name() const { return nm; }
    QFont::CharSet charSet() const;

    const QtFontFamily *parent() const { return p; }

    const QStringList &styles() const;
    const QtFontStyle *style( const QString &name ) const;

    bool isLocaleCharSet() const;
    bool isUnicode() const;

    bool isBitmapScalable() const;
    bool isSmoothlyScalable() const;

private:
    void refresh() const;

    QtFontFamily *p;
    QString nm;

    bool dirty;
    bool namesDirty;
    bool bitmapScalable;
    bool smoothlyScalable;

    bool chSetDirty;
    QFont::CharSet chSet;

    QtFontStyle *normalStyle; // Only makes sense if the font is scalable
    QtFontStyle *italicStyle; // Gives information about which
    QtFontStyle *boldStyle;   //  combinations of these are available.
    QtFontStyle *italicBoldStyle;


    void addStyle( QtFontStyle *style )
	{ styleDict.insert( style->name(), style );  }

    QDict<QtFontStyle> styleDict;
    QStringList styleNames;

    friend void QFontDatabase::createDatabase();
#ifdef _WS_WIN_
    friend void newWinFont( void * p );
    friend void add_style( QtFontCharSet *charSet, const QString& styleName,
		bool italic, bool lesserItalic, int weight );
#endif
};

class QtFontFamily
{
public:
    QtFontFamily( QtFontFoundry *prnt, const QString &n )
	{ p                = prnt;
	  nm               = n;
	  namesDirty       = TRUE;
	  bitmapScalable   = FALSE;
	  smoothlyScalable = FALSE;
	  scalableDirty    = TRUE;
	  localeDirty      = TRUE;
	  supportsLocale   = FALSE;
	}

    const QString &name() const { return nm; }

    const QtFontFoundry *parent() { return p; }

    const QStringList &charSets( bool onlyForLocale = TRUE ) const;
    const QtFontCharSet *charSet( const QString &n = QString::null ) const;

    bool isBitmapScalable() const;
    bool isSmoothlyScalable() const;

    bool hasLocaleCharSet() const;
    bool supportsCharSet( QFont::CharSet chSet ) const;

private:
    void refresh() const;
    void addCharSet( QtFontCharSet *c )
	{ charSetDict.insert( c->name(), c ); }

    QString nm;
    QtFontFoundry *p;

    // QList<QtFontCharSet> charSets;
    QStringList charSetNames;
    QDict<QtFontCharSet> charSetDict;

    bool namesDirty;
    bool localeDirty;
    bool scalableDirty;

    bool bitmapScalable;
    bool smoothlyScalable;
    bool supportsLocale;

    friend void QFontDatabase::createDatabase();
#ifdef _WS_WIN_
    friend void newWinFont( void * p );
    friend void add_style( QtFontCharSet *charSet, const QString& styleName,
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
#ifdef _WS_WIN_
    friend void newWinFont( void * p );
#endif
};

class QFontDatabasePrivate {
public:
    QFontDatabasePrivate(){
	namesDirty  	    = TRUE;
	familiesDirty  	    = TRUE;
	foundryDict.setAutoDelete( TRUE );
    }

    const QStringList &families( bool onlyForLocale ) const;
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
#ifdef _WS_WIN_
    friend void newWinFont( void * p );
#endif
};

QFont QtFontStyle::font( const QString & family, int pointSize ) const
{
    QFont::CharSet charSet = getCharSet( parent()->name() );  // ### fttb

    QFont f( family, pointSize, weight(), italic() );
    f.setCharSet( charSet );
    return f;
}

QFont QtFontStyle::font( int pointSize ) const
{
    return font( parent()->parent()->name(), pointSize );
}

const QValueList<int> &QtFontStyle::pointSizes() const
{
    if ( smoothlyScalable || bitmapScalable )
	return standardSizes();
    else
	return storedSizes();
}

const QValueList<int> &QtFontStyle::smoothSizes() const
{
    if ( smoothlyScalable )
	return standardSizes();
    else
	return storedSizes();
}

int QtFontStyle::weight() const
{
    if ( weightDirty ) {
	QtFontStyle *that = (QtFontStyle*)this; // mutable function
	that->weightVal = qFontGetWeight( weightString, TRUE );
	that->weightDirty = FALSE;
    }
    return weightVal;
}

const QValueList<int> &QtFontStyle::storedSizes() const
{
    if ( sizesDirty ) {
	QtFontStyle *that = (QtFontStyle*)this;  // Mutable function
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

static
int styleSortValue( QtFontStyle *style )
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

QFont::CharSet QtFontCharSet::charSet() const
{
    if ( chSetDirty ) {
	QtFontCharSet *that = (QtFontCharSet*)this;  // Mutable function
	that->chSet      = getCharSet( name() );
	that->chSetDirty = FALSE;
    }
    return chSet;
}

const QStringList &QtFontCharSet::styles() const
{
    if ( namesDirty ) {
	QtFontCharSet *that = (QtFontCharSet*) this;  // Mutable function
#ifdef _WS_WIN_
	// Lazy evaluation
	populate_database(parent()->name());
#endif
	QMap<QString, QString> styleMap;
	QDictIterator<QtFontStyle> iter( styleDict );
	QtFontStyle *tmp;
	for( ; (tmp = iter.current()) ; ++iter ) {
	    styleMap.insert( QString().setNum(styleSortValue( tmp ))+
			tmp->name(), tmp->name() );
	}
	QMap<QString,QString>::Iterator it = styleMap.begin();
	for ( ; it != styleMap.end(); ++it )
	    that->styleNames.append( *it );
	that->namesDirty = FALSE;
    }
    return styleNames;
}

const QtFontStyle *QtFontCharSet::style( const QString &s ) const
{
    return styleDict.find( s );
}

bool QtFontCharSet::isLocaleCharSet() const
{
    return charSet() == QFont::charSetForLocale();
}

bool QtFontCharSet::isUnicode() const
{
    return charSet() == QFont::Unicode;
}

bool QtFontCharSet::isBitmapScalable() const
{
    refresh();
    return bitmapScalable;
}

bool QtFontCharSet::isSmoothlyScalable() const
{
    refresh();
    return smoothlyScalable;
}

/*!
  Traverses all styles. If all of them are scalable, scalable is set to
  TRUE, if all of them are smoothly scalable smoothlyScalable is set to
  TRUE.

  The styles that most closely resemble a normal, italic, bold and bold
  italc font are found.
*/
void QtFontCharSet::refresh() const
{
    if ( !dirty )
	return;
    QtFontCharSet *that = (QtFontCharSet*)this; // mutable function
    that->smoothlyScalable = FALSE;
    that->bitmapScalable   = FALSE;

    that->normalStyle       = 0;
    that->italicStyle       = 0;
    that->boldStyle         = 0;
    that->italicBoldStyle   = 0;

    QtFontStyle *lesserItalicStyle     = 0;
    QtFontStyle *lesserItalicBoldStyle = 0;

    bool smooth = TRUE;
    bool bitmap = TRUE;
		 // Anything bolder than Normal qualifies as bold:
    int  bestBoldDiff             = QFont::Bold - QFont::Normal;
    int  bestItalicBoldDiff       = QFont::Bold - QFont::Normal;
    //int  bestLesserItalicBoldDiff = QFont::Bold - QFont::Normal; NOT USED
    int  bestNormal               = 0;
    int  bestItalicNormal         = 0;
    int  bestLesserItalicNormal   = 0;
    int  boldDiff;
    QtFontStyle *tmp;
    QDictIterator<QtFontStyle> iter(styleDict);
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( !tmp->isSmoothlyScalable() ) {
	    smooth = FALSE;
	    if ( !tmp->isBitmapScalable() )
		bitmap = FALSE;
	}
	if ( tmp->italic() ) {
	    if ( tmp->weight() < QFont::Normal ) {
		if ( tmp->weight() > bestItalicNormal ) {
		    that->italicStyle      = tmp;
		    bestItalicNormal = tmp->weight();
		}
	    } else {
		boldDiff = abs( tmp->weight() - QFont::Bold );
		if ( boldDiff < bestItalicBoldDiff ) {
		    that->italicBoldStyle    = tmp;
		    bestItalicBoldDiff = boldDiff;
		}

	    }
	} else if ( tmp->lesserItalic() ){
	    if ( tmp->weight() < QFont::Normal ) {
		if ( tmp->weight() > bestLesserItalicNormal ) {
		    lesserItalicStyle      = tmp;
		    bestLesserItalicNormal = tmp->weight();
		}
	    } else {
		boldDiff = abs( tmp->weight() - QFont::Bold );
		if ( boldDiff < bestItalicBoldDiff ) {
		    lesserItalicBoldStyle    = tmp;
		    //bestLesserItalicBoldDiff = boldDiff; NOT USED
		}

	    }
	} else {
	    if ( tmp->weight() < QFont::Normal ) {
		if ( tmp->weight() > bestNormal ) {
		    that->normalStyle = tmp;
		    bestNormal  = tmp->weight();
		}
	    } else {
		boldDiff = abs( tmp->weight() - QFont::Bold );
		if ( boldDiff < bestBoldDiff ) {
		    that->boldStyle    = tmp;
		    bestBoldDiff = boldDiff;
		}

	    }
	}
    }
    if ( !that->italicStyle && lesserItalicStyle )
	that->italicStyle = lesserItalicStyle;
    if ( !that->italicBoldStyle && lesserItalicBoldStyle )
	that->italicBoldStyle = lesserItalicBoldStyle;
    if ( smooth )
	that->smoothlyScalable = TRUE;
    else if ( bitmap )
	that->bitmapScalable = TRUE;
    that->dirty    = FALSE;
}

const QStringList &QtFontFamily::charSets( bool onlyForLocale ) const
{
    QtFontFamily *that = (QtFontFamily*)this; // mutable function
    if ( namesDirty ) {
	QDictIterator<QtFontCharSet> iter( charSetDict );
	QtFontCharSet *tmp;
	QString unicode;
	QString local;
	for( ; (tmp = iter.current()) ; ++iter ) {
	    if ( tmp->isLocaleCharSet() )
		local = tmp->name();
	    else if ( tmp->isUnicode() )
		unicode = tmp->name();
	    else if ( !onlyForLocale )
		that->charSetNames.append( tmp->name() );
	}
	that->charSetNames.sort();
	if ( !!unicode )
	    that->charSetNames.prepend( unicode ); // preferred second
	if ( !!local )
	    that->charSetNames.prepend( local ); // preferred first
	that->namesDirty = FALSE;
    }
    return that->charSetNames;
}

static
QString localCharSet()
{
    return "iso10646-1";
}

const QtFontCharSet *QtFontFamily::charSet( const QString &n ) const
{
    if ( n.isEmpty() )
	return charSetDict.find ( localCharSet() );
    else
	return charSetDict.find ( n );
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

bool QtFontFamily::hasLocaleCharSet() const
{
    if ( localeDirty ) {
	QtFontFamily *that   = (QtFontFamily*)this; // mutable function
	QDictIterator<QtFontCharSet> iter( charSetDict );
	QtFontCharSet *tmp;
	that->supportsLocale = FALSE;
	for( ; (tmp = iter.current()) ; ++iter ) {
	    if ( tmp->isLocaleCharSet() || tmp->isUnicode() ) {
		that->supportsLocale = TRUE;
		break;
	    }
	}
	that->localeDirty = FALSE;
    }
    return supportsLocale;
}

bool QtFontFamily::supportsCharSet( QFont::CharSet chSet ) const
{
    QDictIterator<QtFontCharSet> iter( charSetDict );
    QtFontCharSet *tmp;
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( tmp->charSet() == chSet )
	    return TRUE;
    }
    return FALSE;
}

void QtFontFamily::refresh() const
{
    if ( !scalableDirty )
	return;
    QtFontFamily *that = (QtFontFamily*) this;   // Mutable function
    that->scalableDirty    = FALSE;
    that->smoothlyScalable = FALSE;
    that->bitmapScalable   = FALSE;

    bool isSmooth = TRUE;
    QtFontCharSet *tmp;
    QDictIterator<QtFontCharSet> iter(charSetDict);
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( !tmp->isSmoothlyScalable() ) {
	    isSmooth = FALSE;
	    if ( !tmp->isBitmapScalable() )
		return;
	}
    }
    if ( isSmooth )
	that->smoothlyScalable = TRUE;
    else
	that->bitmapScalable   = TRUE;
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

static bool localeNeedsSet()
{
    return (QFont::charSetForLocale() >= QFont::Set_1 &&
	   QFont::charSetForLocale() <= QFont::Set_N)  ||
	   QFont::charSetForLocale() == QFont::Set_GBK ||
	   QFont::charSetForLocale() == QFont::Set_Big5;
}

const QStringList &QFontDatabasePrivate::families( bool onlyForLocale ) const
{
    QFontDatabasePrivate *that = (QFontDatabasePrivate*)this; // Mutable
    if ( familiesDirty ) {
	QDict<QtFontFoundry> firstFoundryForFamily;
	QDict<int> doubles;
	QtFontFoundry *foundry;
	QString s;
	QDictIterator<QtFontFoundry> iter( foundryDict );
	for( ; (foundry = iter.current()) ; ++iter ) {
	    QStringList l = foundry->families();
	    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
		if ( onlyForLocale ) {
		     const QtFontFamily *tmp = foundry->family( *it );
		     if ( !tmp ) {
			 qWarning( "QFontDatabasePrivate::families:"
				   "Internal error, %s not found.",
				   (const char*)*it );
			 continue;
		     }
		     if ( !localeNeedsSet() && !tmp->hasLocaleCharSet() )
			 continue;
		}
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
			    qWarning( "QFontDatabasePrivate::families:"
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


const QtFontFamily *QFontDatabasePrivate::family( const QString &name ) const
{
    if ( name.isEmpty() )
	return 0;
    QFontDatabasePrivate *that = (QFontDatabasePrivate*)this; // Mutable
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

const QStringList &QFontDatabasePrivate::foundries() const
{
    if ( namesDirty ) {
	QFontDatabasePrivate *that = (QFontDatabasePrivate*) this;  // Mutable
	QDictIterator<QtFontFoundry> iter( foundryDict );
	QtFontFoundry *tmp;
	for( ; (tmp = iter.current()) ; ++iter )
	    that->foundryNames.append( tmp->name() );
	that->foundryNames.sort();
	that->namesDirty = FALSE;
    }
    return foundryNames;

}

const QtFontFoundry *QFontDatabasePrivate::foundry( const QString foundryName ) const
{
    return foundryDict.find( foundryName );
}

static QFontDatabasePrivate *db=0;

#ifdef _WS_X11_
char ** readFontDump( const char *fileName, int *xFontCount )
{
    QFile f( fileName );
    if ( f.open( IO_ReadOnly ) ) {
	char ** result = new char* [32768];
	QTextStream is( &f );
	is.setEncoding( QTextStream::Latin1 );
	int i = 0;
	QString s;
	while( !is.atEnd() && i < 32768 ) {
	    s = is.readLine();
	    if ( !s.isEmpty() ) {
		result[i] = new char[s.length() + 1]; // Memory hog, oink!
		qstrcpy( result[i], s.latin1() );
	    }
	    i++;
	}
	*xFontCount = i;
	return result;
    } else {
	*xFontCount = 0;
	return 0;
    }
}

void QFontDatabase::createDatabase()
{
    if ( db ) return;

    db = new QFontDatabasePrivate;

#if 0
    xFontList = readFontDump( "test.fonts", &xFontCount );
    if ( xFontList )
	qWarning("Read font definitions from the file \"test.fonts\""
		 " for debugging\n"
		 "Expect strange results in the font dialog.\n" );
    else
#endif
    xFontList = XListFonts( qt_xdisplay(), "*", 32767, &xFontCount );

    if ( xFontCount >= 32767 )
	qWarning( "More than 32k fonts, please notify qt-bugs@trolltech.com" );

    char *tokens[fontFields];

    for( int i = 0 ; i < xFontCount ; i++ ) {
	QCString fName = xFontList[i];
	if ( qParseXFontName( fName, tokens ) ) {
	    QString foundryName = tokens[Foundry];
	    QtFontFoundry *foundry = db->foundryDict.find( foundryName );
	    if ( !foundry ) {
		//qWarning( "New font foundry [%s]", (const char*) foundryName );
		foundry = new QtFontFoundry( foundryName );
		CHECK_PTR(foundry);
		db->addFoundry( foundry );
	    }
	    QString familyName = tokens[Family];
	    QtFontFamily *family = foundry->familyDict.find( familyName );
	    if ( !family ) {
		//qWarning( "New font family [%s][%s]",
		// (const char*) familyName, (const char*) foundryName );
		family = new QtFontFamily( foundry, familyName );
		CHECK_PTR(family);
		foundry->addFamily( family );
	    }
	    QString charSetName = getCharSetName( tokens[CharsetRegistry],
						 tokens[CharsetEncoding] );
	    QtFontCharSet *charSet = family->charSetDict.find( charSetName );
	    if ( !charSet ) {
		//qWarning( "New charset[%s] for family [%s][%s]",
		// (const char*)charSetName, (const char *)familyName,
		// (const char *)foundryName );
		charSet = new QtFontCharSet( family, charSetName );
		CHECK_PTR(charSet);
		family->addCharSet( charSet );
	    }
	    bool italic;
	    bool lesserItalic;
	    QString styleName = getStyleName( tokens, &italic, &lesserItalic );
	    QtFontStyle *style = charSet->styleDict.find( styleName );
	    if ( !style ) {
		//qWarning( "New style[%s] for [%s][%s][%s]",
		// (const char*)styleName, (const char*)charSetName,
		// (const char*)familyName, (const char *)foundryName );
		style = new QtFontStyle( charSet, styleName );
		CHECK_PTR( style );
		style->ital         = italic;
		style->lesserItal   = lesserItalic;
		style->weightString = tokens[Weight_];
		charSet->addStyle( style );
	    }
	    if ( ::isScalable(tokens) ) {
		if ( ::isSmoothlyScalable( tokens ) ) {
		    style->setSmoothlyScalable();
		    //qWarning( "Smooth [%s][%s][%s]", (const char*) styleName,
		    //     (const char*)charSetName,
		    //     tokens[Family] );
		} else {
		    style->setBitmapScalable();
		    //qWarning( "Scalable, [%s][%s]", (const char*)charSetName,
		    //     tokens[Family] );
		}
	    } else {
		QCString ps = tokens[PointSize];
		int pSize = ps.toInt()/10;
		if ( pSize != 0 ) {
		    style->addPointSize( pSize );
		}
	    }
	} else {
	    //qWarning( "createDatabase: Not XLFD[%s]", xFontList[i] );
	}
    }
}

#endif


#ifdef _WS_WIN_

extern Qt::WindowsVersion qt_winver;		// defined in qapplication_win.cpp

static
int CALLBACK
storeFont( ENUMLOGFONTEX* f, TEXTMETRIC*, int /*type*/, LPARAM /*p*/ )
{
    //QFontDatabasePrivate* d = (QFontDatabasePrivate*)p;

    newWinFont( (void*) f );
    return 1; // Keep enumerating.
}

static
QString winGetCharSetName( BYTE /*chset*/ )
{
    // ##### Could give hints to the user
    return "Unicode";
}

static
void add_style( QtFontCharSet *charSet, const QString& styleName,
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
    QtFontStyle *style = charSet->styleDict.find( sn );
    if ( !style ) {
	//qWarning( "New style[%s] for [%s][%s][%s]",
	// (const char*)styleName, (const char*)charSetName,
	// (const char*)familyName, (const char *)foundryName );
	style = new QtFontStyle( charSet, sn );
	CHECK_PTR( style );
	style->ital         = italic;
	style->lesserItal   = lesserItalic;
	style->weightString = weightString;
	style->weightVal    = weight;
	style->weightDirty  = FALSE;
	charSet->addStyle( style );
    }
    //#### eiriken?
#if 0
else
debug("Already got it");
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
	CHECK_PTR(family);
	foundry->addFamily( family );
    }
    QString charSetName = winGetCharSetName( f->elfLogFont.lfCharSet );
    QtFontCharSet *charSet = family->charSetDict.find( charSetName );
    if ( !charSet ) {
	//qWarning( "New charset[%s] for family [%s][%s]",
	// (const char*)charSetName, (const char *)familyName,
	// (const char *)foundryName );
	charSet = new QtFontCharSet( family, charSetName );
	CHECK_PTR(charSet);
	family->addCharSet( charSet );
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
debug("%s with quality %x",familyName.latin1(),f->elfLogFont.lfQuality);
#endif
	add_style( charSet, styleName, FALSE, FALSE, weight );
	add_style( charSet, styleName, FALSE, TRUE, weight );

	if ( weight < QFont::DemiBold ) {
	    // Can make bolder
	    add_style( charSet, styleName, FALSE, FALSE, QFont::Bold );
	    add_style( charSet, styleName, FALSE, TRUE, QFont::Bold );
	}
    } else {
	if ( italic ) {
	    add_style( charSet, styleName, italic, FALSE, weight );
	} else {
	    add_style( charSet, styleName, italic, FALSE, weight );
	    add_style( charSet, QString::null, italic, TRUE, weight );
	}
	if ( weight < QFont::DemiBold ) {
	    // Can make bolder
	    if ( italic )
		add_style( charSet, QString::null, italic, FALSE, QFont::Bold );
	    else {
		add_style( charSet, QString::null, FALSE, FALSE, QFont::Bold );
		add_style( charSet, QString::null, FALSE, TRUE, QFont::Bold );
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
    db = new QFontDatabasePrivate;
    populate_database(0);
}


#endif




#if 0
static QFont::CharSet getCharSet( const char * registry, const char *encoding )
{
    if ( qstrcmp( registry, "iso8859" ) == 0 ) {
	if ( encoding[0] != 0 && encoding[1] == 0 ) {
	    switch( encoding[0] ) {
	    case '1': return QFont::ISO_8859_1;
	    case '2': return QFont::ISO_8859_2;
	    case '3': return QFont::ISO_8859_3;
	    case '4': return QFont::ISO_8859_4;
	    case '5': return QFont::ISO_8859_5;
	    case '6': return QFont::ISO_8859_6;
	    case '7': return QFont::ISO_8859_7;
	    case '8': return QFont::ISO_8859_8;
	    case '9': return QFont::ISO_8859_9;
	    default: break;
	    }
	} else if ( encoding[0] == '1' && encoding[1] != 0
		    && encoding[2] == 0 ) {
	    switch( encoding[0] ) {
	    case '0': return QFont::ISO_8859_10;
	    case '1': return QFont::ISO_8859_11;
	    case '2': return QFont::ISO_8859_12;
	    case '3': return QFont::ISO_8859_13;
	    case '4': return QFont::ISO_8859_14;
	    case '5': return QFont::ISO_8859_15;
	    default: break;
	    }
	}
	return QFont::AnyCharSet;
    } else if ( qstrcmp( registry, "koi8" ) == 0 &&
		(qstrcmp( encoding, "r" ) == 0 ||
		 qstrcmp( encoding, "1" ) == 0) ) {
	return QFont::KOI8R;
    } else if ( qstrcmp( registry, "iso10646" ) == 0 ) {
	return QFont::Unicode;
    }
    return QFont::AnyCharSet;
}
#endif

static QFont::CharSet getCharSet( const QString &name )
{
    if ( name == "iso8859-1" )
	return QFont::ISO_8859_1;
    if ( name == "iso8859-2" )
	return QFont::ISO_8859_2;
    if ( name == "iso8859-3" )
	return QFont::ISO_8859_3;
    if ( name == "iso8859-4" )
	return QFont::ISO_8859_4;
    if ( name == "iso8859-5" )
	return QFont::ISO_8859_5;
    if ( name == "iso8859-6" )
	return QFont::ISO_8859_6;
    if ( name == "iso8859-7" )
	return QFont::ISO_8859_7;
    if ( name == "iso8859-8" )
	return QFont::ISO_8859_8;
    if ( name == "iso8859-9" )
	return QFont::ISO_8859_9;
    if ( name == "iso8859-10" )
	return QFont::ISO_8859_10;
    if ( name == "iso8859-11" )
	return QFont::ISO_8859_11;
    if ( name == "iso8859-12" )
	return QFont::ISO_8859_12;
    if ( name == "iso8859-13" )
	return QFont::ISO_8859_13;
    if ( name == "iso8859-14" )
	return QFont::ISO_8859_14;
    if ( name == "iso8859-15" )
	return QFont::ISO_8859_15;
    if ( name == "koi8-r" )
	return QFont::KOI8R;
    if ( name == "koi8-1" )
	return QFont::KOI8R;
    if ( name == "iso10646-1" )
	return QFont::Unicode;
    return QFont::AnyCharSet;
}

static const QString getCharSet( QFont::CharSet set)
{
    if ( set == QFont::ISO_8859_1 )
        return "iso8859-1";
    if ( set == QFont::ISO_8859_2 )
        return "iso8859-2";
    if ( set == QFont::ISO_8859_3 )
        return "iso8859-3";
    if ( set == QFont::ISO_8859_4 )
        return "iso8859-4";
    if ( set == QFont::ISO_8859_5 )
        return "iso8859-5";
    if ( set == QFont::ISO_8859_6 )
        return "iso8859-6";
    if ( set == QFont::ISO_8859_7 )
        return "iso8859-7";
    if ( set == QFont::ISO_8859_8 )
        return "iso8859-8";
    if ( set == QFont::ISO_8859_9 )
        return "iso8859-9";
    if ( set == QFont::ISO_8859_10 )
        return "iso8859-10";
    if ( set == QFont::ISO_8859_11 )
        return "iso8859-11";
    if ( set == QFont::ISO_8859_12 )
        return "iso8859-12";
    if ( set == QFont::ISO_8859_13 )
        return "iso8859-13";
    if ( set == QFont::ISO_8859_14 )
        return "iso8859-14";
    if ( set == QFont::ISO_8859_15 )
        return "iso8859-15";
    if ( set == QFont::KOI8R )
        return "koi8-r";
    if ( set == QFont::Unicode )
        return "iso10646-1";

    return "Unknown";
}

#ifdef _WS_X11_
static QString getCharSetName( const char * registry, const char *encoding )
{
    QString tmp = registry;
    tmp += "-";
    tmp += encoding;
    return tmp.lower();
}
#endif

static QString getCharSetName( QFont::CharSet cs )
{
    const char* name;
    switch( cs ) {
    case QFont::ISO_8859_1:
	name = "Western (ISO 8859-1)";
	break;
    case QFont::ISO_8859_2:
	name = "Eastern European (ISO 8859-2)";
	break;
    case QFont::ISO_8859_3:
	name = "Esperanto and more (ISO 8859-3)";
	break;
    case QFont::ISO_8859_4:
	name = "(ISO 8859-4)";
	break;
    case QFont::ISO_8859_5:
	name = "Cyrillic (ISO 8859-5)";
	break;
    case QFont::ISO_8859_6:
	name = "Arabic (ISO 8859-6)";
	break;
    case QFont::ISO_8859_7:
	name = "Greek (ISO 8859-7)";
	break;
    case QFont::ISO_8859_8:
	name = "Hebrew (ISO 8859-8)";
	break;
    case QFont::ISO_8859_9:
	name = "Turkish(ISO 8859-9)";
	break;
    case QFont::ISO_8859_10:
	name = "Nordic(ISO 8859-10)";
	break;
    case QFont::ISO_8859_11:
	name = "Thai(ISO 8859-11)";
	break;
    case QFont::ISO_8859_12:
	name = "Devanagari(Hindi)(ISO 8859-12)";
	break;
    case QFont::ISO_8859_13:
	name = "Baltic(ISO 8859-13)";
	break;
    case QFont::ISO_8859_14:
	name = "Celtic(ISO 8859-14)";
	break;
    case QFont::ISO_8859_15:
	name = "French/Finnish/Euro(ISO 8859-15)";
	break;
    case QFont::KOI8R:
	name = "Cyrillic (KOI8-R)";
	break;
    case QFont::Unicode:
	name = "Unicode (ISO 10646)";
	break;
    default:
	qWarning( "getCharSetName: Internal error, unknown charset (%i).", cs );
	name = "Unknown";
	break;
    }
    return qApp ? qApp->translate("QFont", name) : QString::fromLatin1(name);
}

/*!
  Returns a string which gives a quite detailed describtion of the \a charSetName
  which can be used e.g. for displaying in a dialog for the user.
 */

QString QFontDatabase::verboseCharSetName( const QString &charSetName )
{
    QFont::CharSet cs = getCharSet( charSetName );
    if ( cs != QFont::AnyCharSet )
	return getCharSetName( cs );
    else
	return charSetName;
}

static QString getCharSetSample( QFont::CharSet cs )
{
    // Note that sample is *NOT* translated.
    QString sample = "AaBb";
    switch( cs ) {
    case QFont::ISO_8859_1:
	sample += QChar(0x00C3);
	sample += QChar(0x00E1);
	sample += "Zz";
	break;
    case QFont::ISO_8859_2:
	sample += QChar(0x0104);
	sample += QChar(0x0105);
	sample += QChar(0x0141);
	sample += QChar(0x0142);
	break;
    case QFont::ISO_8859_3:
	sample += QChar(0x00C0);
	sample += QChar(0x00E1);
	sample += QChar(0x0126);
	sample += QChar(0x0127);
	break;
    case QFont::ISO_8859_4:
	sample += QChar(0x00C3);
	sample += QChar(0x00E1);
	sample += QChar(0x0100);
	sample += QChar(0x0101);
	break;
    case QFont::ISO_8859_5:
	sample += QChar(0x0414);
	sample += QChar(0x0434);
	sample += QChar(0x0436);
	sample += QChar(0x0402);
	break;
    case QFont::ISO_8859_6:
	sample += QChar(0x0628);
	sample += QChar(0x0629);
	sample += QChar(0x062A);
	sample += QChar(0x063A);
	break;
    case QFont::ISO_8859_7:
	sample += QChar(0x0393);
	sample += QChar(0x03B1);
	sample += QChar(0x03A9);
	sample += QChar(0x03C9);
	break;
    case QFont::ISO_8859_8:
	sample += QChar(0x05D0);
	sample += QChar(0x05D1);
	sample += QChar(0x05D2);
	sample += QChar(0x05D3);
	break;
    case QFont::ISO_8859_9:
	sample += QChar(0x00C0);
	sample += QChar(0x00E0);
	sample += QChar(0x011E);
	sample += QChar(0x011F);
	break;
    case QFont::ISO_8859_10:
	sample += "YyZz";
	break;
    case QFont::ISO_8859_11:
	sample += "YyZz";
	break;
    case QFont::ISO_8859_12:
	sample += "YyZz";
	break;
    case QFont::ISO_8859_13:
	sample += "YyZz";
	break;
    case QFont::ISO_8859_14:
	sample += "YyZz";
	break;
    case QFont::ISO_8859_15:
	sample += "Zz";
	sample += QChar(0x00A4);
	sample += QChar(0x20AC);
	break;
    case QFont::KOI8R:
	sample += QChar(0x0414);
	sample += QChar(0x0434);
	sample += QChar(0x0436);
	sample += QChar(0x2560);
	break;
    case QFont::Unicode:
	sample = "Aa";
	sample += QChar(0x01A7); // Latin B
	sample += QChar(0x0414); // Cyrillic
	sample += QChar(0x263A); // Symbol :-)
	sample += QChar(0x6ACB); // G/B/C
	sample += QChar(0x6B7D); // J
	sample += QChar(0xACDF); // Hangul
	break;
    default:
	qWarning( "getCharSetSample: Internal error, unknown charset (%i).", cs );
	sample = "Unknown";
	break;
    }
    return sample;
}

/*!
  Returns some sample characters which are in the charset \a charSetName.
*/

QString QFontDatabase::charSetSample( const QString &charSetName )
{
    QFont::CharSet cs = getCharSet( charSetName );
    if ( cs == QFont::AnyCharSet )
	cs = QFont::ISO_8859_1;
    return getCharSetSample( cs );
}


/*!
  Returns a string with describes the style of the font \a f. This is Something like
  "Bold Italic".
*/

QString QFontDatabase::styleString( const QFont &f )  // ### fttb
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

#ifdef _WS_X11_
static QString getStyleName( char ** tokens, bool *italic, bool *lesserItalic )
{
    char slant0	= tolower( tokens[Slant][0] );
    *italic      = FALSE;
    *lesserItalic = FALSE;

    QString nm = QString::fromLatin1(tokens[Weight_]);

    if ( nm == QString::fromLatin1("medium") )
	nm = QString::fromLatin1("");
    if ( nm.length() > 0 )
	nm.replace( 0, 1, QString(nm[0]).upper());

    if ( slant0 == 'r' ) {
	if ( tokens[Slant][1]) {
	    char slant1 = tolower( tokens[Slant][1] );
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
	if ( tokens[Slant][1] ) {
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

#endif

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
  Returns a list of names of all available font families in the current locale if
  \a onlyForLocale is TRUE, otherwise really all available font families independent
  of the current locale are returned.
*/

QStringList QFontDatabase::families( bool onlyForLocale ) const
{
    return d->families( onlyForLocale );
}

/*!
  Retruns all available styles of the font \a family in the
  char set \a charSet.
*/

QStringList QFontDatabase::styles( const QString &family,
					  const QString &charSet ) const
{
    QString cs( charSet );
    if ( charSet.isEmpty() ) {
	QStringList lst = charSets( family );
	cs = lst.first();
    }
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return emptyList;
    const QtFontCharSet * chSet = fam->charSet( cs );
    return chSet ? chSet->styles() : emptyList;
}

/*!
  Returns whether the font which matches \a family, \a style and \a charSet is
  a scaleable bitmap font. Scaling a bitmap font produces a bad, often hardly
  readable result, as the pixels of the font are scaled. It's better to scale such
  a font only to the available fixed sizes (which you can get with smoothSizes()).

  \sa isScalable(), isSmoothlyScalable()
*/

bool  QFontDatabase::isBitmapScalable( const QString &family,
				       const QString &style,
				       const QString &charSet ) const
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return FALSE;
    if ( style.isEmpty() )
	return fam->isBitmapScalable();
    const QtFontCharSet * chSet = fam->charSet( charSet );
    if ( !chSet )
	return FALSE;
    if ( style.isEmpty() )
	return chSet->isBitmapScalable();
    const QtFontStyle *sty = chSet->style( style );
    return sty && sty->isBitmapScalable();
}

/*!
  Returns whether the font which matches \a family, \a style and \a charSet is
  a smoothly scaleable. If this function returns TRUE, it's save to scale this font
  to every size as the result will always look good.

  \sa isScalable(), isBitmapScalable()
*/

bool  QFontDatabase::isSmoothlyScalable( const QString &family,
					 const QString &style,
					 const QString &charSet ) const
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return FALSE;
    if ( style.isEmpty() )
	return fam->isSmoothlyScalable();
    const QtFontCharSet * chSet = fam->charSet( charSet );
    if ( !chSet )
	return FALSE;
    if ( style.isEmpty() )
	return chSet->isSmoothlyScalable();
    const QtFontStyle *sty = chSet->style( style );
    return sty && sty->isSmoothlyScalable();
}

/*!
  Returns TRUE if the font which matches the settings \a family, \a style and \a charSet
  is scaleable.

  \sa isBitmapScalable(), isSmoothlyScalable()
*/

bool  QFontDatabase::isScalable( const QString &family,
				 const QString &style,
				 const QString &charSet ) const
{
    if ( isSmoothlyScalable( family, style, charSet) )
	return TRUE;
    return isBitmapScalable( family, style, charSet );
}

static const QtFontStyle * getStyle( QFontDatabasePrivate *d,
				     const QString &family,
				     const QString &style,
				     const QString &charSet )
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return 0;
    const QtFontCharSet * chSet = fam->charSet( charSet );
    if ( !chSet )
	return 0;
    return chSet->style( style );
}

static QValueList<int> emptySizeList;

/*!
  Returns a list of all availabe sizes of the font \a family in the
  style \a style and the char set \a charSet.

  \sa smoothSizes(), standardSizes()
*/

QValueList<int> QFontDatabase::pointSizes( const QString &family,
						 const QString &style,
						 const QString &charSet )
{
    QString cs( charSet );
    if ( charSet.isEmpty() ) {
	QStringList lst = charSets( family );
	cs = lst.first();
    }
    QString s( style );
    if ( style.isEmpty() ) {
	QStringList lst = styles( family, cs );
	s = lst.first();
    }
    const QtFontStyle *sty = getStyle( d, family, s, cs );
    return sty ? sty->pointSizes() : emptySizeList;
}


/*!
  Returns a QFont object which matches the settings of \a family, \a style,
  \a pointsize and \a charSet. If no matching font could be created
  an empty QFont object is returned.
*/

QFont QFontDatabase::font( const QString family, const QString &style,
			   int pointSize, const QString charSet )
{
    const QtFontStyle *sty;

    sty =  charSet ? getStyle( d, family, style, charSet ) :
                     getStyle( d, family, style, getCharSet(qApp->font().charSet()) );
    if ( !sty ) {
	qWarning( "QFontDatabase::font: Style not found for\n"
		  "%s, %s, %s", (const char*)family,
		  (const char*)style, (const char*)charSet );
	return QFont();
    }
    return sty->font( family, pointSize );
}

/*!
  Returns the point sizes of a font which matches \a family, \a style and \a charSet,
  that is guaranteed to look good. For non-scalable fonts and smoothly scalable fonts this function
  is equivalent to pointSizes().

  \sa pointSizes(), standardSizes()
*/

QValueList<int> QFontDatabase::smoothSizes( const QString &family,
						  const QString &style,
						  const QString &charSet )
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty ? sty->smoothSizes() : emptySizeList;
}

/*!
  Returns a list of standard fontsizes.

  \sa smoothSizes(), pointSizes()
*/

QValueList<int> QFontDatabase::standardSizes()
{
    return QtFontStyle::standardSizes();
}

/*!
  Returns if the font witch matches the settings \a family, \a style and
  \a charSet is italic or not.

  \sa weight(), bold()
*/

bool QFontDatabase::italic( const QString &family,
			    const QString &style,
			    const QString &charSet ) const
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty && sty->italic();
}

/*!
  Returns if the font witch matches the settings \a family, \a style and
  \a charSet is bold or not.

  \sa italic(), weight()
*/

bool QFontDatabase::bold( const QString &family,
			    const QString &style,
			    const QString &charSet ) const
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty && sty->weight() >= QFont::Bold;
}

/*!
  Returns the weight of the font witch matches the settings \a family, \a style and
  \a charSet.

  \sa italic(), bold()
*/

int QFontDatabase::weight( const QString &family,
			   const QString &style,
			   const QString &charSet ) const
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty ? sty->weight() : -1;
}

/*!
  Returns a list of all char sets in which the font \a family is available in the
  current locale if \a onlyForLocale is TRUE, otherwise all charsets of \a family
  independent of the locale are returned.
*/

QStringList QFontDatabase::charSets( const QString &family,
					   bool onlyForLocale ) const
{
    const QtFontFamily *fam = d->family( family );
    return fam ? fam->charSets( onlyForLocale ) : emptyList;
}

#ifdef _WS_QWS_
void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;

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
	QtFontCharSet * mycharset;
	if(!family) {
	    family=new QtFontFamily(foundry,familyname);
	    foundry->addFamily(family);
	    mycharset=new QtFontCharSet(family,"iso646");
	    family->addCharSet(mycharset);
	}
	mycharset=family->charSetDict.find("iso646");
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
	QtFontStyle * mystyle=mycharset->styleDict.find(style);
	if(!mystyle) {
	    mystyle=new QtFontStyle(mycharset,style);
	    mystyle->ital=qdf->italic;
	    mystyle->lesserItal=false;
	    mystyle->weightString=weightString;
	    mystyle->weightVal=weight;
	    mystyle->weightDirty=false;
	    mycharset->addStyle(mystyle);
	}
	mystyle->setSmoothlyScalable();
    }
}

#endif

#endif // QT_NO_FONTDATABASE
