#ifndef HELPMAINWINDOW_H
#define HELPMAINWINDOW_H

#include <qmainwindow.h>

class HelpView;
class HelpNavigation;
class QPopupMenu;
class QShowEvent;
class QProgressBar;
class QLabel;
class QToolButton;
class HelpFindDialog;

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

    void preparePorgress( int );
    void incProcess();
    void finishProgress();

    void forwardAvailable( bool );
    void backwardAvailable( bool );

    void updateViewMenu();

    void findDialogClosed();
    
private:
    QPopupMenu *history;
    QString docDir;
    bool indexCreated;
    QProgressBar *bar;
    QLabel *label;
    QToolButton *forward, *backward;
    int forward_id, backward_id, contents_id, index_id, bookmarks_id, search_id;
    QPopupMenu *go, *view;
    HelpFindDialog *findDialog;
    
};

#endif
