#ifndef CPPBROWSER_H
#define CPPBROWSER_H

#include <browser.h>

class CppEditorBrowser : public EditorBrowser
{
    Q_OBJECT

public:
    CppEditorBrowser( Editor *e );
    bool findCursor( const QTextCursor &c, QTextCursor &from, QTextCursor &to );
    void showHelp( const QString &word );

};

#endif
