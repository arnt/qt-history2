/****************************************************************************
** $Id: $
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

#include <qvaluelist.h>
#include <qtl.h>
#include <qapplication.h>

#include <stdlib.h>

struct QtFontStyle
{
    struct Key {
	Key( const QString &styleString );
	Key() : italic( FALSE ), oblique( FALSE ), weight( QFont::Normal ) {}
	Key( const Key &o ) : italic( o.italic ), oblique( o.oblique ), weight( o.weight ) {}
	bool italic : 1;
	bool oblique : 1;
	int  weight : 30;
	bool operator == ( const Key & other ) {
	    return ( italic == other.italic && oblique == other.oblique && weight == other.weight );
	}
    };

    QtFontStyle( const Key &k ) : key( k ), bitmapScalable( FALSE ), smoothScalable( FALSE ), count( 0 ), pixelSizes( 0 ) {}
    Key key;
    bool bitmapScalable : 1;
    bool smoothScalable : 1;
    int count : 30;
    unsigned short *pixelSizes;

    unsigned short pixelSize( unsigned short size, bool = FALSE );
};

QtFontStyle::Key::Key( const QString &styleString )
{
    weight = QFont::Normal;
    italic = oblique = FALSE;
    if ( styleString.contains( "Bold" ) )
	weight = QFont::Bold;
    if ( styleString.contains( "Italic" ) )
	 italic = TRUE;
    else if ( styleString.contains( "Oblique" ) )
	 oblique = TRUE;
}

unsigned short QtFontStyle::pixelSize( unsigned short size, bool add )
{
    if ( smoothScalable )
	return size;

    for ( int i = 0; i < count; i++ ) {
	if ( pixelSizes[i] == size )
	    return size;
    }
    if ( !add )
	return 0;

    if ( !(count % 8) )
	pixelSizes = (unsigned short *) realloc( pixelSizes, (((count+8) >> 3 ) << 3)*sizeof( unsigned short ) );
    pixelSizes[count] = size;
    count++;
    return size;
}

struct QtFontFoundry
{
    QtFontFoundry( const QString &n ) : name( n ), count( 0 ), styles( 0 ) {}
    QString name;

    int count;
    QtFontStyle **styles;
    QtFontStyle *style( const QtFontStyle::Key &,  bool = FALSE );
};

QtFontStyle *QtFontFoundry::style( const QtFontStyle::Key &key, bool create )
{
    for ( int i = 0; i < count; i++ ) {
	if ( styles[i]->key == key )
	    return styles[i];
    }
    if ( !create )
	return 0;

    if ( !(count % 8) )
	styles = (QtFontStyle **) realloc( styles, (((count+8) >> 3 ) << 3)*sizeof( QtFontStyle * ) );

    styles[count] = new QtFontStyle( key );
    return styles[count++];
}


struct QtFontFamily
{
    QtFontFamily(const QString &n ) : fixedPitch( TRUE ), name( n ), count( 0 ), foundries( 0 ) {}
    bool fixedPitch;
    QString name;

    int count;
    QtFontFoundry **foundries;

    QtFontFoundry *foundry( const QString &f, bool = FALSE );
};

QtFontFoundry *QtFontFamily::foundry( const QString &f, bool create )
{
    for ( int i = 0; i < count; i++ ) {
	if ( foundries[i]->name == f )
	    return foundries[i];
    }
    if ( !create )
	return 0;

    if ( !(count % 8) )
	foundries = (QtFontFoundry **) realloc( foundries, (((count+8) >> 3 ) << 3)*sizeof( QtFontFoundry * ) );

    foundries[count] = new QtFontFoundry( f );
    return foundries[count++];
}


struct QtFontScript
{
    QtFontScript() : count( 0 ), families( 0 ) { }
    int count;
    QtFontFamily **families;

    QtFontFamily *family( const QString &f, bool = FALSE );
};


QtFontFamily *QtFontScript::family( const QString &f, bool create )
{
    for ( int i = 0; i < count; i++ ) {
	if ( families[i]->name == f )
	    return families[i];
    }
    if ( !create )
	return 0;

    if ( !(count % 8) )
	families = (QtFontFamily **) realloc( families, (((count+8) >> 3 ) << 3)*sizeof( QtFontFamily * ) );

    families[count] = new QtFontFamily( f );
    return families[count++];
}


class QFontDatabasePrivate {
public:
    QtFontScript scripts[ QFont::NScripts + 1 ];
    QStringList families;
};

static QFontDatabasePrivate *db=0;

#if defined( Q_WS_X11 )
#  include "qfontdatabase_x11.cpp"
#elif defined( Q_WS_MAC )
#  include "qfontdatabase_mac.cpp"
#elif defined( Q_WS_WIN )
#  include "qfontdatabase_win.cpp"
#elif defined( Q_WS_QWS )
#  include "qfontdatabase_qws.cpp"
#endif

static QString styleString( int weight, bool italic, bool oblique )
{
    QString result;
    if ( weight >= QFont::Bold ) {
        if ( italic )
            result = "Bold Italic";
        else if ( oblique )
            result = "Bold Oblique";
	else
	    result = "Bold";
    } else {
        if ( italic )
            result = "Italic";
        else if ( oblique )
            result = "Oblique";
        else
            result = "Normal";
    }
    return result;
}

/*!
    Returns a string that describes the style of the font \a f. For
    example, "Bold Italic", "Bold", "Italic" or "Normal". An empty
    string may be returned.
*/
QString QFontDatabase::styleString( const QFont &f )
{
    // ### fix oblique here
    return ::styleString( f.weight(), f.italic(), FALSE );
}


