#ifndef TABBEDBROWSER_H
#define TABBEDBROWSER_H

#include "ui_tabbedbrowser.h"

class MainWindow;
class HelpWindow;
class QStyleSheet;
class QMimeSourceFactory;

class TabbedBrowser : public QWidget
{
    Q_OBJECT
public:
    TabbedBrowser(MainWindow *parent);
    virtual ~TabbedBrowser();

    MainWindow *mainWindow() const;
    HelpWindow *currentBrowser() const;
    QStyleSheet *styleSheet() const;
    bool linkUnderline() const;
    QStringList sources() const;
    QFont browserFont() const;
    QList<HelpWindow*> browsers() const;

    HelpWindow* newBackgroundTab(const QString &url);
    HelpWindow* createHelpWindow(const QString &);

    void setMimePath(QStringList lst);
    void setMimeExtension(const QString &ext);
    void setBrowserFont(const QFont &fnt);
    void setTitle(HelpWindow*, const QString &);

public slots:
    void init();
    void forward();
    void backward();
    void setSource(const QString &ref);
    void reload();
    void home();
    void nextTab();
    void previousTab();
    void newTab(const QString &lnk);
    void zoomIn();
    void zoomOut();
    void updateTitle(const QString &title);
    void newTab();
    void transferFocus();
    void initHelpWindow(HelpWindow *win);
    void setup();
    void setLinkUnderline(bool uline);
    void setPalette(const QPalette &pal);
    void copy();
    void closeTab();
    void sourceChanged();

private:
    Ui::TabbedBrowser ui;
    QStyleSheet *tabStyleSheet;
    bool tabLinkUnderline;
    QMimeSourceFactory *mimeSourceFactory;
    QWidget *lastCurrentTab;
    QFont tabFont;
};

#endif // TABBEDBROWSER_H
