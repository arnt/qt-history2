/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlined.h#39 $
**
** Definition of QLineEdit widget class
**
** Created : 941011
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QLINED_H
#define QLINED_H

#include "qwidget.h"
#include "qstring.h"

class QComboBox;
class QValidator;

class QLineEdit : public QWidget
{
    Q_OBJECT
public:
    QLineEdit( QWidget *parent=0, const char *name=0 );
   ~QLineEdit();

    const char *text() const;
    int		maxLength()	const;
    void	setMaxLength( int );

    void	setFrame( bool );
    bool	frame() const;

    enum	EchoMode { Normal, NoEcho, Password };
    void	setEchoMode( EchoMode );
    QLineEdit::EchoMode echoMode() const;

    void	setValidator( QValidator * );
    QValidator * validator() const;

    QSize	sizeHint() const;

    void	setPalette( const QPalette & );

public slots:
    void	setText( const char * );
    void	selectAll();
    void	deselect();

    void	clearValidator();

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

    bool	hasMarkedText() const;
    QString	markedText() const;

private slots:
    void	clipboardChanged();

private:
    void	paint( bool frame = FALSE );
    void	pixmapPaint();
    void	paintText( QPainter *, const QSize &, bool frame = FALSE );
    void	cursorLeft( bool mark, int steps = 1 );
    void	cursorRight( bool mark, int steps = 1 );
    void	backspace();
    void	del();
    void	home( bool mark );
    void	end( bool mark );
    void	newMark( int pos, bool copy=TRUE );
    void	markWord( int pos );
    void	copyText();
    int		lastCharVisible() const;
    int		minMark() const;
    int		maxMark() const;

    QString	tbuf;
    QPixmap    *pm;
    int		cursorPos;
    int		offset;
    int		maxLen;
    int		markAnchor;
    int		markDrag;
    bool	cursorOn;
    bool	dragScrolling;
    bool	scrollingLeft;

private:	// Disabled copy constructor and operator=
    QLineEdit( const QLineEdit & ) {}
    QLineEdit &operator=( const QLineEdit & ) { return *this; }

    friend class QComboBox;
};


#endif // QLINED_H
