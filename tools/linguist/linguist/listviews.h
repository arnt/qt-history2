/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LISTVIEWS_H
#define LISTVIEWS_H

#include "msgedit.h"

#include <qstring.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qlistview.h>

class LVI : public QListViewItem
{
public:
    LVI( QListView *parent, QString text = QString::null );
    LVI( QListViewItem *parent, QString text = QString::null );
    virtual int compare( QListViewItem *other, int column,
			 bool ascending ) const;
    virtual bool danger() const { return FALSE; }

protected:
    void drawObsoleteText( QPainter * p, const QColorGroup & cg, int column,
			   int width, int align );

private:
    static int count;
};

class MessageLVI;
class ContextLVI : public LVI
{
public:
    ContextLVI( QListView *lv, const QString& context );

    virtual bool danger() const { return dangerCount > 0; }

    void appendToComment( const QString& x );
    void incrementUnfinishedCount();
    void decrementUnfinishedCount();
    void incrementDangerCount();
    void decrementDangerCount();
    void incrementObsoleteCount();
    bool isContextObsolete();
    void updateStatus();

    QString context() const { return text( 1 ); }
    QString comment() const { return com; }
    QString fullContext() const;
    bool    finished() const { return unfinishedCount == 0; }

    MessageLVI *firstMessageItem() { return messageItems.first(); }
    MessageLVI * takeMessageItem( int i ) { return messageItems.takeAt( i ); }
    QList<MessageLVI*> messageItemLst() { return messageItems; }
    void appendMessageItem( QListView * lv, MessageLVI * i );
    void instantiateMessageItem( QListView * lv, MessageLVI * i );
    int messageItemsInList() { return messageItems.count(); }

    void paintCell( QPainter * p, const QPalette &cg, int column,
		    int width, int align );
private:
    QList<MessageLVI*> messageItems;
    QString com;
    int unfinishedCount;
    int dangerCount;
    int obsoleteCount;
    int itemCount;
};

class MessageLVI : public LVI
{
public:
    MessageLVI( QListView *parent, const MetaTranslatorMessage & message,
		const QString& text, const QString& comment, ContextLVI * c );

    virtual bool danger() const { return d; }

    void setTranslation( const QString& translation );
    void setFinished( bool finished );
    void setDanger( bool danger );

    void setContextLVI( ContextLVI * c ) { ctxt = c; }
    ContextLVI *contextLVI() const { return ctxt; }
    void updateTranslationText();

    QString context() const;
    QString sourceText() const { return tx; }
    QString comment() const { return com; }
    QString translation() const { return m.translation(); }
    bool finished() const { return fini; }
    MetaTranslatorMessage message() const;

    void paintCell( QPainter * p, const QPalette &cg, int column,
		    int width, int align );
private:
    MetaTranslatorMessage m;
    QString tx;
    QString com;
    bool fini;
    bool d;
    ContextLVI * ctxt;
};

#endif
