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
    virtual void load( const QString &fn );
    virtual void save( const QString &fn );
    QTextDocument *document() const { return QTextEdit::document(); }
    void setDocument( QTextDocument *doc ) { QTextEdit::setDocument( doc ); }
    QTextCursor *textCursor() const { return QTextEdit::textCursor(); }

    virtual EditorCompletion *completionManager() { return 0; }

private slots:
    void cursorPosChanged( QTextCursor *c );

protected:
    ParenMatcher *parenMatcher;
    QString filename;

};

#endif
