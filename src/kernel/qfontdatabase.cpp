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

#include "qmap.h"
#include "qdict.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qapplication.h"
#include "qpainter.h"

#include "private/qapplication_p.h"
#include "private/qfontdata_p.h"

#include <stdlib.h>
#include <ctype.h>

class QtFontFamily;
class QtFontFoundry;

#ifdef Q_WS_WIN
static void newWinFont( void * p );
static void add_style( QtFontFamily *family, const QString& styleName,
		       bool italic, bool lesserItalic, int weight );
#elif defined( Q_WS_MAC )
static void add_style( QtFontFamily *family, const QString& styleName,
		       bool italic, bool lesserItalic, int weight );
#endif

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

#ifdef Q_WS_QWS
    friend void QFontDatabase::qwsAddDiskFont( QDiskFont *qdf );
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

#ifdef Q_WS_QWS
    friend void QFontDatabase::qwsAddDiskFont( QDiskFont *qdf );
#endif


};


class QtFontFoundry
{
public:
    QtFontFoundry( const QString &n )
    {
	nm = n;
	namesDirty = TRUE;
    }

    QString name() const { return nm; }

    const QStringList &families() const;
    const QtFontFamily *family( const QString &name ) const;

private:
    QString nm;

    QStringList familyNames;
    QDict<QtFontFamily> familyDict;

    bool namesDirty;

    void addFamily( QtFontFamily *f ) { familyDict.insert( f->name(), f ); }

    friend void QFontDatabase::createDatabase();

#ifdef Q_WS_WIN
    friend void newWinFont( void * p );
#endif

#ifdef Q_WS_QWS
    friend void QFontDatabase::qwsAddDiskFont( QDiskFont *qdf );
#endif
};

