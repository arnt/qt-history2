/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsplitter.h#4 $
**
**  Splitter widget
**
**  Created:  980105
**
** Copyright (C) 1998 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/
#ifndef QSPLITTER_H
#define QSPLITTER_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

class QSplitter : public QFrame
{
	Q_OBJECT;
public:
    enum Orientation { Horizontal, Vertical };

    QSplitter( QWidget *parent=0, const char *name=0 );
    QSplitter( Orientation, QWidget *parent=0, const char *name=0 );

    void setFirstWidget( QWidget * );
    void setSecondWidget( QWidget * );

    void setOrientation( Orientation );
    Orientation orientation() const { return orient; }

    void setRatio( float f );
    void setFixed( int w, int size );

    bool event( QEvent * );

    void setOpaqueResize( bool = TRUE );
    bool opaqueResize() const { return opaque; }

protected:
    void childInsertEvent( QChildEvent * );
    void childRemoveEvent( QChildEvent * );
    void layoutHintEvent( QEvent * );
    void leaveEvent( QEvent * );
    void resizeEvent( QResizeEvent * );
    //    void paintEvent( QPaintEvent * );
    void drawContents( QPainter * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

    void moveSplitter( QCOORD pos );
    virtual void drawSplitter( QPainter*, QCOORD x, QCOORD y, 
			       QCOORD w, QCOORD h );

    int adjustPos( int );
    void setRubberband( int );

    // styleChange
    // frameChange

    // virtual int border2()

private:
    void init();
    void recalc();
    int hit( QPoint p );
    void doResize();


    QCOORD pick( const QPoint &p ) const
    { return orient == Horizontal ? p.x() : p.y(); }
    QCOORD pick( const QSize &s ) const
    { return orient == Horizontal ? s.width() : s.height(); }

    QCOORD trans( const QPoint &p ) const
    { return orient == Vertical ? p.x() : p.y(); }
    QCOORD trans( const QSize &s ) const
    { return orient == Vertical ? s.width() : s.height(); }

    QCOORD r2p( int ) const;
    int p2r( QCOORD ) const;

    QWidget *w1;
    QWidget *w2;
    int moving;
    int ratio;
    int fixedWidget;
    void *d;
    bool opaque;

    Orientation orient;
    QCOORD bord; //half border
};


#endif //QSPLITTER_H
