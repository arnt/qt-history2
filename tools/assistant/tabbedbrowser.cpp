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


#include "tabbedbrowser.h"
#include "mainwindow.h"
#include "helpwindow.h"
#include "config.h"

#include <qtooltip.h>
#include <qfileinfo.h>
#include <qtoolbutton.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <qstyle.h>
#include <qtimer.h>

TabbedBrowser::TabbedBrowser(MainWindow *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    init();
}

TabbedBrowser::~TabbedBrowser()
{
    delete mimeSourceFactory;
    mimeSourceFactory = 0;
}

MainWindow *TabbedBrowser::mainWindow() const
{
    return static_cast<MainWindow*>(parentWidget());
}

static QString reduceLabelLength( const QString &s )
{
    int maxLength = 16;
    QString str = s;
    if ( str.length() < maxLength )
        return str;
    str = str.left( maxLength - 3 );
    str += QLatin1String("...");
    return str;
}

void TabbedBrowser::forward()
{
    currentBrowser()->forward();
}

void TabbedBrowser::backward()
{
    currentBrowser()->backward();
}

void TabbedBrowser::setSource( const QString &ref )
{
    HelpWindow * win = currentBrowser();
    win->setSource(ref);
}

void TabbedBrowser::reload()
{
    currentBrowser()->reload();
}

void TabbedBrowser::home()
{
    currentBrowser()->home();
}

HelpWindow *TabbedBrowser::currentBrowser() const
{
    return static_cast<HelpWindow*>(ui.tab->currentWidget());
}

void TabbedBrowser::nextTab()
{
    if(ui.tab->currentIndex()<=ui.tab->count()-1)
        ui.tab->setCurrentPage(ui.tab->currentIndex()+1);
}

void TabbedBrowser::previousTab()
{
    int idx = ui.tab->currentIndex()-1;
    if(idx>=0)
        ui.tab->setCurrentPage(idx);
}

HelpWindow *TabbedBrowser::createHelpWindow(const QString &title)
{
    MainWindow *mainWin = mainWindow();
    HelpWindow *win = new HelpWindow(mainWin, 0, "qt_assistant_helpwin");
    win->setFont(browserFont());
    win->setPalette(palette());
    win->setLinkUnderline(tabLinkUnderline);
    win->setStyleSheet(tabStyleSheet);
    win->setMimeSourceFactory(mimeSourceFactory);
    ui.tab->addTab(win, reduceLabelLength(title));
    connect(win, SIGNAL(highlighted(const QString &)),
             (const QObject*) (mainWin->statusBar()), SLOT(message(const QString &)));
    connect(win, SIGNAL(chooseWebBrowser()), mainWin, SLOT(showWebBrowserSettings()));
    connect(win, SIGNAL(backwardAvailable(bool)),
             mainWin, SLOT(backwardAvailable(bool)));
    connect(win, SIGNAL(forwardAvailable(bool)),
             mainWin, SLOT(forwardAvailable(bool)));
    connect(win, SIGNAL(sourceChanged(const QString &)), this, SLOT(sourceChanged()));

    ui.tab->cornerWidget(Qt::TopRight)->setEnabled(ui.tab->count() > 1);
    return win;
}

HelpWindow *TabbedBrowser::newBackgroundTab(const QString &url)
{
    HelpWindow *win = createHelpWindow(url);
    return win;
}

void TabbedBrowser::newTab(const QString &lnk)
{
    QString link(lnk);
    if(link.isNull()) {
        HelpWindow *w = currentBrowser();
        if(w)
            link = w->source();
    }
    HelpWindow *win = createHelpWindow(link);
    ui.tab->showPage(win);
    if(!link.isNull()) {
         win->setSource(link);
    }
}

void TabbedBrowser::zoomIn()
{
    currentBrowser()->zoomIn();
}

void TabbedBrowser::zoomOut()
{
    currentBrowser()->zoomOut();
}