/*!
    \class QFontDatabase qfontdatabase.h
    \brief The QFontDatabase class provides information about the fonts available in the underlying window system.

    \ingroup environment
    \ingroup graphics

    The most common uses of this class are to query the database for
    the list of font families() and for the pointSizes() and styles()
    that are available for each family. An alternative to pointSizes()
    is smoothSizes() which returns the sizes at which a given family
    and style will look attractive.

    If the font family is available from two or more foundries the
    foundry name is included in the family name, e.g. "Helvetica
    [Adobe]" and "Helvetica [Cronyx]". When you specify a family you
    can either use the old hyphenated Qt 2.x "foundry-family" format,
    e.g. "Cronyx-Helvetica", or the new bracketed Qt 3.x "family
    [foundry]" format e.g. "Helvetica [Cronyx]". If the family has a
    foundry it is always returned, e.g. by families(), using the
    bracketed format.

    The font() function returns a QFont given a family, style and
    point size.

    A family and style combination can be checked to see if it is
    italic() or bold(), and to retrieve its weight(). Similarly we can
    call isBitmapScalable(), isSmoothlyScalable(), isScalable() and
    isFixedPitch().

    A text version of a style is given by styleString().

    The QFontDatabase class also supports some static functions, for
    example, standardSizes(). You can retrieve the Unicode 3.0
    description of a \link QFont::Script script\endlink using
    scriptName(), and a sample of characters in a script with
    scriptSample().

    Example:
\code
#include <qapplication.h>
#include <qfontdatabase.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QFontDatabase fdb;
    QStringList families = fdb.families();
    for ( QStringList::Iterator f = families.begin(); f != families.end(); ++f ) {
	QString family = *f;
	qDebug( family );
	QStringList styles = fdb.styles( family );
	for ( QStringList::Iterator s = styles.begin(); s != styles.end(); ++s ) {
	    QString style = *s;
	    QString dstyle = "\t" + style + " (";
	    QValueList<int> smoothies = fdb.smoothSizes( family, style );
	    for ( QValueList<int>::Iterator points = smoothies.begin();
		  points != smoothies.end(); ++points ) {
		dstyle += QString::number( *points ) + " ";
	    }
	    dstyle = dstyle.left( dstyle.length() - 1 ) + ")";
	    qDebug( dstyle );
	}
    }
    return 0;
}
\endcode
    This example gets the list of font families, then the list of
    styles for each family and the point sizes that are available for
    each family/style combination.
*/
/*!
    \obsolete
    \fn inline QStringList QFontDatabase::families( bool ) const
*/
/*!
    \obsolete
    \fn inline QStringList QFontDatabase::styles( const QString &family,
					  const QString & ) const
*/
/*!
    \obsolete
    \fn inline QValueList<int> QFontDatabase::pointSizes( const QString &family,
						  const QString &style ,
						  const QString & )
*/

