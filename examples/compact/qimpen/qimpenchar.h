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

#ifndef QIMPENCHAR_H_
#define QIMPENCHAR_H_

#include <qlist.h>
#include "qimpenstroke.h"

struct QIMPenSpecialKeys {
    int code;
    char *name;
};

extern const QIMPenSpecialKeys qimpen_specialKeys[];


class QIMPenChar
{
public:
    QIMPenChar();
    QIMPenChar( const QIMPenChar & );

    unsigned int character() const { return ch; }
    void setCharacter( unsigned int c ) { ch = c; }

    bool isEmpty() const { return strokes.isEmpty(); }
    void clear();
    int match( QIMPenChar *ch );
    const QList<QIMPenStroke> &penStrokes() { return strokes; }
    QPoint startingPoint() const { return strokes.getFirst()->startingPoint(); }
    QRect boundingRect();

    void setFlag( int f ) { flags |= f; }
    void clearFlag( int f ) { flags &= ~f; }
    bool testFlag( int f ) { return flags & f; }

    enum Flags { System=0x01, Deleted=0x02 };

    QIMPenChar &operator=( const QIMPenChar &s );

    void addStroke( QIMPenStroke * );

protected:
    unsigned int ch;
    Q_UINT8 flags;
    QList<QIMPenStroke> strokes;

    friend QDataStream &operator<< (QDataStream &, const QIMPenChar &);
    friend QDataStream &operator>> (QDataStream &, QIMPenChar &);
};

QDataStream & operator<< (QDataStream & s, const QIMPenChar &ws);
QDataStream & operator>> (QDataStream & s, QIMPenChar &ws);

struct QIMPenCharMatch
{
    int error;
    QIMPenChar *penChar;
};

class QIMPenCharMatchList : public QList<QIMPenCharMatch>
{
public:
    virtual int compareItems( QCollection::Item, QCollection::Item );
};


class QIMPenCharSet
{
public:
    QIMPenCharSet();
    QIMPenCharSet( const QString &fn );

    bool isEmpty() { return chars.isEmpty(); }

    void setDescription( const QString &d ) { desc = d; }
    QString description() const { return desc; }
    void setTitle( const QString &t ) { csTitle = t; }
    QString title() const { return csTitle; }

    const QIMPenCharMatchList &match( QIMPenChar *ch );
    void addChar( QIMPenChar *ch );
    void removeChar( QIMPenChar *ch );

    unsigned maximumStrokes() const { return maxStrokes; }

    void up( QIMPenChar *ch );
    void down( QIMPenChar *ch );

    QList<QIMPenChar> characters() { return chars; }

    void setFilename( const QString &fn ) { filename = fn; }
    void load( const QString &fn );
    void save();

protected:
    QString csTitle;
    QString desc;
    QString filename;
    unsigned maxStrokes;
    QList<QIMPenChar> chars;
    QIMPenCharMatchList matches;
};

#endif