void TabbedBrowser::init()
{
    tabLinkUnderline = false;
    tabStyleSheet = new QStyleSheet(QStyleSheet::defaultSheet());
    lastCurrentTab = 0;
    while(ui.tab->count()) {
        QWidget *page = ui.tab->widget(0);
        ui.tab->removeTab(0);
        delete page;
    }

    mimeSourceFactory = new QMimeSourceFactory();
    mimeSourceFactory->setExtensionType(QLatin1String("html"), "text/html;charset=UTF-8");
    mimeSourceFactory->setExtensionType(QLatin1String("htm"), "text/html;charset=UTF-8");
    mimeSourceFactory->setExtensionType(QLatin1String("png"), "image/png");
    mimeSourceFactory->setExtensionType(QLatin1String("jpg"), "image/jpeg");
    mimeSourceFactory->setExtensionType(QLatin1String("jpeg"), "image/jpeg");
    setMimePath(Config::configuration()->mimePaths());

    connect(ui.tab, SIGNAL(currentChanged(QWidget*)),
             this, SLOT(transferFocus()));

    QTabBar *tabBar = qFindChild<QTabBar*>(ui.tab);

    int m = (tabBar ? style().pixelMetric(QStyle::PM_TabBarTabVSpace, (QWidget*)tabBar)
              + style().pixelMetric(QStyle::PM_TabBarBaseHeight, (QWidget*)tabBar) : 0);
    int s = ui.tab->height() - m;

    // workaround for sgi style
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QPalette::Button, pal.color(QPalette::Active, QPalette::Background));
    pal.setColor(QPalette::Disabled, QPalette::Button, pal.color(QPalette::Disabled, QPalette::Background));
    pal.setColor(QPalette::Inactive, QPalette::Button, pal.color(QPalette::Inactive, QPalette::Background));

    QToolButton *newTabButton = new QToolButton(this);
    ui.tab->setCornerWidget(newTabButton, Qt::TopLeft);
    newTabButton->setCursor(Qt::arrowCursor);
    newTabButton->setAutoRaise(true);
    newTabButton->setIcon(QPixmap(QString::fromUtf8(":/trolltech/assistant/images/addtab.png")));
    newTabButton->setFixedSize(s, s);
    QObject::connect(newTabButton, SIGNAL(clicked()), this, SLOT(newTab()));
    newTabButton->setToolTip(tr("Add page"));

    QToolButton *closeTabButton = new QToolButton(this);
    closeTabButton->setPalette(pal);
    ui.tab->setCornerWidget(closeTabButton, Qt::TopRight);
    closeTabButton->setCursor(Qt::arrowCursor);
    closeTabButton->setAutoRaise(true);
    QIconSet is(QString::fromUtf8(":/trolltech/assistant/images/closetab.png"));
    QPixmap disabledPix = QString::fromUtf8(":/trolltech/assistant/images/d_closetab.png");
    is.setPixmap(disabledPix, QIconSet::Small, QIconSet::Disabled);
    closeTabButton->setIcon(is);
    closeTabButton->setFixedSize(s, s);
    QObject::connect(closeTabButton, SIGNAL(clicked()), this, SLOT(closeTab()));
    closeTabButton->setToolTip(tr("Close page"));
    closeTabButton->setEnabled(false);
}

void TabbedBrowser::setMimePath(QStringList lst)
{
    mimeSourceFactory->setFilePath(lst);
}

void TabbedBrowser::setMimeExtension(const QString &ext)
{
    mimeSourceFactory->setExtensionType(QLatin1String("html"), ext);
    mimeSourceFactory->setExtensionType(QLatin1String("htm"), ext);
}

void TabbedBrowser::updateTitle(const QString &title)
{
    ui.tab->setTabText(ui.tab->indexOf(currentBrowser()), title);
}

void TabbedBrowser::newTab()
{
    newTab(QString::null);
}

void TabbedBrowser::transferFocus()
{
    if(currentBrowser()) {
        currentBrowser()->setFocus();
    }
    mainWindow()->setWindowTitle(Config::configuration()->title()
                             + QLatin1String(" - ") 
                             + currentBrowser()->documentTitle());
}

void TabbedBrowser::initHelpWindow(HelpWindow * /*win*/)
{
}

