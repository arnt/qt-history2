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


private:
    const QValueList<int> &storedSizes() const;

    void addPointSize( int size );
    void setSmoothlyScalable();
    void setBitmapScalable();


    QtFontFamily *p;
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
	scalableDirty    = TRUE;
    }

    const QString &name() const { return nm; }

    const QtFontFoundry *parent() { return p; }

    const QStringList &styles() const;
    const QtFontStyle *style(const QString &) const;

    bool isBitmapScalable() const;
    bool isSmoothlyScalable() const;


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


class QFontDatabasePrivate {
public:
    QFontDatabasePrivate(){
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
    return QFont( family, pointSize, weight(), italic() );
}


QFont QtFontStyle::font( int pointSize ) const
{
    return font( parent()->name(), pointSize );
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







/*
  const QStringList &QtFontCharSet::styles() const
{

}

const QtFontStyle *QtFontCharSet::style( const QString &s ) const
{
    return styleDict.find( s );
}

bool QtFontCharSet::isLocaleCharSet() const
{
    // return charSet() == QFont::charSetForLocale();
}

bool QtFontCharSet::isUnicode() const
{
    // return charSet() == QFont::Unicode;
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
*/

/*!
  Traverses all styles. If all of them are scalable, scalable is set to
  TRUE, if all of them are smoothly scalable smoothlyScalable is set to
  TRUE.

  The styles that most closely resemble a normal, italic, bold and bold
  italic font are found.
*/
/*
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
*/




/*
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
*/



/*
  static
  QString localCharSet()
  {
  return "iso10646-1";
  }
*/


/*
  const QtFontCharSet *QtFontFamily::charSet( const QString &n ) const
  {
  if ( n.isEmpty() )
  return charSetDict.find ( localCharSet() );
  else
  return charSetDict.find ( n );
  }
*/


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


/*
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
*/


/*
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
*/


void QtFontFamily::refresh() const
{
    if ( !scalableDirty )
        return;

    QtFontFamily *that = (QtFontFamily*) this;   // Mutable function
    that->scalableDirty    = FALSE;
    that->smoothlyScalable = FALSE;
    that->bitmapScalable   = FALSE;

    bool isSmooth = TRUE;
    QtFontStyle *tmp;
    QDictIterator<QtFontStyle> iter(styleDict);

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


const QStringList &QFontDatabasePrivate::families() const
{
    QFontDatabasePrivate *that = (QFontDatabasePrivate*) this; // Mutable

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

    QFontDatabasePrivate *that = (QFontDatabasePrivate*) this; // Mutable
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


#ifdef Q_WS_X11

void QFontDatabase::createDatabase()
{
    if ( db ) return;

    db = new QFontDatabasePrivate;

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
    //QFontDatabasePrivate* d = (QFontDatabasePrivate*)p;

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
        //qWarning( "New style[%s] for [%s][%s][%s]",
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
    db = new QFontDatabasePrivate;
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
  db = new QFontDatabasePrivate;

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
      int len = n[0];
      memcpy(n, n+1, len);
      n[len] = '\0';
      fam_name = (char *)n;

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

	QString style_nam;
	bool italic;
	int weight;

	while(!FMGetNextFontFamilyInstance(&fit, &font, &font_style, &font_size)) {

	  //THESE are all that's left
	  style_nam = "blah"; //FIXME
	  italic = TRUE; //FIXME
	  weight = 0; //FIXME

	  if ( style_nam.isEmpty() ) {
	    add_style( family, style_nam, FALSE, FALSE, weight );
	    add_style( family, style_nam, FALSE, TRUE, weight );

	    if ( weight < QFont::DemiBold ) {
	      add_style( family, style_nam, FALSE, FALSE, QFont::Bold );
	      add_style( family, style_nam, FALSE, TRUE, QFont::Bold );
	    }
	  } else {
	    if ( italic ) {
	      add_style( family, style_nam, italic, FALSE, weight );
	    } else {
	      add_style( family, style_nam, italic, FALSE, weight );
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
	FMDisposeFontFamilyInstanceIterator(&fit);
      }
    }
    FMDisposeFontFamilyIterator(&it);
  }
}

#endif // Q_WS_MAC


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
    } else if ( strcmp( registry, "koi8" ) == 0 &&
		qstrcmp( encoding, "u" ) == 0) {
	return QFont::KOI8U;
    } else if ( qstrcmp( registry, "tscii" ) == 0 &&
		qstrcmp( encoding, "0" ) == 0 ) {
	return QFont::TSCII;
    } else if ( qstrcmp( registry, "tis620" ) == 0 &&
		qstrcmp( encoding, "0" ) == 0 ) {
	return QFont::TIS620;
    } else if ( qstrcmp( registry, "iso10646" ) == 0 ) {
	return QFont::Unicode;
    }
    return QFont::AnyCharSet;
}

#endif


/*
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
    if ( name == "koi8-u" )
        return QFont::KOI8U;
    if ( name == "tis620-0" )
        return QFont::TIS620;
    if ( name == "tscii-0" )
	return QFont::TSCII;
    if ( name == "iso10646-1" )
        return QFont::Unicode;
    return QFont::AnyCharSet;
}

static const QString getCharSet( QFont::CharSet set)
{
    switch( set ) {
    case QFont::ISO_8859_1:
        return "iso8859-1";
    case QFont::ISO_8859_2:
        return "iso8859-2";
    case QFont::ISO_8859_3:
        return "iso8859-3";
    case QFont::ISO_8859_4:
        return "iso8859-4";
    case QFont::ISO_8859_5:
        return "iso8859-5";
    case QFont::ISO_8859_6:
        return "iso8859-6";
    case QFont::ISO_8859_7:
        return "iso8859-7";
    case QFont::ISO_8859_8:
        return "iso8859-8";
    case QFont::ISO_8859_9:
        return "iso8859-9";
    case QFont::ISO_8859_10:
        return "iso8859-10";
    case QFont::ISO_8859_11:
        return "iso8859-11";
    case QFont::ISO_8859_12:
        return "iso8859-12";
    case QFont::ISO_8859_13:
        return "iso8859-13";
    case QFont::ISO_8859_14:
        return "iso8859-14";
    case QFont::ISO_8859_15:
        return "iso8859-15";
    case QFont::KOI8R:
        return "koi8-r";
    case QFont::KOI8U:
        return "koi8-u";
    case QFont::Unicode:
        return "iso10646-1";
    default:
        break;
    }
    return "Unknown";
}


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
    case QFont::KOI8U:
        name = "Ukrainian (KOI8-U)";
        break;
    case QFont::TSCII:
	name = "Tamil (TSCII)";
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
*/


/*!
  Returns a string that gives a quite detailed description of the \a
  charSetName (e.g. for displaying in a dialog for the user).
 */
QString QFontDatabase::verboseCharSetName( const QString &/*charSetName*/ )
{
    /*
      Font::CharSet cs = getCharSet( charSetName );

      if ( cs != QFont::AnyCharSet )
      return getCharSetName( cs );

      return charSetName;
    */

    return QString::fromLatin1("Not implemented");
}


/*
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
    case QFont::KOI8U:
        sample += QChar(0x0404);
        sample += QChar(0x0454);
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
*/


/*!
  Returns some sample characters that are in the charset \a charSetName.
*/
QString QFontDatabase::charSetSample( const QString &/*charSetName*/ )
{
    /*
	  QFont::CharSet cs = getCharSet( charSetName );
	  if ( cs == QFont::AnyCharSet )
	  cs = QFont::ISO_8859_1;
	  return getCharSetSample( cs );
    */

    return QString::fromLatin1("Not implemented");
}


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


#ifdef Q_WS_QWS

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
