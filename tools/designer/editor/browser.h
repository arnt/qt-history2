#ifndef BROWSER_H
#define BROWSER_H

#include <qobject.h>
#include "dlldefs.h"

class Editor;
class QTextCursor;
class QTextParag;
class QTextFormat;

class EDITOR_EXPORT EditorBrowser : public QObject
{
    Q_OBJECT

public:
    EditorBrowser( Editor *e );
    ~EditorBrowser();

    bool eventFilter( QObject *o, QEvent *e );
    virtual void setCurrentEdior( Editor *e );
    virtual void addEditor( Editor *e );
    virtual bool findCursor( const QTextCursor &c, QTextCursor &from, QTextCursor &to );
    virtual void showHelp( const QString & ) {}

protected:
    Editor *curEditor;
    QTextParag *oldHighlightedParag;
    QString lastWord;
    QTextFormat *highlightedFormat;

};

#endif
