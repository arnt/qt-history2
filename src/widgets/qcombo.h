/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombo.h#28 $
**
** Definition of QComboBox class
**
** Created : 950426
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QCOMBO_H
#define QCOMBO_H

#include "qwidget.h"


struct QComboData;
class  QStrList;
class  QLineEdit;


class QComboBox : public QWidget
{
    Q_OBJECT
public:
    QComboBox( QWidget *parent=0, const char *name=0 );
   ~QComboBox();

    int		count() const;

    void	insertStrList( const QStrList *, int index=-1 );
    void	insertStrList( const char **, int numStrings=-1, int index=-1);

    void	insertItem( const char *text, int index=-1 );
    void	insertItem( const QPixmap &pixmap, int index=-1 );

    void	removeItem( int index );
    void	clear();

    const char *text( int index ) const;
    const QPixmap *pixmap( int index ) const;

    void	changeItem( const char *text, int index );
    void	changeItem( const QPixmap &pixmap, int index );

    int		currentItem() const;
    void	setCurrentItem( int index );

    bool	autoResize()	const;
    void	setAutoResize( bool );
    QSize	sizeHint() const;

    void	setBackgroundColor( const QColor & );
    void	setPalette( const QPalette & );
    void	setFont( const QFont & );

signals:
    void	activated( int index );
    void	highlighted( int index );

private slots:
    void	internalActivate( int );
    void	internalHighlight( int );
    void	internalClickTimeout();

protected:
    void	paintEvent( QPaintEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent *e );

    void	popup();

private:
    void	popDownListBox();
    void	reIndex();
    void	currentChanged();
    QRect	arrowRect() const;
    bool	getMetrics( int *dist, int *buttonW, int *buttonH ) const;
    bool	eventFilter( QObject *object, QEvent *event );

    QComboData	*d;

private:	// Disabled copy constructor and operator=
    QComboBox( const QComboBox & ) {}
    QComboBox &operator=( const QComboBox & ) { return *this; }
};


class QEditableComboBox: public QComboBox
{
    Q_OBJECT
public:
    QEditableComboBox( QWidget *parent=0, const char *name=0 );

    void	insertItem( const char *text, int index=-1 )
		{ QComboBox::insertItem( text, index ); }
    void	changeItem( const char *text, int index=-1 )
		{ QComboBox::changeItem( text, index ); }

signals:
    void	activated( const char * );
    void	highlighted( const char * );

protected:
    void	resizeEvent( QResizeEvent * );

private slots:
    void	translateActivate( int );
    void	translateHighlight( int );
    void	returnPressed();

private:
    QLineEdit * ed;  // /bin/ed rules!

private:	// Disabled functions
    QEditableComboBox( const QEditableComboBox & ) {}
    QEditableComboBox &operator=( const QEditableComboBox & ) { return *this; }

    void insertItem( const QPixmap &, int =1 ) {}
    const QPixmap *pixmap( int ) const { return 0; }
    void changeItem( const QPixmap &, int ) {}
};


#endif // QCOMBOBOX_H
