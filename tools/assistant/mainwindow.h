#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <qpointer.h>
#include <qmap.h>

class TabbedBrowser;
class HelpDialog;
class FindDialog;
class SettingsDialog;
class HelpWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    virtual ~MainWindow();

    TabbedBrowser *browsers() const;
    HelpDialog *helpDialog() const;

    void setupPopupMenu(QPopupMenu *menu);

    virtual bool close();

public slots:
    MainWindow *newWindow();

    void hide();
    void setup();
    void showLink(const QString &link);
    void showLinks(const QStringList &links);
    void saveToolbarSettings();
    void saveSettings();
    void updateBookmarkMenu();

private slots:
    void on_actionGoHome_triggered();
    void on_actionEditFind_triggered();
    void on_actionEditFindAgain_triggered();
    void on_actionEditFindAgainPrev_triggered();
    void on_actionFilePrint_triggered();
    void on_actionSettings_triggered();
    void on_actionClose_triggered();
    void on_actionHelpWhatsThis_triggered();
    void on_actionHelpAssistant_triggered();
    void on_actionAboutApplication_triggered();

    void about();
    void setupBookmarkMenu();
    void showBookmark(QAction *action);
    void showLinkFromClient(const QString &link);
    void showQtHelp();
    void showWebBrowserSettings();
    void showSettingsDialog(int page);
    void showSearchLink(const QString &link, const QStringList &terms);
    void showGoActionLink();
    void updateProfileSettings();
    void backwardAvailable(bool);
    void forwardAvailable(bool);

protected:
    void timerEvent(QTimerEvent *);

private:
    void setupGoActions();
    bool insertActionSeparator();

private:
    Ui::MainWindow gui;

    QList<QAction*> goActions;
    int setupCompleted:1;
    TabbedBrowser *tabs;
    SettingsDialog *settingsDia;
    QMap<QAction*, QString> bookmarks;
    HelpDialog *helpDock;
    QDockWindow *dw;
    QPointer<FindDialog> findDialog;
    static QList<MainWindow*> windows;
    QMap<QAction*,QString> *goActionDocFiles;
    QStringList pendingLinks;
    QList<HelpWindow*> pendingBrowsers;
};

#endif // MAINWINDOW_H
