/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlineedit.h#49 $
**
** Definition of QLineEdit widget class
**
** Created : 941011
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QLINED_H
#define QLINED_H

struct QLineEditPrivate;

class QComboBox;
class QValidator;


#ifndef QT_H
#include "qwidget.h"
#include "qstring.h"
#endif // QT_H


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
    EchoMode 	echoMode() const;

    void	setValidator( QValidator * );
    QValidator * validator() const;

    QSize	sizeHint() const;

    void	setEnabled( bool );
    void	setFont( const QFont & );

public slots:
    void	setText( const char * );
    void	selectAll();
    void	deselect();

    void	clearValidator();

    void	insert( const char * );
    
    void	clear();

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

    bool	event( QEvent * );

    bool	hasMarkedText() const;
    QString	markedText() const;

    bool	validateAndSet( const char *, int, int, int );

    void	repaintArea( int, int );

private slots:
    void	clipboardChanged();
    void	blinkSlot();
    void	dragScrollSlot();

private:
    // obsolete
    void	paint( const QRect& clip, bool frame = FALSE );
    void	pixmapPaint( const QRect& clip );
    // kept
    void	paintText( QPainter *, const QSize &, bool frame = FALSE );
    // to be replaced by publics
    void	cursorLeft( bool mark, int steps = 1 );
    void	cursorRight( bool mark, int steps = 1 );
    void	backspace();
    void	del();
    void	home( bool mark );
    void	end( bool mark );
    // kept
    void	newMark( int pos, bool copy=TRUE );
    void	markWord( int pos );
    void	copyText();
    int		lastCharVisible() const;
    int		minMark() const;
    int		maxMark() const;

    QString	tbuf;
    QLineEditPrivate * d;
    int		cursorPos;
    int		offset;
    int		maxLen;
    int		markAnchor;
    int		markDrag;
    bool	cursorOn;
    bool	dragScrolling;
    bool	scrollingLeft;

private:	// Disabled copy constructor and operator=
    QLineEdit( const QLineEdit & );
    QLineEdit &operator=( const QLineEdit & );

    friend class QComboBox;
};


#endif // QLINED_H