/*!
    \obsolete
    \fn inline QValueList<int> QFontDatabase::smoothSizes( const QString &family,
						   const QString &style,
						   const QString & )
*/
/*!
    \obsolete
    \fn inline QFont QFontDatabase::font( const QString &familyName,
				  const QString &style,
				  int pointSize,
				  const QString &)
*/
/*!
    \obsolete
    \fn inline bool QFontDatabase::isBitmapScalable( const QString &family,
					     const QString &style,
					     const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::isSmoothlyScalable( const QString &family,
					       const QString &style,
					       const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::isScalable( const QString &family,
				       const QString &style,
				       const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::isFixedPitch( const QString &family,
					 const QString &style,
					 const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::italic( const QString &family,
				   const QString &style,
				   const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::bold( const QString &family,
				 const QString &style,
				 const QString & ) const
*/

/*!
    \obsolete
    \fn inline int QFontDatabase::weight( const QString &family,
				  const QString &style,
				  const QString & ) const
*/


/*!
    Creates a font database object.
*/
QFontDatabase::QFontDatabase()
{
    createDatabase();

    d = db;
}


static int ucstrcmp( const QString &as, const QString &bs )
{
    const QChar *a = as.unicode();
    const QChar *b = bs.unicode();
    if ( a == b )
	return 0;
    if ( a == 0 )
	return 1;
    if ( b == 0 )
	return -1;
    int l=QMIN(as.length(),bs.length());
    while ( l-- && *a == *b )
	a++,b++;
    if ( l==-1 )
	return ( as.length()-bs.length() );
    return a->unicode() - b->unicode();
}

static int sortFamilies( const void *one, const void *two )
{
    const QtFontFamily **as = (const QtFontFamily **)one;
    const QtFontFamily **bs = (const QtFontFamily **)two;
    return ucstrcmp( (*as)->name, (*bs)->name );
}

/*! Returns a sorted list of the names of the available font families.

    If a family exists in several foundries, the returned name for
    that font is in the form "family [foundry]". Examples: "Times
    [Adobe]", "Times [Cronyx]", "Palatino".
*/
QStringList QFontDatabase::families() const
{
    if ( d->families.isEmpty() ) {
	QtFontFamily **f = 0;
	int count = 0;
	int allocated = 0;
	for ( int s = 0; s < QFont::NScripts+1; s++ ) {
	    QtFontScript &script = d->scripts[s];
	    int ncount = count + script.count;
	    if ( ncount > allocated ) {
		allocated = (((ncount+128) >> 7 ) << 7);
		f = (QtFontFamily **) realloc( f, allocated*sizeof( QtFontFamily * ) );
	    }
	    memcpy( f + count, script.families, script.count*sizeof( QtFontFamily * ) );
	    count = ncount;
	}
	qsort( f, count, sizeof( QtFontFamily * ), sortFamilies );

	QString currentFamily = f[0]->name;
	int start = 0;
	for ( int i = 1; i < count; i++ ) {
	    if ( f[i]->name != currentFamily ) {
		QtFontFamily allFoundries( currentFamily );
		for ( int j = start; j < i; j++ ) {
		    QtFontFamily *family = f[j];
		    for ( int k = 0; k < family->count; k++ ) {
			if ( *(family->foundries[k]->name.unicode()) == QChar ( 0xfffd ) )
			    continue;
			allFoundries.foundry( family->foundries[k]->name,  TRUE );
		    }
		}
		bool printFoundries = ( allFoundries.count > 1 );
		for ( int j = 0; j < allFoundries.count; j++ ) {
		    QString str = currentFamily;
		    if ( printFoundries && !allFoundries.foundries[j]->name.isEmpty() )
			str += " [" + allFoundries.foundries[j]->name + "]";
		    d->families.append( str );
		}
		currentFamily = f[i]->name;
		start = i;
	    }
	}
    }
    return d->families;
}


