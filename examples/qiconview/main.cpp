/****************************************************************************
** $Id: //depot/qt/main/examples/qiconview/main.cpp#4 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qiconview.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qpixmap.h>
#include <qiconset.h>

#include <qmime.h>
#include <stdio.h>

class ListenDND : public QObject
{
    Q_OBJECT

public:
    ListenDND( QWidget *w )
        : view( w )
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

protected:
    QWidget *view;

};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QIconView qiconview;
    qiconview.setSelectionMode( QIconView::Multi );
    qiconview.setViewMode( QIconSet::Large );
    qiconview.resize( 600, 480 );

    for ( unsigned int i = 0; i < 350; i++ )
        ( void )new QIconViewItem( &qiconview, QString( "Item %1" ).arg( i + 1 ) );

    qiconview.setCaption( "Iconview" );

    ListenDND listen_dnd( &qiconview );
    QObject::connect( &qiconview, SIGNAL( dropped( QDropEvent * ) ), &listen_dnd, SLOT( dropped( QDropEvent * ) ) );
    QObject::connect( &qiconview, SIGNAL( moved() ), &listen_dnd, SLOT( moved() ) );

    a.setMainWidget( &qiconview );
    qiconview.show();

    return a.exec();
}

#include "main.moc"
