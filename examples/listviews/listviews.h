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

#ifndef LISTVIEWS_H
#define LISTVIEWS_H

#include <qsplitter.h>
#include <qstring.h>
#include <qobject.h>
#include <qlist.h>
#include <qdatetime.h>

#include <qlistview.h>

class QListView;
class QLabel;
class QPainter;
class QColorGroup;
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
    { lstIt = lstMessages.begin(); }

    void addMessage( Message *m ) { lstMessages.append( m ); }

    QString folderName() { return fName; }

    Message *firstMessage() { lstIt = lstMessages.begin(); return *lstIt; }
    Message *nextMessage() { if(lstIt == lstMessages.end()) return 0; ++lstIt; return *lstIt; }

protected:
    QString fName;
    QList<Message*>::Iterator lstIt;
    QList<Message*> lstMessages;

};

// -----------------------------------------------------------------

class FolderListItem : public QListViewItem
{
public:
    FolderListItem( QListView *parent, Folder *f );
    FolderListItem( FolderListItem *parent, Folder *f );

    void insertSubFolders( const QObjectList &lst );

    Folder *folder() { return myFolder; }

protected:
    Folder *myFolder;

};

// -----------------------------------------------------------------

class MessageListItem : public QListViewItem
{
public:
    MessageListItem( QListView *parent, Message *m );

    virtual void paintCell( QPainter *p, const QPalette &pal,
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

    QList<Folder*> lstFolders;

protected slots:
    void slotFolderChanged( QListViewItem* );
    void slotMessageChanged();
    void slotRMB( QListViewItem*, const QPoint &, int );

};

#endif
