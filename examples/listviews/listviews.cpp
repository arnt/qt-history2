/****************************************************************************
** $Id: //depot/qt/main/examples/listviews/listviews.cpp#2 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "listviews.h"

#include <qlabel.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qobjectlist.h>

// -----------------------------------------------------------------

Folder::Folder( Folder *parent, const QString &name )
    : QObject( parent, name ), fName( name )
{
    lstMessages.setAutoDelete( TRUE );
}

// -----------------------------------------------------------------

FolderListItem::FolderListItem( QListView *parent, Folder *f )
    : QListViewItem( parent )
{
    myFolder = f;
    setText( 0, f->folderName() );

    if ( myFolder->children() )
        insertSubFolders( myFolder->children() );
}

FolderListItem::FolderListItem( FolderListItem *parent, Folder *f )
    : QListViewItem( parent )
{
    myFolder = f;

    setText( 0, f->folderName() );

    if ( myFolder->children() )
        insertSubFolders( myFolder->children() );
}

void FolderListItem::insertSubFolders( const QObjectList *lst )
{
    Folder *f;
    for ( f = ( Folder* )( ( QObjectList* )lst )->first(); f; f = ( Folder* )( ( QObjectList* )lst )->next() )
        (void)new FolderListItem( this, f );
}

// -----------------------------------------------------------------

MessageListItem::MessageListItem( QListView *parent, Message *m )
    : QListViewItem( parent )
{
    myMessage = m;
    setText( 0, myMessage->header().sender() );
    setText( 1, myMessage->header().subject() );
    setText( 2, myMessage->header().datetime().toString() );
}

void MessageListItem::paintCell( QPainter *p, const QColorGroup &cg,
                                 int column, int width, int alignment )
{
    QColorGroup _cg( cg );
    QColor c = _cg.text();

    if ( myMessage->state() == Message::Unread )
        _cg.setColor( QColorGroup::Text, Qt::red );

    QListViewItem::paintCell( p, _cg, column, width, alignment );

    _cg.setColor( QColorGroup::Text, c );
}

// -----------------------------------------------------------------

ListViews::ListViews( QWidget *parent, const char *name )
    : QSplitter( Qt::Horizontal, parent, name )
{
    lstFolders.setAutoDelete( TRUE );

    folders = new QListView( this );
    folders->addColumn( "Folder" );

    initFolders();
    setupFolders();

    folders->setMinimumSize( QSize( 200, 0 ) );
    folders->setRootIsDecorated( TRUE );
    setResizeMode( folders, QSplitter::KeepSize );

    QSplitter *vsplitter = new QSplitter( Qt::Vertical, this );

    messages = new QListView( vsplitter );
    messages->addColumn( "Sender" );
    messages->addColumn( "Subject" );
    messages->addColumn( "Date" );
    messages->resize( 0, 200 );
    vsplitter->setResizeMode( messages, QSplitter::KeepSize );

    message = new QLabel( vsplitter );
    message->setAlignment( Qt::AlignTop );

    connect( folders, SIGNAL( selectionChanged( QListViewItem* ) ), this, SLOT( slotFolderChanged( QListViewItem* ) ) );
    connect( messages, SIGNAL( selectionChanged( QListViewItem* ) ), this, SLOT( slotMessageChanged( QListViewItem* ) ) );

    // some preperationes
    folders->firstChild()->setOpen( TRUE );
    folders->firstChild()->firstChild()->setOpen( TRUE );
    folders->setCurrentItem( folders->firstChild()->firstChild()->firstChild() );
    folders->setSelected( folders->firstChild()->firstChild()->firstChild(), TRUE );

    messages->setSelected( messages->firstChild(), TRUE );
    messages->setCurrentItem( messages->firstChild() );
}

void ListViews::initFolders()
{
    unsigned int mcount = 1;

    for ( unsigned int i = 1; i < 20; i++ ) {
        QString str;
        str.sprintf( "Folder %d", i );
        Folder *f = new Folder( 0L, str );
        for ( unsigned int j = 1; j < 5; j++ ) {
            QString str2;
            str2.sprintf( "Sub Folder %d", j );
            Folder *f2 = new Folder( f, str2 );
            for ( unsigned int k = 1; k < 3; k++ ) {
                QString str3;
                str3.sprintf( "Sub Sub Folder %d", k );
                Folder *f3 = new Folder( f2, str3 );
                initFolder( f3, mcount );
            }
        }
        lstFolders.append( f );
    }
}

void ListViews::initFolder( Folder *folder, unsigned int &count )
{
    for ( unsigned int i = 0; i < 15; i++, count++ ) {
        QString str;
        str.sprintf( "Message %d  ", count );
        QDateTime dt = QDateTime::currentDateTime();
        dt = dt.addSecs( 60 * count );
        MessageHeader mh( "Troll Tech <info@troll.no>  ", str, dt );

        QString body;
        body.sprintf( "This is the message number %d of this application, \n"
                      "which shows how to use QListViews, QListViewItems, \n"
                      "QSplitters and so on. The code should show how easy\n"
                      "this can be done in Qt.", count );
        Message *msg = new Message( mh, body );
        folder->addMessage( msg );
    }
}

void ListViews::setupFolders()
{
    folders->clear();

    for ( Folder* f = lstFolders.first(); f; f = lstFolders.next() )
        (void)new FolderListItem( folders, f );
}

void ListViews::slotFolderChanged( QListViewItem *i )
{
    messages->clear();
    message->setText( "" );

    FolderListItem *item = ( FolderListItem* )i;

    for ( Message* msg = item->folder()->firstMessage(); msg;
		msg = item->folder()->nextMessage() )
        (void)new MessageListItem( messages, msg );
}

void ListViews::slotMessageChanged( QListViewItem *i )
{
    if ( !i ) return;

    message->setText( "" );

    MessageListItem *item = ( MessageListItem* )i;
    Message *msg = item->message();

    QString text;
    text.sprintf( "From: %s\n"
                  "Subject: %s\n"
                  "Date: %s\n\n"
                  "%s",
                  msg->header().sender().ascii(), msg->header().subject().ascii(),
                  msg->header().datetime().toString().ascii(), msg->body().ascii() );

    message->setText( text );

    msg->setState( Message::Read );
}