/*!
    Returns a list of the styles available for the font family \a
    family. Some example styles: "Light", "Light Italic", "Bold",
    "Oblique", "Demi". The list may be empty.
*/
QStringList QFontDatabase::styles( const QString &family ) const
{
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    QtFontFoundry allStyles( foundryName );
    for ( int s = 0; s < QFont::NScripts + 1; s++ ) {
	QtFontScript &script = d->scripts[s];
	for ( int i = 0; i < script.count; i++ ) {
	    QtFontFamily *f = script.families[i];
	    if ( f->name == family ) {
		for ( int j = 0; j < f->count; j++ ) {
		    QtFontFoundry *foundry = f->foundries[j];
		    if ( foundryName.isEmpty() || foundry->name == foundryName ) {
			for ( int k = 0; k < foundry->count; k++ )
			    allStyles.style( foundry->styles[k]->key,  TRUE );
		    }
		}
	    }
	}
    }
    QStringList l;
    for ( int i = 0; i < allStyles.count; i++ )
	l.append( ::styleString( allStyles.styles[i]->key.weight,
				 allStyles.styles[i]->key.italic, allStyles.styles[i]->key.oblique ) );
    return l;
}

/*!
    Returns TRUE if the font that has family \a family and style \a
    style is fixed pitch; otherwise returns FALSE.
*/

bool QFontDatabase::isFixedPitch(const QString &family,
				 const QString &style) const
{
    Q_UNUSED(style);

    bool fixed = TRUE;
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    for ( int s = 0; s < QFont::NScripts + 1; s++ ) {
	QtFontScript &script = d->scripts[s];
	for ( int i = 0; i < script.count; i++ ) {
	    QtFontFamily *f = script.families[i];
	    if ( f->name == family )
		if ( !f->fixedPitch ) {
		    fixed = FALSE;
		    break;
		}
	}
    }
    return fixed;
}

/*!
    Returns TRUE if the font that has family \a family and style \a
    style is a scalable bitmap font; otherwise returns FALSE. Scaling
    a bitmap font usually produces an unattractive hardly readable
    result, because the pixels of the font are scaled. If you need to
    scale a bitmap font it is better to scale it to one of the fixed
    sizes returned by smoothSizes().

    \sa isScalable(), isSmoothlyScalable()
*/
bool  QFontDatabase::isBitmapScalable( const QString &family,
                                       const QString &style) const
{
    bool bitmapScalable = FALSE;
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    QtFontStyle::Key styleKey( style );

    for ( int s = 0; s < QFont::NScripts + 1; s++ ) {
	QtFontScript &script = d->scripts[s];
	for ( int i = 0; i < script.count; i++ ) {
	    QtFontFamily *f = script.families[i];
	    if ( f->name == family )
		for ( int j = 0; j < f->count; j++ ) {
		    QtFontFoundry *foundry = f->foundries[j];
		    if ( foundryName.isEmpty() || foundry->name == foundryName ) {
			for ( int k = 0; k < foundry->count; k++ )
			    if ( foundry->styles[k]->key == styleKey &&
				 foundry->styles[k]->bitmapScalable ) {
				bitmapScalable = TRUE;
				goto end;
			    }
		    }
		}
	}
    }
 end:
    return bitmapScalable;
}


/*!
    Returns TRUE if the font that has family \a family and style \a
    style is smoothly scalable; otherwise returns FALSE. If this
    function returns TRUE, it's safe to scale this font to any size,
    and the result will always look attractive.

    \sa isScalable(), isBitmapScalable()
*/
bool  QFontDatabase::isSmoothlyScalable( const QString &family,
                                         const QString &style) const
{
    bool smoothScalable = FALSE;
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    QtFontStyle::Key styleKey( style );

    for ( int s = 0; s < QFont::NScripts + 1; s++ ) {
	QtFontScript &script = d->scripts[s];
	for ( int i = 0; i < script.count; i++ ) {
	    QtFontFamily *f = script.families[i];
	    if ( f->name == family )
		for ( int j = 0; j < f->count; j++ ) {
		    QtFontFoundry *foundry = f->foundries[j];
		    if ( foundryName.isEmpty() || foundry->name == foundryName ) {
			for ( int k = 0; k < foundry->count; k++ )
			    if ( foundry->styles[k]->key == styleKey &&
				 foundry->styles[k]->smoothScalable ) {
				smoothScalable = TRUE;
				goto end;
			    }
		    }
		}
	}
    }
 end:
    return smoothScalable;
}

