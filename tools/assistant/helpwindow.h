#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qtextbrowser.h>

class MainWindow;

class HelpWindow : public QTextBrowser
{
public:
    HelpWindow( MainWindow *m, QWidget *parent = 0, const char *name = 0 );
    void setSource( const QString &name );
    QPopupMenu *createPopupMenu();

private:
    MainWindow *mw;
    
};

#endif
