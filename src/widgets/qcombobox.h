/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombobox.h#1 $
**
** Definition of QComboBox class
**
** Author  : Eirik Eng
** Created : 950426
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QCOMBO_H
#define QCOMBO_H

#include "qwidget.h"


class QComboData;
class QStrList;

class QComboBox : public QWidget
{
    Q_OBJECT
public:
    QComboBox( QWidget *parent=0, const char *name=0 );
   ~QComboBox();

    void	 setStrList( const QStrList * );
    void	 setStrList( const char **, int numStrings );

    void	 insertStrList( const QStrList *, int index=-1 );
    void	 insertStrList( const char**, int numStrings, int index=-1 );

    void	 insertItem( const char *string, int index=-1 );
    void	 insertItem( const QPixmap &pixmap, int index=-1 );
//###    void	 inSort( const char *string );
    void	 removeItem( int index );

    const char	*string( int index ) const;	 // get string at index
    QPixmap     *pixmap( int index ) const;	 // get pixmap at index

    void	 changeItem( const char *string, int index );
    void	 changeItem( const QPixmap &pixmap, int index );
    void	 clear();

    int		 count() const;

    void	 setCurrentItem( int index );
    int		 currentItem() const;

    void         setAutoResizing( bool );
    bool         autoResizing() const;
    void         adjustSize();

    void         setBackgroundColor( const QColor & );

signals:
    void	 highlighted( int index );
    void	 activated( int index );

slots:  // private
    void         internalActivate( int );
    void         internalHighlight( int );

protected:    
    void         setPalette( const QPalette & );
    void         setFont( const QFont & );

    void	 paintEvent( QPaintEvent * );
    void	 mousePressEvent( QMouseEvent * );
    void	 mouseMoveEvent( QMouseEvent * );
    void	 mouseReleaseEvent( QMouseEvent * );
    void	 mouseDoubleClickEvent( QMouseEvent * );
    void	 keyPressEvent( QKeyEvent *e );

    void         popup();
    void         reIndex();

private:
    void	 init();
    void	 currentChanged();

    QComboData  *d;
};

#endif // QCOMBOBOX_H
