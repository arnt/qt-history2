/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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

#include <q3listview.h>

class Q3ListView;
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
    ~Folder();

    void addMessage( Message *m ) { lstMessages.append( m ); }

    QString folderName() { return fName; }

    Message *firstMessage() { lstIt = lstMessages.begin(); return *lstIt; }
    Message *nextMessage() { ++lstIt; if(lstIt == lstMessages.end()) return 0;  return *lstIt; }

protected:
    QString fName;
    QList<Message*>::Iterator lstIt;
    QList<Message*> lstMessages;

};

// -----------------------------------------------------------------

class FolderListItem : public Q3ListViewItem
{
public:
    FolderListItem( Q3ListView *parent, Folder *f );
    FolderListItem( FolderListItem *parent, Folder *f );

    void insertSubFolders( const QObjectList &lst );

    Folder *folder() { return myFolder; }

protected:
    Folder *myFolder;

};

// -----------------------------------------------------------------

class MessageListItem : public Q3ListViewItem
{
public:
    MessageListItem( Q3ListView *parent, Message *m );

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
    ~ListViews();

protected:
    void initFolders();
    void initFolder( Folder *folder, unsigned int &count );
    void setupFolders();

    Q3ListView *messages, *folders;
    QLabel *message;
    QPopupMenu* menu;

    QList<Folder*> lstFolders;

protected slots:
    void slotFolderChanged( Q3ListViewItem* );
    void slotMessageChanged();
    void slotRMB( Q3ListViewItem*, const QPoint &, int );

};

#endif
