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

protected:
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );

private:
    MainWindow *mw;
    bool shiftPressed;

};

#endif
