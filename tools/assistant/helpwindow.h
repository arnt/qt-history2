#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qtextbrowser.h>

class MainWindow;

class HelpWindow : public QTextBrowser
{
    Q_OBJECT
public:
    HelpWindow( MainWindow *m, QWidget *parent = 0, const char *name = 0 );
    void setSource( const QString &name );
    QPopupMenu *createPopupMenu( const QPoint& pos );
    void blockScrolling( bool b );

protected:
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );

protected slots:
    void ensureCursorVisible();

private slots:
    void openLinkInNewWindow();

private:
    MainWindow *mw;
    bool shiftPressed;
    QString lastAnchor;
    bool blockScroll;

};

#endif
