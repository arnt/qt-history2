/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlineedit.h#71 $
**
** Definition of QLineEdit widget class
**
** Created : 941011
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

#ifndef QLINEEDIT_H
#define QLINEEDIT_H

struct QLineEditPrivate;

class QComboBox;
class QValidator;


#ifndef QT_H
#include "qwidget.h"
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QLineEdit : public QWidget
{
    Q_OBJECT
public:
    QLineEdit( QWidget *parent, const char *name=0 );
    QLineEdit( const QString &, QWidget *parent, const char *name=0 );
   ~QLineEdit();

    QString text() const;
    int		maxLength()	const;
    virtual void setMaxLength( int );

    virtual void setFrame( bool );
    bool	frame() const;

    enum	EchoMode { Normal, NoEcho, Password };
    virtual void setEchoMode( EchoMode );
    EchoMode 	echoMode() const;

    virtual void setValidator( const QValidator * );
    const QValidator * validator() const;

    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

    virtual void setEnabled( bool );
    virtual void setFont( const QFont & );
    virtual void setPalette( const QPalette & );

    virtual void setSelection( int, int );
    virtual void setCursorPosition( int );
    int		cursorPosition() const;

    bool	validateAndSet( const QString &, int, int, int );

    void	cut();
    void	copy() const;
    void	paste();

    void setAlignment( int flag );
    int alignment() const;

    void	cursorLeft( bool mark, int steps = 1 );
    void	cursorRight( bool mark, int steps = 1 );
    void	backspace();
    void	del();
    void	home( bool mark );
    void	end( bool mark );

    void	setEdited( bool );
    bool	edited() const;

    bool	hasMarkedText() const;
    QString	markedText() const;

public slots:
    virtual void setText( const QString &);
    void	selectAll();
    void	deselect();

    void	clearValidator();

    void	insert( const QString &);

    void	clear();

signals:
    void	textChanged( const QString &);
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
    void	leaveEvent( QEvent * );
    void	dragEnterEvent( QDragEnterEvent * );
    void	dropEvent( QDropEvent * );

    void	repaintArea( int, int );

private slots:
    void	clipboardChanged();
    void	blinkSlot();
    void	dragScrollSlot();

private:
    // kept
    void	newMark( int pos, bool copy=TRUE );
    void	markWord( int pos );
    void	copyText();
    int		lastCharVisible() const;
    int		minMark() const;
    int		maxMark() const;

    void	init();

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
    int		alignmentFlag;
    int 	alignOffset;
    bool	ed;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLineEdit( const QLineEdit & );
    QLineEdit &operator=( const QLineEdit & );
#endif

    friend class QComboBox;
};


#endif // QLINEEDIT_H
