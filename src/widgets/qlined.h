/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlined.h#20 $
**
** Definition of QLineEdit widget class
**
** Author  : Eirik Eng
** Created : 941011
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QLINED_H
#define QLINED_H

#include "qwidget.h"
#include "qstring.h"


class QLineEdit : public QWidget
{
    Q_OBJECT
public:
    QLineEdit( QWidget *parent=0, const char *name=0 );
   ~QLineEdit();

    const char *text() const;
    void	setMaxLength( int );
    int		maxLength()	const;

public slots:
    void	setText( const char * );
    void	selectAll();

signals:
    void	textChanged( const char * );
    void	returnPressed();

protected:
    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	paintEvent( QPaintEvent * );
    void	timerEvent( QTimerEvent * );
    void	resizeEvent( QResizeEvent * );

    bool        hasMarkedText() const;
    QString     markedText() const;

private:
    void	paint( bool frame = FALSE );
    void	pixmapPaint();
    void	paintText( QPainter *, const QSize &, bool frame = FALSE );
    void        newMark( int pos );
    void	cursorLeft( bool mark, int steps = 1 );
    void	cursorRight( bool mark, int steps = 1 );
    void	backspace();
    void	del();
    void	home( bool mark );
    void	end( bool mark );
    void	markWord( int pos );
    int		lastCharVisible() const;
    int		minMark() const;
    int		maxMark() const;

    QString	tbuf;
    QPixmap    *pm;
    int		cursorPos;
    int		offset;
    int		maxLen;
    int         markAnchor;
    int         markDrag;
    bool	cursorOn;
    bool	dragScrolling;
    bool	scrollingLeft;

private:	//Disabled copy constructor and operator=
    QLineEdit( const QLineEdit & ) {}
    QLineEdit &operator=( const QLineEdit & ) { return *this; }
};


#endif // QLINED_H
