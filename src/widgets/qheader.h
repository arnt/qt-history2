/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qheader.h#7 $
**
**  Table header
**
**  Created:  961105
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#ifndef QHEADER_H
#define QHEADER_H

#include "qwidget.h"
#include "qarray.h"

class QHeader : public QWidget
{
    Q_OBJECT
public:
    enum Orientation { Horizontal, Vertical };

    QHeader( QWidget *parent=0, const char *name=0 );
    QHeader( int, QWidget *parent=0, const char *name=0 );
    ~QHeader();

    void	setLabel( int, const char * );
    int		addLabel( const char * );
    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool enable );
    bool	tracking() const;

    int		cellSize( int i ) const;
    int		count() const;

    QSize	sizeHint() const;
signals:
    void	sectionClicked( int );
    void	sizeChange();
    void	moved( int from, int to );
protected:
    //    void	timerEvent( QTimerEvent * );

    void	resizeEvent( QResizeEvent * );

    QRect	sRect( int i );

    void	paintEvent( QPaintEvent * );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );

private:
    void	init( int );
    void	recalc();
    void	paintRect( int p, int s );
    void	markLine( int idx );
    void	unMarkLine( int idx );
    int		pPos( int i ) const { return places[i]; }
    int		pSize( int i ) const { return places[i+1] - places[i]; }

    int 	pos2idx( int );
    int 	findLine( int );

    void	moveAround( int fromIdx, int toIdx );

    int		handleIdx;
    int		moveToIdx;
    enum State { None, Sliding, Pressed, Moving };
    State	state;
    QCOORD	clickPos;
    bool	trackingIsOn;
    
    QArray<QCOORD>	places;
    QArray<const char*>	labels;

    Orientation orient;

private:	// Disabled copy constructor and operator=
    QHeader( const QHeader & ) {}
    QHeader &operator=( const QHeader & ) { return *this; }
};


inline QHeader::Orientation QHeader::orientation() const
{
    return orient;
}

inline int QHeader::cellSize( int i ) const { return pSize( i ); }
inline int QHeader::count() const { return places.size() - 1; }

inline void QHeader::setTracking( bool enable ) { trackingIsOn = enable; }
inline bool QHeader::tracking() const { return trackingIsOn; }

#endif //QHEADER_H
