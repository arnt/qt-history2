/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input method character
**
** Created : 20000414
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qfile.h>
#include <qtl.h>
#include <math.h>
#include <values.h>
#include "qimpenchar.h"

#define QIMPEN_MATCH_THRESHOLD	    120000

const QIMPenSpecialKeys qimpen_specialKeys[] = {
    { Qt::Key_Escape,       "[Esc]" },
    { Qt::Key_Tab,          "[Tab]" },
    { Qt::Key_Backspace,    "[BackSpace]" },
    { Qt::Key_Return,       "[Return]" },
    { Qt::Key_unknown,      0 } };


/*!
  \class QIMPenChar qimpenchar.h

  Handles a single character.  Can calculate closeness of match to
  another character.
*/

QIMPenChar::QIMPenChar()
{
    flags = 0;
    strokes.setAutoDelete( TRUE );
}

QIMPenChar::QIMPenChar( const QIMPenChar &chr )
{
    strokes.setAutoDelete( TRUE );
    ch = chr.ch;
    flags = chr.flags;
    QListIterator<QIMPenStroke> it( chr.strokes );
    while ( it.current() ) {
        strokes.append( new QIMPenStroke( *it.current() ) );
        ++it;
    }
}

QIMPenChar &QIMPenChar::operator=( const QIMPenChar &chr )
{
    ch = chr.ch;
    flags = chr.flags;
    QListIterator<QIMPenStroke> it( chr.strokes );
    while ( it.current() ) {
        strokes.append( new QIMPenStroke( *it.current() ) );
        ++it;
    }

    return *this;
}

void QIMPenChar::clear()
{
    ch = 0;
    flags = 0;
    strokes.clear();
}

/*!
  Begin inputting a new character.
*/
void QIMPenChar::addStroke( QIMPenStroke *st )
{
    QIMPenStroke *stroke = new QIMPenStroke( *st );
    strokes.append( stroke );
}

/*!
  Return an indicator of the closeness of this character to \a pen.
  Lower value is better.
*/
int QIMPenChar::match( QIMPenChar *pen )
{
    if ( strokes.count() > pen->strokes.count() )
        return MAXINT;

    int err = 0;
    int maxErr = 0;
    int diff = 0;
    QListIterator<QIMPenStroke> it1( strokes );
    QListIterator<QIMPenStroke> it2( pen->strokes );
    while ( it1.current() ) {
        err = it1.current()->match( it2.current() );
        if ( err > maxErr )
            maxErr = err;
        QPoint p1 = it1.current()->startingPoint() - startingPoint();
        QPoint p2 = it2.current()->startingPoint() - pen->startingPoint();
        int xdiff = QABS( p1.x() - p2.x() );
        int ydiff = QABS( p1.y() - p2.y() );
        diff += xdiff*xdiff + ydiff*ydiff;
        ++it1;
        ++it2;
    }
    
    if ( diff > 500 ) // not a chance
        return MAXINT;

    maxErr += diff * diff * 8; // magic weighting :)

//    qDebug( "maxErr %d, diff %d, (%d)", maxErr, diff, strokes.count() );
    return maxErr;
}

/*!
  Return the bounding rect of this character.  It may have sides with
  negative coords since its origin is where the user started drawing
  the character.
*/
QRect QIMPenChar::boundingRect()
{
    QRect br;
    QIMPenStroke *st = strokes.first();
    while ( st ) {
        br |= st->boundingRect();
        st = strokes.next();
    }

    return br;
}


/*!
  Write the character's data to the stream.
*/
QDataStream &operator<< (QDataStream &s, const QIMPenChar &ws)
{
    s << ws.ch;
    s << ws.flags;
    s << ws.strokes.count();
    QListIterator<QIMPenStroke> it( ws.strokes );
    while ( it.current() ) {
        s << *it.current();
        ++it;
    }

    return s;
}

/*!
  Read the character's data from the stream.
*/
QDataStream &operator>> (QDataStream &s, QIMPenChar &ws)
{
    s >> ws.ch;
    s >> ws.flags;
    unsigned size;
    s >> size;
    for ( unsigned i = 0; i < size; i++ ) {
        QIMPenStroke *st = new QIMPenStroke();
        s >> *st;
        ws.strokes.append( st );
    }

    return s;
}

//===========================================================================

