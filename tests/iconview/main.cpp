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

#include <qiconview.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <qpushbutton.h>

#include <qmime.h>
#include <stdio.h>

class ListenDND : public QObject
{
    Q_OBJECT

public:
    ListenDND( QWidget *w, QIconView*i )
        : view( w ), iv( i )
    {}

public slots:
    void dropped( QDropEvent *mime ) {
        printf( "Dropped Mimesource %p into the view %p\n", mime, view );
        printf( "  Formats:\n" );
        int i = 0;
        const char *str = mime->format( i );
        printf( "    %s\n", str );
        while ( str ) {
            str = mime->format( ++i );
            printf( "    %s\n", str );
        }
    };
    void moved() {
        printf( "All selected items were moved to another widget\n" );
    }

    void selectionChanged() {
	qDebug( "selectionChanged" );
    }
    void selectionChanged( QIconViewItem *item ) {
	if ( !item )
	    return;
	qDebug( "selectionChanged %p %s", item, item->text().latin1() );
    }
    void currentChanged( QIconViewItem *item ) {
	if ( !item )
	    return;
	qDebug( "currentChanged %p %s", item, item->text().latin1() );
    }
    void remove() {
	if ( iv->currentItem() )
	    delete iv->currentItem();
    }
    void clear() {
	iv->selectAll( FALSE );
    }
    void select() {
	iv->setSelected( iv->firstItem()->nextItem(), TRUE );
    }
    void current() {
	iv->setCurrentItem( iv->firstItem()->nextItem() );
    }
    
    
protected:
    QWidget *view;
    QIconView *iv;
    
	
};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QIconView qiconview;
    qiconview.setSelectionMode( QIconView::NoSelection );

    for ( unsigned int i = 0; i < 3000; i++ ) {
	QIconViewItem *item = new QIconViewItem( &qiconview, QString( "Item %1" ).arg( i + 1 ) );
	item->setRenameEnabled( TRUE );
    }

    qiconview.setCaption( "Iconview" );
    ListenDND listen_dnd( &qiconview, &qiconview );
    QObject::connect( &qiconview, SIGNAL( selectionChanged() ),
		      &listen_dnd, SLOT( selectionChanged() ) );
    QObject::connect( &qiconview, SIGNAL( selectionChanged( QIconViewItem * ) ),
		      &listen_dnd, SLOT( selectionChanged( QIconViewItem * ) ) );
    QObject::connect( &qiconview, SIGNAL( currentChanged( QIconViewItem * ) ),
		      &listen_dnd, SLOT( currentChanged( QIconViewItem * ) ) );

    QObject::connect( &qiconview, SIGNAL( dropped( QDropEvent * ) ), &listen_dnd, SLOT( dropped( QDropEvent * ) ) );
    QObject::connect( &qiconview, SIGNAL( moved() ), &listen_dnd, SLOT( moved() ) );

    a.setMainWidget( &qiconview );
    qiconview.show();
    qiconview.resize( qiconview.sizeHint() );

    QPushButton *pb = new QPushButton( "remove", 0 );
    pb->show();
    QObject::connect( pb, SIGNAL( clicked() ),
	     &listen_dnd, SLOT( remove() ) );
    pb = new QPushButton( "clear", 0 );
    pb->show();
    QObject::connect( pb, SIGNAL( clicked() ),
	     &listen_dnd, SLOT( clear() ) );
    pb = new QPushButton( "select", 0 );
    pb->show();
    QObject::connect( pb, SIGNAL( clicked() ),
	     &listen_dnd, SLOT( select() ) );
    pb = new QPushButton( "current", 0 );
    pb->show();
    QObject::connect( pb, SIGNAL( clicked() ),
	     &listen_dnd, SLOT( current() ) );
    
    return a.exec();
}

#include "main.moc"
