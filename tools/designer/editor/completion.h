/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef COMPLETION_H
#define COMPLETION_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qlist.h>
#include <qmap.h>

class Q3TextDocument;
class Editor;
class QVBox;
class QListBox;
class ArgHintWidget;

struct CompletionEntry
{
    QString type;
    QString text;
    QString postfix;
    QString prefix;
    QString postfix2;

    bool operator==( const CompletionEntry &c ) const {
	return ( c.type == type &&
		 c.text == text &&
		 c.postfix == postfix &&
		 c.prefix == prefix &&
		 c.postfix2 == postfix2 );
    }
};

class EditorCompletion : public QObject
{
    Q_OBJECT

public:
    EditorCompletion( Editor *e );
    ~EditorCompletion();

    virtual void addCompletionEntry( const QString &s, Q3TextDocument *doc, bool strict );
    virtual QList<CompletionEntry> completionList( const QString &s, Q3TextDocument *doc ) const;
    virtual void updateCompletionMap( Q3TextDocument *doc );

    bool eventFilter( QObject *o, QEvent *e );
    virtual void setCurrentEdior( Editor *e );
    virtual bool doCompletion();
    virtual bool doObjectCompletion();
    virtual bool doObjectCompletion( const QString &object );
    virtual bool doArgumentHint( bool useIndex );

    virtual void addEditor( Editor *e );
    virtual QList<QStringList> functionParameters( const QString &func, QChar &, QString &prefix, QString &postfix );

    virtual void setContext( QObject *this_ );

    void setEnabled( bool b ) { enabled = b; }

protected:
    virtual bool continueComplete();
    virtual void showCompletion( const QList<CompletionEntry> &lst );
    virtual void completeCompletion();

protected:
    QVBox *completionPopup;
    QListBox *completionListBox;
    ArgHintWidget *functionLabel;
    int completionOffset;
    Editor *curEditor;
    QString searchString;
    QList<CompletionEntry> cList;
    QMap<QChar, QStringList> completionMap;
    bool enabled;
    Q3TextDocument *lastDoc;

};

#endif
