/**********************************************************************
** $Id$
**
** Definition of QLineEdit widget class
**
** Created : 941011
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
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

#if QT_FEATURE_WIDGETS

class Q_EXPORT QLineEdit : public QWidget
{
    Q_OBJECT
    Q_ENUMS( EchoMode )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( int maxLength READ maxLength WRITE setMaxLength )
    Q_PROPERTY( bool frame READ frame WRITE setFrame )
    Q_PROPERTY( EchoMode echoMode READ echoMode WRITE setEchoMode )
    Q_PROPERTY( QString displayText READ displayText )
    Q_PROPERTY( int cursorPosition READ cursorPosition WRITE setCursorPosition )
    Q_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
    Q_PROPERTY( bool edited READ edited WRITE setEdited )
    Q_PROPERTY( bool hasMarkedText READ hasMarkedText )
    Q_PROPERTY( QString markedText READ markedText )
	
public:
    QLineEdit( QWidget *parent, const char *name=0 );
    QLineEdit( const QString &, QWidget *parent, const char *name=0 );
   ~QLineEdit();

    QString text() const;

    QString displayText() const;

    int		maxLength()	const;
    virtual void setMaxLength( int );

    virtual void setFrame( bool );
    bool	frame() const;

    enum	EchoMode { Normal, NoEcho, Password };
    virtual void setEchoMode( EchoMode );
    EchoMode 	echoMode() const;
#if QT_VERSION >= 300
#error "Make setReadOnly virtual"
#endif
    void setReadOnly( bool );
    bool isReadOnly() const;

    virtual void setValidator( const QValidator * );
    const QValidator * validator() const;

    QSize	sizeHint() const;
    QSize	minimumSizeHint() const;
    QSizePolicy sizePolicy() const;

    virtual void setEnabled( bool );
    virtual void setFont( const QFont & );
    virtual void setPalette( const QPalette & );
    virtual void setSelection( int, int );

    virtual void setCursorPosition( int );
    int		cursorPosition() const;

    bool	validateAndSet( const QString &, int, int, int );

#if QT_FEATURE_CLIPBOARD
    void	cut();
    void	copy() const;
    void	paste();
#endif

    void setAlignment( int flag );
    int alignment() const;

    void	cursorLeft( bool mark, int steps = 1 );
    void	cursorRight( bool mark, int steps = 1 );
    void	cursorWordForward( bool mark );
    void	cursorWordBackward( bool mark );
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
    void	resizeEvent( QResizeEvent * );
    void	leaveEvent( QEvent * );
#if QT_FEATURE_DRAGANDDROP
    void	dragEnterEvent( QDragEnterEvent * );
    void	dropEvent( QDropEvent * );
#endif

    void	repaintArea( int, int );

private slots:
    void	clipboardChanged();
    void	blinkSlot();
    void	dragScrollSlot();
#if QT_FEATURE_DRAGANDDROP
    void 	doDrag();
#endif

private:
    // kept
    void	newMark( int pos, bool copy=TRUE );
    void	markWord( int pos );
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
    bool	ed;

    void updateOffset();
    int xPosToCursorPos( int ) const;
    void blinkOn();
    void makePixmap() const;
    void undoInternal();
    void redoInternal();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLineEdit( const QLineEdit & );
    QLineEdit &operator=( const QLineEdit & );
#endif
};


#endif // QT_FEATURE_WIDGETS

#endif // QLINEEDIT_H
