/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qheader.h#30 $
**
** Definition of QHeader widget class (table header)
**
** Created : 961105
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QHEADER_H
#define QHEADER_H

#ifndef QT_H
#include "qtableview.h"
#endif // QT_H

struct QHeaderData;

class Q_EXPORT QHeader : public QWidget
{
    Q_OBJECT
public:
    QHeader( QWidget *parent=0, const char *name=0 );
    QHeader( int, QWidget *parent=0, const char *name=0 );
    ~QHeader();

    int		addLabel( const QString &, int size = -1 );
    virtual void setLabel( int, const QString &, int size = -1 );
    QString 	label( int );
    virtual void setOrientation( Orientation );
    Orientation orientation() const;
    virtual void setTracking( bool enable );
    bool	tracking() const;

    virtual void setClickEnabled( bool, int logIdx = -1 );
    virtual void setResizeEnabled( bool, int logIdx = -1 );
    virtual void setMovingEnabled( bool );

    virtual void setCellSize( int i, int s );
    int		cellSize( int i ) const;
    int		cellPos( int i ) const;
    int		cellAt( int i ) const;
    int		count() const;

    int 	offset() const;

    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

    int		mapToLogical( int ) const;
    int		mapToActual( int ) const;

    virtual void moveCell( int fromIdx, int toIdx );

public slots:
    virtual void	setOffset( int );

signals:
    void	sectionClicked( int );
    void	sizeChange( int section, int oldSize, int newSize );
    void	moved( int from, int to );

protected:
    void	resizeEvent( QResizeEvent * );
    void	paintEvent( QPaintEvent * );
    QRect	sRect( int i );

    void	paintSection( QPainter *, int, QRect );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );

private:
    void	init( int );

    void	paintRect( int p, int s );
    void	markLine( int idx );
    void	unMarkLine( int idx );
    int		pPos( int i ) const;
    int		pSize( int i ) const;

    int 	findLine( int );

    void	handleColumnResize(int, int, bool);

    int		offs;
    int		handleIdx;
    int		oldHIdxSize;
    int		moveToIdx;
    enum State { Idle, Sliding, Pressed, Moving, Blocked };
    State	state;
    QCOORD	clickPos;
    bool	trackingIsOn;
    int       cachedIdx;
    int	cachedPos;
    Orientation orient;

    QHeaderData *data;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QHeader( const QHeader & );
    QHeader &operator=( const QHeader & );
#endif
};


inline QHeader::Orientation QHeader::orientation() const
{
    return orient;
}

inline void QHeader::setTracking( bool enable ) { trackingIsOn = enable; }
inline bool QHeader::tracking() const { return trackingIsOn; }

#endif //QHEADER_H
