/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlined.h#12 $
**
** Definition of QLineEdit widget class
**
** Author  : Eirik Eng
** Created : 941011
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
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

    char       *text() const;
    void	setMaxLength( int );
    int		maxLength()	const;

public slots:
    void	setText( const char * );

signals:
    void	textChanged( char * );
    void        returnPressed();

protected:
    void	mousePressEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	paintEvent( QPaintEvent * );
    void	timerEvent( QTimerEvent * );
    void	resizeEvent( QResizeEvent * );

private:
    void	paint( bool frame = FALSE );
    void	pixmapPaint();
    void	paintText( QPainter *, const QSize &, bool frame = FALSE );
    void	cursorLeft();
    void	cursorRight();
    void	backspace();
    void	del();
    void	home();
    void	end();

    QString	tbuf;
    QPixmap    *pm;
    uint	cursorPos;
    uint	offset;
    uint	maxLen;
    uint	cursorOn	: 1;
};


#endif // QLINED_H
