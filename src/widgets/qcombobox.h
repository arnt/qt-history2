/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombobox.h#66 $
**
** Definition of QComboBox class
**
** Created : 950426
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

#ifndef QCOMBOBOX_H
#define QCOMBOBOX_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


struct QComboData;
class QStrList;
class QLineEdit;
class QValidator;
class QListBox;


class Q_EXPORT QComboBox : public QWidget
{
    Q_OBJECT
public:
    QComboBox( QWidget *parent=0, const char *name=0 );
    QComboBox( bool rw, QWidget *parent=0, const char *name=0 );
   ~QComboBox();

    int		count() const;

    void	insertStrList( const QStrList *, int index=-1 );
    void	insertStrList( const char **, int numStrings=-1, int index=-1);

    void	insertItem( const QString &text, int index=-1 );
    void	insertItem( const QPixmap &pixmap, int index=-1 );

    void	removeItem( int index );
    void	clear();

    QString currentText() const;
    QString text( int index ) const;
    const QPixmap *pixmap( int index ) const;

    void	changeItem( const QString &text, int index );
    void	changeItem( const QPixmap &pixmap, int index );

    int		currentItem() const;
    virtual void	setCurrentItem( int index );

    bool	autoResize()	const;
    virtual void	setAutoResize( bool );
    QSize	sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

    virtual void	setBackgroundColor( const QColor & );
    virtual void	setPalette( const QPalette & );
    virtual void	setFont( const QFont & );
    virtual void	setEnabled( bool );

    virtual void	setSizeLimit( int );
    int		sizeLimit() const;

    virtual void	setMaxCount( int );
    int		maxCount() const;

    enum Policy { NoInsertion, AtTop, AtCurrent, AtBottom,
		  AfterCurrent, BeforeCurrent };

    virtual void	setInsertionPolicy( Policy policy );
    Policy 	insertionPolicy() const;

    virtual void	setValidator( const QValidator * );
    const QValidator * validator() const;

    virtual void	setListBox( QListBox * );
    QListBox * 	listBox() const;

    virtual void	setAutoCompletion( bool );
    bool	autoCompletion() const;

    bool	eventFilter( QObject *object, QEvent *event );

public slots:
    void	clearValidator();
    void	clearEdit();
    virtual void	setEditText( const QString &);

signals:
    void	activated( int index );
    void	highlighted( int index );
    void	activated( const QString &);
    void	highlighted( const QString &);
    void	textChanged( const QString &);

private slots:
    void	internalActivate( int );
    void	internalHighlight( int );
    void	internalClickTimeout();
    void	returnPressed();

protected:
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent *e );
    void	focusInEvent( QFocusEvent *e );

    void	popup();
    void updateMask();

private:
    void	popDownListBox();
    void	reIndex();
    void	currentChanged();
    QRect	arrowRect() const;
    bool	getMetrics( int *dist, int *buttonW, int *buttonH ) const;

    QComboData	*d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QComboBox( const QComboBox & );
    QComboBox &operator=( const QComboBox & );
#endif
};


#endif // QCOMBOBOX_H
