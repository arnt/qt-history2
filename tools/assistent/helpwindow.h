#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qtextbrowser.h>

class HelpWindow : public QTextBrowser
{
public:
    HelpWindow( QWidget *parent = 0, const char *name = 0 );
    void setSource( const QString &name );

};

#endif
