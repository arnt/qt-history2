/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LISTBOX_H
#define LISTBOX_H


class QSpinBox;
class QListBox;
class QButtonGroup;

#include <qwidget.h>
#include <qlistbox.h>


class ListBoxDemo: public QWidget
{
    Q_OBJECT
public:
    ListBoxDemo();
    ~ListBoxDemo();

private slots:
    void setNumRows();
    void setNumCols();
    void setRowsByHeight();
    void setColsByWidth();
    void setVariableWidth( bool );
    void setVariableHeight( bool );
    void setMultiSelection( bool );
    void sortAscending();
    void sortDescending();

    void highlighted( int index ) {
	qDebug( "highlighted %d", index );
    }
    void selected( int index ){
	qDebug( "selected %d", index );
    }
    void highlighted( const QString &s){
	qDebug( "highlighted %s", s.latin1() );
    }
    void selected( const QString &s){
	qDebug( "selected %s", s.latin1() );
    }
    void highlighted( QListBoxItem *i ){
	if ( !i )
	    return;
	qDebug( "highlighted %p %s", i, i->text().latin1() );
    }
    void selected( QListBoxItem *i ){
	if ( !i )
	    return;
	qDebug( "selected %p %s", i, i->text().latin1() );
    }
    void selectionChanged(){
	qDebug( "selectionChanged" );
    }
    void selectionChanged( QListBoxItem *i ){
	if ( !i )
	    return;
	qDebug( "selectionChanged %p %s", i, i->text().latin1() );
    }
    void currentChanged( QListBoxItem *i ){
	if ( !i )
	    return;
	qDebug( "currentChanged %p %s", i, i->text().latin1() );
    }
    void clicked( QListBoxItem *i ){
	if ( !i )
	    return;
	qDebug( "clicked %p %s", i, i->text().latin1() );
    }
    void pressed( QListBoxItem *i ){
	if ( !i )
	    return;
	qDebug( "pressed %p %s", i, i->text().latin1() );
    }
    void doubleClicked( QListBoxItem *i ){
	if ( !i )
	    return;
	qDebug( "doubleClicked %p %s", i, i->text().latin1() );
    }
    void returnPressed( QListBoxItem *i ){
	if ( !i )
	    return;
	qDebug( "returnPressed %p %s", i, i->text().latin1() );
    }
    void rightButtonClicked( QListBoxItem *i, const QPoint & ){
	if ( !i )
	    return;
	qDebug( "rightButtonClicked %p %s", i, i->text().latin1() );
    }
    void rightButtonPressed( QListBoxItem *i, const QPoint & ){
	if ( !i )
	    return;
	qDebug( "rightButtonPressed %p %s", i, i->text().latin1() );
    }
    void mouseButtonPressed( int, QListBoxItem*i, const QPoint& ){
	if ( !i )
	    return;
	qDebug( "mouseButtonPressed %p %s", i, i->text().latin1() );
    }
    void mouseButtonClicked( int, QListBoxItem*i, const QPoint& ){
	if ( !i )
	    return;
	qDebug( "mouseButtonClicked %p %s", i, i->text().latin1() );
    }
    void onItem( QListBoxItem *i ){
	if ( !i )
	    return;
	qDebug( "onItem %p %s", i, i->text().latin1() );
    }
    void onViewport(){
	qDebug( "onViewport" );
    }

private:
    QListBox * l;
    QSpinBox * columns;
    QSpinBox * rows;
    QButtonGroup * bg;
};


#endif
