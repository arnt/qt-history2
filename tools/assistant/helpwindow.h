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

protected:
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );

private slots:
    void openLinkInNewWindow();
    
private:
    MainWindow *mw;
    bool shiftPressed;
    QString lastAnchor;

};

#endif
