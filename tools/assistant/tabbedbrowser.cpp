
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
    gui.setupUI(this);

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
    str += "...";
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
    return static_cast<HelpWindow*>(gui.tab->currentWidget());
}

void TabbedBrowser::nextTab()
{
    if(gui.tab->currentIndex()<=gui.tab->count()-1)
        gui.tab->setCurrentPage(gui.tab->currentIndex()+1);
}

void TabbedBrowser::previousTab()
{
    int idx = gui.tab->currentIndex()-1;
    if(idx>=0)
        gui.tab->setCurrentPage(idx);
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
    gui.tab->addTab(win, reduceLabelLength(title));
    connect(win, SIGNAL(highlighted(const QString &)),
             (const QObject*) (mainWin->statusBar()), SLOT(message(const QString &)));
    connect(win, SIGNAL(chooseWebBrowser()), mainWin, SLOT(showWebBrowserSettings()));
    connect(win, SIGNAL(backwardAvailable(bool)),
             mainWin, SLOT(backwardAvailable(bool)));
    connect(win, SIGNAL(forwardAvailable(bool)),
             mainWin, SLOT(forwardAvailable(bool)));
    connect(win, SIGNAL(sourceChanged(const QString &)), this, SLOT(sourceChanged()));

    gui.tab->cornerWidget(Qt::TopRight)->setEnabled(gui.tab->count() > 1);
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
    gui.tab->showPage(win);
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
    while(gui.tab->count()) {
        QWidget *page = gui.tab->widget(0);
        gui.tab->removeTab(0);
        delete page;
    }

    mimeSourceFactory = new QMimeSourceFactory();
    mimeSourceFactory->setExtensionType("html","text/html;charset=UTF-8");
    mimeSourceFactory->setExtensionType("htm","text/html;charset=UTF-8");
    mimeSourceFactory->setExtensionType("png", "image/png");
    mimeSourceFactory->setExtensionType("jpg", "image/jpeg");
    mimeSourceFactory->setExtensionType("jpeg", "image/jpeg");
    setMimePath(Config::configuration()->mimePaths());

    connect(gui.tab, SIGNAL(currentChanged(QWidget*)),
             this, SLOT(transferFocus()));

    QTabBar *tabBar = (QTabBar*)gui.tab->child(0, "QTabBar", false);
    int m = (tabBar ? style().pixelMetric(QStyle::PM_TabBarTabVSpace, (QWidget*)tabBar)
              + style().pixelMetric(QStyle::PM_TabBarBaseHeight, (QWidget*)tabBar) : 0);
    int s = gui.tab->height() - m;

    // workaround for sgi style
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Button, pal.active().background());
    pal.setColor(QPalette::Disabled, QColorGroup::Button, pal.disabled().background());
    pal.setColor(QPalette::Inactive, QColorGroup::Button, pal.inactive().background());

    QToolButton *newTabButton = new QToolButton(this);
    gui.tab->setCornerWidget(newTabButton, Qt::TopLeft);
    newTabButton->setCursor(arrowCursor);
    newTabButton->setAutoRaise(true);
    newTabButton->setIcon(QPixmap::fromMimeSource("addtab.png"));
    newTabButton->setFixedSize(s, s);
    QObject::connect(newTabButton, SIGNAL(clicked()), this, SLOT(newTab()));
    newTabButton->setToolTip(tr("Add page"));

    QToolButton *closeTabButton = new QToolButton(this);
    closeTabButton->setPalette(pal);
    gui.tab->setCornerWidget(closeTabButton, Qt::TopRight);
    closeTabButton->setCursor(arrowCursor);
    closeTabButton->setAutoRaise(true);
    QIconSet is(QPixmap::fromMimeSource("closetab.png"));
    QPixmap disabledPix = QPixmap::fromMimeSource("d_closetab.png");
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
    mimeSourceFactory->setExtensionType("html", ext);
    mimeSourceFactory->setExtensionType("htm", ext);
}

void TabbedBrowser::updateTitle(const QString &title)
{
    gui.tab->setTabText(gui.tab->indexOf(currentBrowser()), title);
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
                             + " - " + currentBrowser()->documentTitle());
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
    tabStyleSheet->item("pre")->setFontFamily(family);
    tabStyleSheet->item("code")->setFontFamily(family);
    tabStyleSheet->item("tt")->setFontFamily(family);

    newTab(QString::null);
}

void TabbedBrowser::setLinkUnderline(bool uline)
{
    if(uline==tabLinkUnderline)
        return;
    tabLinkUnderline = uline;
    int cnt = gui.tab->count();
    for(int i=0; i<cnt; i++)
        ((QTextBrowser*) gui.tab->widget(i))->setLinkUnderline(tabLinkUnderline);
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
    int cnt = gui.tab->count();
    for(int i=0; i<cnt; i++)
        ((QTextBrowser*) gui.tab->widget(i))->setFont(fnt);
}

void TabbedBrowser::setPalette(const QPalette &pal)
{
    if(palette()==pal)
        return;
    QWidget::setPalette(pal);
    int cnt = gui.tab->count();
    for(int i=0; i<cnt; i++)
        ((QTextBrowser*) gui.tab->widget(i))->setPalette(pal);
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
    if(gui.tab->count()==1)
        return;
    HelpWindow *win = currentBrowser();
    gui.tab->removePage(win);
    QTimer::singleShot(0, win, SLOT(deleteLater()));
    gui.tab->cornerWidget(Qt::TopRight)->setEnabled(gui.tab->count() > 1);
}

QStringList TabbedBrowser::sources() const
{
    QStringList lst;
    int cnt = gui.tab->count();
    for(int i=0; i<cnt; i++) {
        lst.append(((QTextBrowser*) gui.tab->widget(i))->source());
    }
    return lst;
}

QList<HelpWindow*> TabbedBrowser::browsers() const
{
    QList<HelpWindow*> list;
    for (int i=0; i<gui.tab->count(); ++i) {
        Q_ASSERT(::qt_cast<HelpWindow*>(gui.tab->widget(i)));
        list.append(static_cast<HelpWindow*>(gui.tab->widget(i)));
    }
    return list;
}

void TabbedBrowser::sourceChanged()
{
    HelpWindow *win = ::qt_cast<HelpWindow *>(QObject::sender());
    Q_ASSERT(win);
    QString docTitle(win->documentTitle());
    if (docTitle.isEmpty())
        docTitle = "...";
    setTitle(win, docTitle);
}

void TabbedBrowser::setTitle(HelpWindow *win, const QString &title)
{
    gui.tab->setTabText(gui.tab->indexOf(win), reduceLabelLength(title));
    if (win == currentBrowser())
        mainWindow()->setWindowTitle(Config::configuration()->title() + " - " + title);
}

