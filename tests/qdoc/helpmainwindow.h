#ifndef HELPMAINWINDOW_H
#define HELPMAINWINDOW_H

#include <qmainwindow.h>

class HelpView;
class HelpNavigation;
class QPopupMenu;
class QShowEvent;

class HelpMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    HelpMainWindow();
    ~HelpMainWindow();

public slots:
    void addBookmark();
    void removeBookmark();

protected:
    void showEvent( QShowEvent *e );

private:
    HelpView *viewer;
    HelpNavigation *navigation;

private slots:
    void slotFilePrint();
    void slotEditCopy();
    void slotEditSelectAll();
    void slotEditFind();
    void slotViewContents();
    void slotViewIndex();
    void slotViewSearch();
    void slotViewBookmarks();
    void slotGoBack();
    void slotGoForward();
    void slotGoHome();
    void slotHelpAbout();
    void slotHelpAboutQt();

    void newSource( const QString &name );
    void moveFocusToBrowser();
    void createDatabase();
    void setupHistoryMenu();
    void showFromHistory( int id );
    
private:
    QPopupMenu *history;
    QString docDir;

};

#endif
