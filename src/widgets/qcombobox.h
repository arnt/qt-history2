/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombobox.h#37 $
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
    QComboBox( bool, QWidget *parent=0, const char *name=0 );
   ~QComboBox();

    int		count() const;

    void	insertStrList( const QStrList *, int index=-1 );
    void	insertStrList( const char **, int numStrings=-1, int index=-1);

    void	insertItem( const char *text, int index=-1 );
    void	insertItem( const QPixmap &pixmap, int index=-1 );

    void	removeItem( int index );
    void	clear();

    const char *currentText() const;
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
    void	setEnabled( bool );

    void	setSizeLimit( int );
    int		sizeLimit() const;

    void	setMaxCount( int );
    int		maxCount() const;

    enum Policy { NoInsertion, AtTop, AtCurrent, AtBottom };

    void	setInsertionPolicy( QComboBox::Policy policy );
    QComboBox::Policy insertionPolicy() const;

signals:
    void	activated( int index );
    void	highlighted( int index );
    void	activated( const char * );
    void	highlighted( const char * );

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


#endif // QCOMBOBOX_H