void TabbedBrowser::setup()
{
    Config *config = Config::configuration();

    QFont fnt(font());
    QFontInfo fntInfo(fnt);
    fnt.setFamily(config->fontFamily());
    if (config->fontSize() > 0)
        fnt.setPointSize(config->fontSize());
    setBrowserFont(fnt);

    QPalette pal = palette();
    QColor lc(config->linkColor());
    pal.setColor(QPalette::Active, QColorGroup::Link, lc);
    pal.setColor(QPalette::Inactive, QColorGroup::Link, lc);
    pal.setColor(QPalette::Disabled, QColorGroup::Link, lc);
    setPalette(pal);

    tabLinkUnderline = config->isLinkUnderline();

    QString family = config->fontFixedFamily();
    tabStyleSheet->item(QLatin1String("pre"))->setFontFamily(family);
    tabStyleSheet->item(QLatin1String("code"))->setFontFamily(family);
    tabStyleSheet->item(QLatin1String("tt"))->setFontFamily(family);

    newTab(QString::null);
}

void TabbedBrowser::setLinkUnderline(bool uline)
{
    if(uline==tabLinkUnderline)
        return;
    tabLinkUnderline = uline;
    int cnt = ui.tab->count();
    for(int i=0; i<cnt; i++)
        ((QTextBrowser*) ui.tab->widget(i))->setLinkUnderline(tabLinkUnderline);
}

QFont TabbedBrowser::browserFont() const
{
    return tabFont;
}

void TabbedBrowser::setBrowserFont(const QFont &fnt)
{
    if(tabFont == fnt)
        return;
    tabFont = fnt;
    int cnt = ui.tab->count();
    for(int i=0; i<cnt; i++)
        ((QTextBrowser*) ui.tab->widget(i))->setFont(fnt);
}

void TabbedBrowser::setPalette(const QPalette &pal)
{
    if(palette()==pal)
        return;
    QWidget::setPalette(pal);
    int cnt = ui.tab->count();
    for(int i=0; i<cnt; i++)
        ((QTextBrowser*) ui.tab->widget(i))->setPalette(pal);
}

QStyleSheet* TabbedBrowser::styleSheet() const
{
    return tabStyleSheet;
}

bool TabbedBrowser::linkUnderline() const
{
    return tabLinkUnderline;
}

void TabbedBrowser::copy()
{
    currentBrowser()->copy();
}

void TabbedBrowser::closeTab()
{
    if(ui.tab->count()==1)
        return;
    HelpWindow *win = currentBrowser();
    ui.tab->removePage(win);
    QTimer::singleShot(0, win, SLOT(deleteLater()));
    ui.tab->cornerWidget(Qt::TopRight)->setEnabled(ui.tab->count() > 1);
}

QStringList TabbedBrowser::sources() const
{
    QStringList lst;
    int cnt = ui.tab->count();
    for(int i=0; i<cnt; i++) {
        lst.append(((QTextBrowser*) ui.tab->widget(i))->source());
    }
    return lst;
}

QList<HelpWindow*> TabbedBrowser::browsers() const
{
    QList<HelpWindow*> list;
    for (int i=0; i<ui.tab->count(); ++i) {
        Q_ASSERT(::qt_cast<HelpWindow*>(ui.tab->widget(i)));
        list.append(static_cast<HelpWindow*>(ui.tab->widget(i)));
    }
    return list;
}

void TabbedBrowser::sourceChanged()
{
    HelpWindow *win = ::qt_cast<HelpWindow *>(QObject::sender());
    Q_ASSERT(win);
    QString docTitle(win->documentTitle());
    if (docTitle.isEmpty())
        docTitle = QLatin1String("...");
    setTitle(win, docTitle);
}

void TabbedBrowser::setTitle(HelpWindow *win, const QString &title)
{
    ui.tab->setTabText(ui.tab->indexOf(win), reduceLabelLength(title));
    if (win == currentBrowser())
        mainWindow()->setWindowTitle(Config::configuration()->title() + QLatin1String(" - ") + title);
}