/*!
    Returns TRUE if the font that has family \a family and style \a
    style is scalable; otherwise returns FALSE.

    \sa isBitmapScalable(), isSmoothlyScalable()
*/
bool  QFontDatabase::isScalable( const QString &family,
                                 const QString &style) const
{
    if ( isSmoothlyScalable( family, style) )
        return TRUE;

    return isBitmapScalable( family, style);
}


/*!
    Returns a list of the point sizes available for the font that has
    family \a family and style \a style. The list may be empty.

    \sa smoothSizes(), standardSizes()
*/
QValueList<int> QFontDatabase::pointSizes( const QString &family,
					   const QString &style)
{
    bool smoothScalable = FALSE;
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    QtFontStyle::Key styleKey( style );

    QValueList<int> sizes;

    for ( int s = 0; s < QFont::NScripts + 1; s++ ) {
	QtFontScript &script = d->scripts[s];
	for ( int i = 0; i < script.count; i++ ) {
	    QtFontFamily *f = script.families[i];
	    if ( f->name == family ) {
		for ( int j = 0; j < f->count; j++ ) {
		    QtFontFoundry *foundry = f->foundries[j];
		    if ( foundryName.isEmpty() || foundry->name == foundryName ) {
			for ( int k = 0; k < foundry->count; k++ ) {
			    QtFontStyle *style = foundry->styles[k];
			    if ( style->key == styleKey ) {
				if ( style->smoothScalable ) {
				    smoothScalable = TRUE;
				    goto end;
				}
				for ( int l = 0; l < style->count; l++ ) {
				    if ( !sizes.contains( style->pixelSizes[l] ) )
					sizes.append( style->pixelSizes[l] );
				}
			    }
			}
		    }
		}
	    }
	}
    }
 end:
    if ( smoothScalable )
	return QValueList<int>();

    qHeapSort( sizes );

    // ### convert to point sizes

    return sizes;
}

#if 0
/*!
    Returns a QFont object that has family \a family, style \a style
    and point size \a pointSize. If no matching font could be created,
    a QFont object that uses the application's default font is
    returned.
*/
QFont QFontDatabase::font( const QString &family, const QString &style,
                           int pointSize)
{
    const QtFontStyle *sty = 0;

    sty = getStyle( d, family, style);
    if ( !sty ) {
	const char *f = family, *s = style;
	if (! f)
	    f = "none";
	if (! s)
	    s = "none";

#ifndef QT_NO_DEBUG
        qWarning( "QFontDatabase::font: Style not found for %s, %s, %d",
		  f, s, pointSize);
#endif
        return QFont();
    }

    return sty->font( family, pointSize );
}


/*!
    Returns the point sizes of a font that has family \a family and
    style \a style that will look attractive. The list may be empty.
    For non-scalable fonts and smoothly scalable fonts, this function
    is equivalent to pointSizes().

  \sa pointSizes(), standardSizes()
*/
QValueList<int> QFontDatabase::smoothSizes( const QString &family,
					    const QString &style)
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty ? sty->smoothSizes() : QValueList<int>();
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
    Returns TRUE if the font that has family \a family and style \a
    style is italic; otherwise returns FALSE.

    \sa weight(), bold()
*/
bool QFontDatabase::italic( const QString &family,
                            const QString &style) const
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty && sty->italic();
}


/*!
    Returns TRUE if the font that has family \a family and style \a
    style is bold; otherwise returns FALSE.

    \sa italic(), weight()
*/
bool QFontDatabase::bold( const QString &family,
			  const QString &style) const
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty && sty->weight() >= QFont::Bold;
}


/*!
    Returns the weight of the font that has family \a family and style
    \a style. If there is no such family and style combination,
    returns -1.

    \sa italic(), bold()
*/
int QFontDatabase::weight( const QString &family,
                           const QString &style) const
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty ? sty->weight() : -1;
}

