/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qheader.h#9 $
**
**  Table header
**
**  Created:  961105
**
** Copyright (C) 1996-1997 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/
#ifndef QHEADER_H
#define QHEADER_H

#include "qtablevw.h"
#include "qarray.h"

class QHeader : public QTableView
{
    Q_OBJECT
public:
    enum Orientation { Horizontal, Vertical };

    QHeader( QWidget *parent=0, const char *name=0 );
    QHeader( int, QWidget *parent=0, const char *name=0 );
    ~QHeader();

    void	setLabel( int, const char *, int size = 0 );
    int		addLabel( const char *, int size = 0 );
    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool enable );
    bool	tracking() const;

    void 	setClickEnabled( bool, int logIdx = -1 );
    void	setResizeEnabled( bool, int logIdx = -1 );
    void	setMovingEnabled( bool );

    void	setCellSize( int i, int s );
    int		cellSize( int i ) const;
    int		cellPos( int i ) const;
    int		count() const;

    int 	offset() const;

    QSize	sizeHint() const;


    int		mapToLogical( int ) const;
    int		mapToActual( int ) const;

public slots:
    void	setOffset( int );

signals:
    void	sectionClicked( int );
    void	sizeChange( int section, int newSize );
    void	moved( int from, int to );
protected:
    //    void	timerEvent( QTimerEvent * );

    void	resizeEvent( QResizeEvent * );

    QRect	sRect( int i );

    void	paintCell( QPainter *, int, int );
    void	setupPainter( QPainter * );

    int		cellHeight( int );
    int		cellWidth( int );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );

private:
    void	init( int );
    //    void	recalc();
    void	paintRect( int p, int s );
    void	markLine( int idx );
    void	unMarkLine( int idx );
    int		pPos( int i ) const;
    int		pSize( int i ) const;

    int 	pos2idx( int );
    int 	findLine( int );

    void	moveAround( int fromIdx, int toIdx );

    int		handleIdx;
    int		moveToIdx;
    enum State { Idle, Sliding, Pressed, Moving };
    State	state;
    QCOORD	clickPos;
    bool	trackingIsOn;
    
    QArray<QCOORD>	sizes;
    QArray<const char*>	labels;
    QArray<int>	        a2l;
    QArray<int>	        l2a;


    Orientation orient;

private:	// Disabled copy constructor and operator=
    QHeader( const QHeader & ) {}
    QHeader &operator=( const QHeader & ) { return *this; }
};


inline QHeader::Orientation QHeader::orientation() const
{
    return orient;
}

inline int QHeader::count() const { return labels.size() - 1; }

inline void QHeader::setTracking( bool enable ) { trackingIsOn = enable; }
inline bool QHeader::tracking() const { return trackingIsOn; }

#endif //QHEADER_H
