#ifndef EDITOR_H
#define EDITOR_H

#include <qtextedit.h>
#include "dlldefs.h"

class ParenMatcher;
class EditorCompletion;

class CPP_EXPORT Editor : public QTextEdit
{
    Q_OBJECT

public:
    Editor( const QString &fn, QWidget *parent, const char *name );
    void tabify( QString &s );
    void load( const QString &fn );
    void save( const QString &fn );
    QTextDocument *document() const { return QTextEdit::document(); }
    void setDocument( QTextDocument *doc ) { QTextEdit::setDocument( doc ); }
    QTextCursor *textCursor() const { return QTextEdit::textCursor(); }

    virtual EditorCompletion *completionManager() { return completion; }

private slots:
    void cursorPosChanged( QTextCursor *c );

protected:
    ParenMatcher *parenMatcher;
    static EditorCompletion *completion;
    QString filename;

};

#endif