#endif

/*!
    Returns a string that gives a default description of the \a script
    (e.g. for displaying to the user in a dialog).  The name matches
    the name of the script as defined by the Unicode 3.0 standard.

    \sa QFont::Script
*/
QString QFontDatabase::scriptName(QFont::Script script)
{
    const char *name = 0;

    switch (script) {
    case QFont::Latin:
	name = QT_TRANSLATE_NOOP("QFont",  "Latin");
	break;
    case QFont::Greek:
	name = QT_TRANSLATE_NOOP("QFont",  "Greek" );
	break;
    case QFont::Cyrillic:
	name = QT_TRANSLATE_NOOP("QFont",  "Cyrillic" );
	break;
    case QFont::Armenian:
	name = QT_TRANSLATE_NOOP("QFont",  "Armenian" );
	break;
    case QFont::Georgian:
	name = QT_TRANSLATE_NOOP("QFont",  "Georgian" );
	break;
    case QFont::Runic:
	name = QT_TRANSLATE_NOOP("QFont",  "Runic" );
	break;
    case QFont::Ogham:
	name = QT_TRANSLATE_NOOP("QFont",  "Ogham" );
	break;
    case QFont::Hebrew:
	name = QT_TRANSLATE_NOOP("QFont",  "Hebrew" );
	break;
    case QFont::Arabic:
	name = QT_TRANSLATE_NOOP("QFont",  "Arabic" );
	break;
    case QFont::Syriac:
	name = QT_TRANSLATE_NOOP("QFont",  "Syriac" );
	break;
    case QFont::Thaana:
	name = QT_TRANSLATE_NOOP("QFont",  "Thaana" );
	break;
    case QFont::Devanagari:
	name = QT_TRANSLATE_NOOP("QFont",  "Devanagari" );
	break;
    case QFont::Bengali:
	name = QT_TRANSLATE_NOOP("QFont",  "Bengali" );
	break;
    case QFont::Gurmukhi:
	name = QT_TRANSLATE_NOOP("QFont",  "Gurmukhi" );
	break;
    case QFont::Gujarati:
	name = QT_TRANSLATE_NOOP("QFont",  "Gujarati" );
	break;
    case QFont::Oriya:
	name = QT_TRANSLATE_NOOP("QFont",  "Oriya" );
	break;
    case QFont::Tamil:
	name = QT_TRANSLATE_NOOP("QFont",  "Tamil" );
	break;
    case QFont::Telugu:
	name = QT_TRANSLATE_NOOP("QFont",  "Telugu" );
	break;
    case QFont::Kannada:
	name = QT_TRANSLATE_NOOP("QFont",  "Kannada" );
	break;
    case QFont::Malayalam:
	name = QT_TRANSLATE_NOOP("QFont",  "Malayalam" );
	break;
    case QFont::Sinhala:
	name = QT_TRANSLATE_NOOP("QFont",  "Sinhala" );
	break;
    case QFont::Thai:
	name = QT_TRANSLATE_NOOP("QFont",  "Thai" );
	break;
    case QFont::Lao:
	name = QT_TRANSLATE_NOOP("QFont",  "Lao" );
	break;
    case QFont::Tibetan:
	name = QT_TRANSLATE_NOOP("QFont",  "Tibetan" );
	break;
    case QFont::Myanmar:
	name = QT_TRANSLATE_NOOP("QFont",  "Myanmar" );
	break;
    case QFont::Khmer:
	name = QT_TRANSLATE_NOOP("QFont",  "Khmer" );
	break;
    case QFont::Han:
	name = QT_TRANSLATE_NOOP("QFont",  "Han" );
	break;
    case QFont::Hiragana:
	name = QT_TRANSLATE_NOOP("QFont",  "Hiragana" );
	break;
    case QFont::Katakana:
	name = QT_TRANSLATE_NOOP("QFont",  "Katakana" );
	break;
    case QFont::Hangul:
	name = QT_TRANSLATE_NOOP("QFont",  "Hangul" );
	break;
    case QFont::Bopomofo:
	name = QT_TRANSLATE_NOOP("QFont",  "Bopomofo" );
	break;
    case QFont::Yi:
	name = QT_TRANSLATE_NOOP("QFont",  "Yi" );
	break;
    case QFont::Ethiopic:
	name = QT_TRANSLATE_NOOP("QFont",  "Ethiopic" );
	break;
    case QFont::Cherokee:
	name = QT_TRANSLATE_NOOP("QFont",  "Cherokee" );
	break;
    case QFont::CanadianAboriginal:
	name = QT_TRANSLATE_NOOP("QFont",  "Canadian Aboriginal" );
	break;
    case QFont::Mongolian:
	name = QT_TRANSLATE_NOOP("QFont",  "Mongolian" );
	break;

    case QFont::CurrencySymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Currency Symbols" );
	break;

    case QFont::LetterlikeSymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Letterlike Symbols" );
	break;

    case QFont::NumberForms:
	name = QT_TRANSLATE_NOOP("QFont",  "Number Forms" );
	break;

    case QFont::MathematicalOperators:
	name = QT_TRANSLATE_NOOP("QFont",  "Mathematical Operators" );
	break;

    case QFont::TechnicalSymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Technical Symbols" );
	break;

    case QFont::GeometricSymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Geometric Symbols" );
	break;

    case QFont::MiscellaneousSymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Miscellaneous Symbols" );
	break;

    case QFont::EnclosedAndSquare:
	name = QT_TRANSLATE_NOOP("QFont",  "Enclosed and Square" );
	break;

    case QFont::Braille:
	name = QT_TRANSLATE_NOOP("QFont",  "Braille" );
	break;

    case QFont::Unicode:
	name = QT_TRANSLATE_NOOP("QFont",  "Unicode" );
	break;

    default:
	name = "";
	break;
    }

    return qApp ? qApp->translate("QFont", name) : QString::fromLatin1(name);
}


