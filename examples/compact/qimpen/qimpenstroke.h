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

#ifndef QIMPENSTROKE_H_
#define QIMPENSTROKE_H_

#include <qobject.h>
#include <qarray.h>
#include <qlist.h>

struct QIMPenGlyphLink
{
    char dx;
    char dy;
};

class QIMPenStroke
{
public:
    QIMPenStroke();
    QIMPenStroke( const QIMPenStroke & );

    void clear();
    bool isEmpty() const { return links.isEmpty(); }
    int match( QIMPenStroke *st );
    const QArray<QIMPenGlyphLink> &chain() const { return links; }
    QPoint startingPoint() const { return startPoint; }
    QRect boundingRect();

    QIMPenStroke &operator=( const QIMPenStroke &s );

    void beginInput( QPoint p );
    bool addPoint( QPoint p );
    void endInput();

    QArray<int> sig() { createTanSignature(); return tsig; } // for debugging

protected:
    void createSignatures();
    void createTanSignature();
    void createAngleSignature();
    void createDistSignature();
    int calcError( const QArray<int> &base, const QArray<int> &win,
                      int off, bool t );
    QArray<int> scale( const QArray<int> &s, unsigned count, bool t = FALSE );
    void internalAddPoint( QPoint p );
    QPoint calcCenter();
    int arcTan( int dy, int dx );
    QArray<int> createBase( const QArray<int> a, int e );
    void smooth( QArray<int> &);

protected:
    QPoint startPoint;
    QPoint lastPoint;
    QArray<QIMPenGlyphLink> links;
    QArray<int> tsig;
    QArray<int> asig;
    QArray<int> dsig;

    friend QDataStream &operator<< (QDataStream &, const QIMPenStroke &);
    friend QDataStream &operator>> (QDataStream &, QIMPenStroke &);
};

QDataStream & operator<< (QDataStream & s, const QIMPenStroke &ws);
QDataStream & operator>> (QDataStream & s, const QIMPenStroke &ws);

#endif

