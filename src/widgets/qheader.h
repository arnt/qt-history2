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
    QHeader( int minValue, int maxValue, int step, int value, Orientation,
	 QWidget *parent=0, const char *name=0 );
    ~QHeader() {}


    void	setLabel( int, const char * );
    void	addLabel( const char * );
    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool enable );
    bool	tracking() const;

    int		cellSize( int i ) const { return pSize( i ); }
    int		count() const { return places.size() - 1; }

    QSize	sizeHint();
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
    //bool	track;
    void	paintLine( int idx, int oldPos, int newPos );
    void	paintRect( int p, int s );
    void	markLine( int idx );
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

#endif //QHEADER_H