/*!
    Returns a string with sample characters from \a script.

    \sa QFont::Script
*/
QString QFontDatabase::scriptSample(QFont::Script script)
{
    QString sample = "AaBb";

    switch (script) {
    case QFont::Latin:
	// This is cheating... we only show latin-1 characters so that we don't
	// end up loading lots of fonts - at least on X11...
	sample += QChar(0x00C3);
	sample += QChar(0x00E1);
	sample += "Zz";
	break;
    case QFont::Greek:
	sample += QChar(0x0393);
	sample += QChar(0x03B1);
	sample += QChar(0x03A9);
	sample += QChar(0x03C9);
	break;
    case QFont::Cyrillic:
	sample += QChar(0x0414);
	sample += QChar(0x0434);
	sample += QChar(0x0436);
	sample += QChar(0x0402);
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



    case QFont::Han:
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


    case QFont::CurrencySymbols:
    case QFont::LetterlikeSymbols:
    case QFont::NumberForms:
    case QFont::MathematicalOperators:
    case QFont::TechnicalSymbols:
    case QFont::GeometricSymbols:
    case QFont::MiscellaneousSymbols:
    case QFont::EnclosedAndSquare:
    case QFont::Braille:
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




/*!
  \internal

  This makes sense of the font family name:

  1) if the family name contains a '-' (ie. "Adobe-Courier"), then we
  split at the '-', and use the string as the foundry, and the string to
  the right as the family

  2) if the family name contains a '[' and a ']', then we take the text
  between the square brackets as the foundry, and the text before the
  square brackets as the family (ie. "Arial [Monotype]")
*/
void QFontDatabase::parseFontName(const QString &name, QString &foundry, QString &family)
{
    if ( name.contains('-') ) {
	int i = name.find('-');
	foundry = name.left( i ).lower();
	family = name.right( name.length() - i - 1 ).lower();
    } else if ( name.contains('[') && name.contains(']')) {
	int i = name.find('[');
	int li = name.findRev(']');

	if (i < li) {
	    foundry = name.mid(i + 1, li - i - 1).lower();
	    if (name[i - 1] == ' ')
		i--;
	    family = name.left(i).lower();
	}
    } else {
	foundry = QString::null;
	family = name.lower();
    }
}

#endif // QT_NO_FONTDATABASE
