#ifndef COMPLETION_H
#define COMPLETION_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qmap.h>
#include "dlldefs.h"

class QTextDocument;
class Editor;
class QVBox;
class QListBox;
class QLabel;

class CPP_EXPORT EditorCompletion : public QObject
{
    Q_OBJECT

public:
    EditorCompletion( Editor *e );

    virtual void addCompletionEntry( const QString &s, QTextDocument *doc );
    virtual QStringList completionList( const QString &s, QTextDocument *doc ) const;
    virtual void updateCompletionMap( QTextDocument *doc );

    bool eventFilter( QObject *o, QEvent *e );
    virtual void setCurrentEdior( Editor *e );
    virtual bool doCompletion();
    virtual bool doObjectCompletion();
    virtual bool doObjectCompletion( const QString &object );
    virtual bool doArgumentHint( bool useIndex );

    virtual void addEditor( Editor *e );
    virtual QStringList functionParameters( const QString &func, QChar & );

    virtual void setContext( QObjectList *toplevels, QObject *this_ );

protected:
    virtual bool continueComplete();

protected:
    QVBox *completionPopup;
    QListBox *completionListBox;
    QLabel *functionLabel;
    int completionOffset;
    Editor *curEditor;
    QString searchString;
    QStringList cList;
    QMap<QChar, QStringList> completionMap;

};

#endif