int QIMPenCharMatchList::compareItems( QCollection::Item item1,
                                       QCollection::Item item2 )
{
    QIMPenCharMatch *m1 = (QIMPenCharMatch *)item1;
    QIMPenCharMatch *m2 = (QIMPenCharMatch *)item2;

    return m1->error - m2->error;
}

//===========================================================================

/*!
  \class QIMPenCharSet qimpenchar.h

  Maintains a set of related characters.
*/

QIMPenCharSet::QIMPenCharSet()
{
    chars.setAutoDelete( TRUE );
    matches.setAutoDelete( TRUE );
    desc = "Unnamed";
    csTitle = "abc";
    maxStrokes = 0;
}

/*!
  Construct and load a characters set from file \a fn.
*/
QIMPenCharSet::QIMPenCharSet( const QString &fn )
{
    chars.setAutoDelete( TRUE );
    matches.setAutoDelete( TRUE );
    desc = "Unnamed";
    csTitle = "abc";
    maxStrokes = 0;
    load( fn );
}

/*!
  Load a character set from file \a fn.
*/
void QIMPenCharSet::load( const QString &fn )
{
    filename = fn;
    chars.clear();

    QFile file( filename );
    if ( file.open( IO_ReadOnly ) ) {
        QDataStream ds( &file );
	QString version;
	ds >> version;
        ds >> csTitle;
        ds >> desc;
        while ( !ds.atEnd() ) {
            QIMPenChar *pc = new QIMPenChar;
            ds >> *pc;
            addChar( pc );
        }
    }
}

/*!
  Save this character set.
*/
void QIMPenCharSet::save()
{
    if ( chars.count() == 0 )
        return;

    QFile file( filename );
    if ( file.open( IO_WriteOnly ) ) {
        QDataStream ds( &file );
	ds << QString( "QPT 1.0" );
        ds << csTitle;
        ds << desc;
        QListIterator<QIMPenChar> ci( chars );
        for ( ; ci.current(); ++ci ) {
            ds << *ci.current();
        }
    }
}

/*!
  Find the best matches for \a ch in this character set.
*/
const QIMPenCharMatchList &QIMPenCharSet::match( QIMPenChar *ch )
{
    QArray<int> errs( chars.count() );
    int minErr = MAXINT;

    QListIterator<QIMPenChar> ci( chars );
    for ( unsigned i = 0; ci.current(); ++ci, i++ ) {
        if ( ci.current()->testFlag( QIMPenChar::Deleted ) ) {
            errs[i] = MAXINT;
            continue;
        }
        errs[i] = ch->match( ci.current() );
        if ( errs[i] < minErr )
            minErr = errs[i];
    }

    matches.clear();

    if ( minErr > QIMPEN_MATCH_THRESHOLD ) {
        return matches;
    }

    for ( unsigned i = 0; i < errs.count(); i++ ) {
        if ( errs[i] < QIMPEN_MATCH_THRESHOLD ) {
            if (chars.at(i)->penStrokes().count() != ch->penStrokes().count())
                errs[i] = QIMPEN_MATCH_THRESHOLD;
            QIMPenCharMatch *m = new QIMPenCharMatch;
            m->error = errs[i];
            m->penChar = chars.at(i);
            matches.inSort( m );
            qDebug( "Added Match \'%c\'", m->penChar->character() );
        }
    }

    if ( matches.first() ) {
        qDebug("Best match \'%c\'", matches.first()->penChar->character() );
    }

    return matches;
}

/*!
  Add a character \a ch to this set.
  QIMPenCharSet will delete this character when it is no longer needed.
*/
void QIMPenCharSet::addChar( QIMPenChar *ch )
{
    if ( ch->penStrokes().count() > maxStrokes )
        maxStrokes = ch->penStrokes().count();
    chars.append( ch );
}

/*!
  Remove a character \a ch from this set.
  QIMPenCharSet will delete this character.
*/
void QIMPenCharSet::removeChar( QIMPenChar *ch )
{
    chars.remove( ch );
}

/*!
  Move the character up the list of characters.
*/
void QIMPenCharSet::up( QIMPenChar *ch )
{
    int idx = chars.findRef( ch );
    if ( idx > 0 ) {
        chars.take();
        chars.insert( idx - 1, ch );
    }
}

/*!
  Move the character down the list of characters.
*/
void QIMPenCharSet::down( QIMPenChar *ch )
{
    int idx = chars.findRef( ch );
    if ( idx >= 0 && idx < (int)chars.count() - 1 ) {
        chars.take();
        chars.insert( idx + 1, ch );
    }
}