class QFontDatabasePrivate {
public:
    QFontDatabasePrivate()
    {
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

    void addFoundry( QtFontFoundry *f ) { foundryDict.insert( f->name(), f ); }

    friend void QFontDatabase::createDatabase();

#ifdef Q_WS_WIN
    friend void newWinFont( void * p );
#endif

#ifdef Q_WS_QWS
    friend void QFontDatabase::qwsAddDiskFont( QDiskFont *qdf );
#endif
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
        that->familyNames.clear();
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


const QStringList &QFontDatabasePrivate::families() const
{
    QFontDatabasePrivate *that = (QFontDatabasePrivate*) this; // Mutable

    if ( familiesDirty ) {
        QDict<QtFontFoundry> firstFoundryForFamily;
        QDict<int> doubles;
        QtFontFoundry *foundry;
        QDictIterator<QtFontFoundry> iter( foundryDict );
	QString s, f;
	int index;

        for( ; (foundry = iter.current()) ; ++iter ) {
            QStringList l = foundry->families();

            for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
		s = *it;
		s[0] = s[0].upper();
		index = -1;
		while ((index = s.find(' ', index + 1)) != -1 &&
		       uint(index + 1) < s.length() - 1)
		    s[index + 1] = s[index + 1].upper();

                if ( !firstFoundryForFamily.find( s ) ) {
		    that->familyNames.append( s );
		    firstFoundryForFamily.insert( s, foundry );
                } else {
		    if ( !doubles.find(s) ) { // 2nd foundry for family?
                        doubles.insert( s, (int*)1 );
                        QtFontFoundry *tmp = firstFoundryForFamily.find(s);
			that->familyNames.remove( s );

                        if ( tmp )
                            f = tmp->name();
                        else
                            qWarning( "QFontDatabasePrivate::families:"
				      "Internal error, Cannot find first foundry");

			f[0] = f[0].upper();
			index = -1;
			while ((index = f.find(' ', index + 1)) != -1 &&
			       uint(index + 1) < f.length() - 1)
			    f[index + 1] =
				f[index + 1].upper();

			that->familyNames.append( s + " [" + f + "]" );
                    }


		    f = foundry->name();
		    f[0] = f[0].upper();
		    index = -1;
		    while ((index = f.find(' ', index + 1)) != -1 &&
			   uint(index + 1) < f.length() - 1)
			f[index + 1] =
			    f[index + 1].upper();

                    s += " [" + f + "]";
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

    QFontDatabasePrivate *that = (QFontDatabasePrivate*) this; // Mutable
    const QtFontFamily *result = bestFamilyDict.find(name);

    if ( !result ) {
	QString familyName, foundryName;
        const QtFontFoundry *fndry;
        const QtFontFamily *fam;

	QFontDatabase::parseFontName(name, foundryName, familyName);
	if (! foundryName.isNull()) {
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
            fam = fndry->family( name.lower() );

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


/*!
  Returns a string that describes the style of the font \a f. For
  example, "Bold Italic", "Bold", "Italic" or "Normal". An empty string
  may be returned.
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


static QStringList emptyList;


/*! \class QFontDatabase qfontdatabase.h
    \ingroup graphics

  \brief The QFontDatabase class provides information about the fonts available in the underlying window system.
  \ingroup environment

    The most common uses of this class are to query the database for the
    list of font families() and the pointSizes() and styles() that are
    available for each family. An alternative to pointSizes() is
    smoothSizes() which returns the sizes at which a given family and
    style will look attractive.

    If the font family is available from two or more foundries the
    foundry name is included in the family name, e.g. "Helvetica
    [Adobe]" and "Helvetica [Cronyx]". When you specify a family you can
    either use the hyphenated "foundry-family" format, e.g.
    "Cronyx-Helvetica", or the bracketed format, e.g. "Helvetica
    [Cronyx]". If the family has a foundry it is always returned, e.g.
    by families(), using the bracketed format.

    The font() function returns a QFont given a family, style and point
    size.

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
	    for ( QValueList<int>::Iterator points = smoothies.begin(); points != smoothies.end(); ++points ) {
		dstyle += QString::number( *points ) + " ";
	    }
	    dstyle = dstyle.left( dstyle.length() - 1 ) + ")";
	    qDebug( dstyle );
	}
    }
    return 0;
}
\endcode
    This example gets the list of font families, then the list of styles
    for each family and the point sizes that are available for each
    family/style combination.
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


/*!
    Returns a list of the names of the available font families.

  If a family exists in several foundries, the returned name for that font
  is in the form "family [foundry]". Examples: "Times [Adobe]", "Times
  [Cronyx]", "Palatino".
*/
QStringList QFontDatabase::families() const
{
    return d->families();
}


/*!
  Returns a list of the styles available for the font family, \a family.
  Some example styles: "Light", "Light Italic", "Bold", "Oblique",
  "Demi". The list may be empty.
*/
QStringList QFontDatabase::styles( const QString &family) const
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
        return emptyList;

    return fam->styles();
}


/*!
  Returns TRUE if the font that has family \a family and style \a style
  is fixed pitch; otherwise returns FALSE.
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
  Returns TRUE if the font that has family \a family and style \a style
  is a scalable bitmap font; otherwise returns FALSE. Scaling a bitmap
  font usually produces an unattractive hardly readable result, because
  the pixels of the font are scaled. If you need to scale a bitmap font
  it is better to scale it to one of the fixed sizes returned by
  smoothSizes().

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
  Returns TRUE if the font that has family \a family and style \a style
  is smoothly scalable; otherwise returns FALSE. If this function
  returns TRUE, it's safe to scale this font to any size, and the result
  will always look attractive.

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
  Returns TRUE if the font that has family \a family and style \a style
  is scalable; otherwise returns FALSE.

  \sa isBitmapScalable(), isSmoothlyScalable()
*/
bool  QFontDatabase::isScalable( const QString &family,
                                 const QString &style) const
{
    if ( isSmoothlyScalable( family, style) )
        return TRUE;

    return isBitmapScalable( family, style);
}


static const QtFontStyle * getStyle( QFontDatabasePrivate *d,
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
  Returns a list of the point sizes available for the font that has
  family \a family and style \a style. The list may be empty.

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


/*! Returns a QFont object that has family \a family, style \a style
  and point size \a pointSize. If no matching font could be created, a
  QFont object that uses the application's default font is returned.
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

        qWarning( "QFontDatabase::font: Style not found for %s, %s, %d",
		  f, s, pointSize);
        return QFont();
    }

    return sty->font( family, pointSize );
}


/*!
  Returns the point sizes of a font that has family \a family and style
  \a style that will look attractive. The list may be empty. For
  non-scalable fonts and smoothly scalable fonts, this function is
  equivalent to pointSizes().

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
  Returns TRUE if the font that has family \a family and style \a style
  is italic; otherwise returns FALSE.

  \sa weight(), bold()
*/
bool QFontDatabase::italic( const QString &family,
                            const QString &style) const
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty && sty->italic();
}


/*!
  Returns TRUE if the font that has family \a family and style \a style
  is bold; otherwise returns FALSE.

  \sa italic(), weight()
*/
bool QFontDatabase::bold( const QString &family,
			  const QString &style) const
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty && sty->weight() >= QFont::Bold;
}


/*!
  Returns the weight of the font that has family \a family and
  style \a style. If there is no such family and style combination,
  returns -1.

  \sa italic(), bold()
*/
int QFontDatabase::weight( const QString &family,
                           const QString &style) const
{
    const QtFontStyle *sty = getStyle( d, family, style );
    return sty ? sty->weight() : -1;
}


/*!
  Returns a string that gives a default description of the \a script
  (e.g. for displaying to the user in a dialog).  The name matches the
  name of the script as indicated by the Unicode 3.0 standard.

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
