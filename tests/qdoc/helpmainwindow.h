#ifndef HELPMAINWINDOW_H
#define HELPMAINWINDOW_H

#include <qmainwindow.h>

class HelpView;
class HelpNavigation;
class QPopupMenu;

class HelpMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    HelpMainWindow();
    
private:
    HelpView *view;
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
    
private:
    QPopupMenu *history;
    
};

#endif
