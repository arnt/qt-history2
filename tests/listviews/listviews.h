/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef LISTVIEWS_H
#define LISTVIEWS_H

#include <qsplitter.h>
#include <qstring.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qlistview.h>

class QListView;
class QLabel;
class QPainter;
class QColorGroup;
class QObjectList;
class QPopupMenu;

// -----------------------------------------------------------------

class MessageHeader
{
public:
    MessageHeader( const QString &_sender, const QString &_subject, const QDateTime &_datetime )
	: msender( _sender ), msubject( _subject ), mdatetime( _datetime )
    {}

    MessageHeader( const MessageHeader &mh );
    MessageHeader &operator=( const MessageHeader &mh );

    QString sender() { return msender; }
    QString subject() { return msubject; }
    QDateTime datetime() { return mdatetime; }

protected:
    QString msender, msubject;
    QDateTime mdatetime;

};

// -----------------------------------------------------------------

class Message
{
public:
    enum State { Read = 0,
		 Unread};

    Message( const MessageHeader &mh, const QString &_body )
	: mheader( mh ), mbody( _body ), mstate( Unread )
    {}

    Message( const Message &m )
	: mheader( m.mheader ), mbody( m.mbody ), mstate( m.mstate )
    {}

    MessageHeader header() { return mheader; }
    QString body() { return mbody; }

    void setState( const State &s ) { mstate = s; }
    State state() { return mstate; }

protected:
    MessageHeader mheader;
    QString mbody;
    State mstate;

};

// -----------------------------------------------------------------

class Folder : public QObject
{
    Q_OBJECT

public:
    Folder( Folder *parent, const QString &name );
    ~Folder()
    {}

    void addMessage( Message *m )
    { lstMessages.append( m ); }

    QString folderName() { return fName; }

    Message *firstMessage() { return lstMessages.first(); }
    Message *nextMessage() { return lstMessages.next(); }

protected:
    QString fName;
    QPtrList<Message> lstMessages;

};

// -----------------------------------------------------------------

class FolderListItem : public QListViewItem
{
public:
    FolderListItem( QListView *parent, Folder *f );
    FolderListItem( FolderListItem *parent, Folder *f );

    void insertSubFolders( const QObjectList *lst );

    Folder *folder() { return myFolder; }

protected:
    Folder *myFolder;

};

// -----------------------------------------------------------------

class MessageListItem : public QListViewItem
{
public:
    MessageListItem( QListView *parent, Message *m );

    virtual void paintCell( QPainter *p, const QColorGroup &cg,
			    int column, int width, int alignment );

    Message *message() { return myMessage; }

protected:
    Message *myMessage;

};

// -----------------------------------------------------------------

class ListViews : public QSplitter
{
    Q_OBJECT

public:
    ListViews( QWidget *parent = 0, const char *name = 0 );
    ~ListViews()
    {}

protected:
    void initFolders();
    void initFolder( Folder *folder, unsigned int &count );
    void setupFolders();

    QListView *messages, *folders;
    QLabel *message;
    QPopupMenu* menu;

    QPtrList<Folder> lstFolders;

protected slots:
    void slotFolderChanged( QListViewItem* );
    void slotMessageChanged( QListViewItem* );
    void slotRMB( QListViewItem*, const QPoint &, int );

    void selectionChanged() {
	qDebug( "selectionChanged" );
    }
    void selectionChanged( QListViewItem *i ) {
	if ( !i )
	    return;
	qDebug( "selectionChanged %p %s", i, i->text( 0 ).latin1() );
    }
    void currentChanged( QListViewItem *i ) {
	if ( !i )
	    return;
	qDebug( "currentChanged %p %s", i, i->text( 0 ).latin1() );
    }
    void clicked( QListViewItem *i ) {
	if ( !i )
	    return;
	qDebug( "clicked %p %s", i, i->text( 0 ).latin1() );
    }
    void pressed( QListViewItem *i ) {
	if ( !i )
	    return;
	qDebug( "pressed %p %s", i, i->text( 0 ).latin1() );
    }
    void doubleClicked( QListViewItem *i ) {
	if ( !i )
	    return;
	qDebug( "doubleClicked %p %s", i, i->text( 0 ).latin1() );
    }
    void returnPressed( QListViewItem *i ) {
	if ( !i )
	    return;
	qDebug( "returnPressed %p %s", i, i->text( 0 ).latin1() );
    }
    void rightButtonClicked( QListViewItem *i, const QPoint&, int ) {
	if ( !i )
	    return;
	qDebug( "rightButtonClicked %p %s", i, i->text( 0 ).latin1() );
    }
    void rightButtonPressed( QListViewItem *i, const QPoint&, int ) {
	if ( !i )
	    return;
	qDebug( "rightButtonPressed %p %s", i, i->text( 0 ).latin1() );
    }
    void mouseButtonPressed( int, QListViewItem *i, const QPoint& , int ) {
	if ( !i )
	    return;
	qDebug( "mouseButtonPressed %p %s", i, i->text( 0 ).latin1() );
    }
    void mouseButtonClicked( int, QListViewItem *i,  const QPoint&, int ) {
	if ( !i )
	    return;
	qDebug( "mouseButtonClicked %p %s", i, i->text( 0 ).latin1() );
    }
    void onItem( QListViewItem *i ) {
	if ( !i )
	    return;
	qDebug( "onItem %p %s", i, i->text( 0 ).latin1() );
    }
    void onViewport() {
	qDebug( "onViewport" );
    }
    void expanded( QListViewItem *i ) {
	if ( !i )
	    return;
	qDebug( "expanded %p %s", i, i->text( 0 ).latin1() );
    }
    void collapsed( QListViewItem *i ) {
	if ( !i )
	    return;
	qDebug( "collapsed %p %s", i, i->text( 0 ).latin1() );
    }
    void remove() {
	if ( folders->currentItem() && folders->currentItem()->isSelectable() )
	    delete folders->currentItem();
    }
    void clear() {
	folders->selectAll( FALSE );
    }
    void current() {
	folders->setCurrentItem( folders->firstChild()->nextSibling() );
    }
    void select() {
	//folders->setSelected( folders->firstChild()->nextSibling(), TRUE );
	if ( folders->selectedItem() )
	    qDebug( "%s", folders->selectedItem()->text( 0 ).latin1() );
	else
	    qDebug( "nono" );
    }
    
};

#endif

