/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsplitter.h#20 $
**
** Defintion of  QSplitter class
**
**  Created:  980105
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#ifndef QSPLITTER_H
#define QSPLITTER_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

class QSplitterHandle;
class QSplitterData;
class QSplitterLayoutStruct;

class Q_EXPORT QSplitter : public QFrame
{
    Q_OBJECT
public:
    enum ResizeMode { Stretch, KeepSize };

    QSplitter( QWidget *parent=0, const char *name=0 );
    QSplitter( Orientation, QWidget *parent=0, const char *name=0 );
    ~QSplitter();
    virtual void setOrientation( Orientation );
    Orientation orientation() const { return orient; }

    virtual void setResizeMode( QWidget *w, ResizeMode );

    //    bool event( QEvent * );

    virtual void setOpaqueResize( bool = TRUE );
    bool opaqueResize() const;

    void moveToFirst( QWidget * );
    void moveToLast( QWidget * );

    //void setHidden( QWidget *, bool );
    //bool isHidden( QWidget *) const;

    void refresh() { recalc( TRUE ); }
    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

protected:
    void childEvent( QChildEvent * );

    void layoutHintEvent( QEvent * );
    void resizeEvent( QResizeEvent * );

    void moveSplitter( QCOORD pos, int id );
    virtual void drawSplitter( QPainter*, QCOORD x, QCOORD y,
			       QCOORD w, QCOORD h );

    int adjustPos( int , int );
    virtual void setRubberband( int );
    void getRange( int id, int*, int* );

private:
    void init();
    void recalc( bool update = FALSE );
    int hit( QPoint p );
    void doResize();
    void storeSizes();
    QSplitterLayoutStruct *addWidget( QWidget*, bool first = FALSE );
    void recalcId();
    void moveBefore( int pos, int id, bool upLeft );
    void moveAfter( int pos, int id, bool upLeft );
    void setG( QWidget *w, int p, int s );

    // Josef Wagmann <josef.wagmann@passau.netsurf.de> wants access to these
    QCOORD pick( const QPoint &p ) const
    { return orient == Horizontal ? p.x() : p.y(); }
    QCOORD pick( const QSize &s ) const
    { return orient == Horizontal ? s.width() : s.height(); }

    QCOORD trans( const QPoint &p ) const
    { return orient == Vertical ? p.x() : p.y(); }
    QCOORD trans( const QSize &s ) const
    { return orient == Vertical ? s.width() : s.height(); }


#if 0
    QWidget *w1;
    QWidget *w2;
    QWidget *fixedWidget;
    QSplitterHandle *d;
#else
    QSplitterData *data;
#endif
    Orientation orient;
    QCOORD bord; //half border
    friend class QSplitterHandle;
};


#endif //QSPLITTER_H
