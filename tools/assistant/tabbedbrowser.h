/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
    QStringList sources() const;
    QFont browserFont() const;
    QList<HelpWindow*> browsers() const;

    HelpWindow* newBackgroundTab(const QString &url);
    HelpWindow* createHelpWindow(const QString &);

    void setBrowserFont(const QFont &fnt);
    void setTitle(HelpWindow*, const QString &);
    void applySettings();

    
    void setFixedFontFamily(const QString &family) { fixedFontFam = family; }
    QString fixedFontFamily() const { return fixedFontFam; }

    void setLinkColor(const QColor &col) { lnkColor = col; }
    QColor linkColor() const { return lnkColor; }

    void setUnderlineLink(bool udrline) { underlineLnk = udrline; }
    bool underlineLink() const { return underlineLnk; }


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
    void copy();
    void closeTab();
    void sourceChanged();

private:
    Ui::TabbedBrowser ui;
    QWidget *lastCurrentTab;
    QFont tabFont;

    QString fixedFontFam;
    QColor lnkColor;
    bool underlineLnk;
};

#endif // TABBEDBROWSER_H
