
#include "mainwindow.h"
#include "tabbedbrowser.h"
#include "helpdialog.h"
#include "finddialog.h"
#include "settingsdialog.h"
#include "config.h"

#include <qdockwindow.h>

#include <qdir.h>
#include <qtimer.h>
#include <qstatusbar.h>
#include <qaccel.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qeventloop.h>
#include <qsimplerichtext.h>
#include <qpaintdevicemetrics.h>
#include <qfontdatabase.h>

#if 0 // ### enable me
#include <qprinter.h>
#endif

QList<MainWindow*> MainWindow::windows;

#if defined(Q_WS_WIN)
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

MainWindow::MainWindow()
{
    ui.setupUi(this);

#if defined(Q_WS_WIN)
    // Workaround for QMimeSourceFactory failing in QFileInfo::isReadable() for
    // certain user configs. See task: 34372
    qt_ntfs_permission_lookup = 0;
#endif
    setupCompleted = FALSE;

    goActions = QList<QAction*>();
    goActionDocFiles = new QMap<QAction*,QString>;

    windows.append(this);
    tabs = new TabbedBrowser(this);
    setCenterWidget(tabs);
    settingsDia = 0;

    Config *config = Config::configuration();

    updateProfileSettings();

    dw = new QDockWindow(this);
    helpDock = new HelpDialog(dw, this);

    dw->setAllowedAreas(AllDockWindowAreas);
    dw->setClosable(true);
    dw->setMovable(true);
    dw->setFloatable(true);
    dw->setCurrentArea(DockWindowAreaLeft);
    dw->setWindowTitle(tr("Sidebar"));

/*
    dw->setResizeEnabled(true);
    dw->setCloseMode(QDockWindow::Always);
    addDockWindow(dw, DockLeft);
    dw->setWidget(helpDock);
    dw->setWindowTitle("Sidebar");
    dw->setFixedExtentWidth(320);
*/
    // read geometry configuration
    setupGoActions();

    if (!config->isMaximized()) {
        QRect geom = config->geometry();
        if(geom.isValid()) {
            resize(geom.size());
            move(geom.topLeft());
        }
    }

    QString mainWindowLayout = config->mainWindowLayout();

#if 0 // ### port me
    QTextStream ts(&mainWindowLayout, IO_ReadOnly);
    ts >> *this;
#endif

    if (config->sideBarHidden())
        dw->hide();

    tabs->setup();
    QTimer::singleShot(0, this, SLOT(setup()));
#if defined(Q_OS_MACX)
    // Use the same forward and backward browser shortcuts as Safari and Internet Explorer do
    // on the Mac. This means that if you have access to one of those cool Intellimice, the thing
    // works just fine, since that's how Microsoft hacked it.
    ui.actionGoPrevious->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Left));
    ui.actionGoNext->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Right));
#endif
}

MainWindow::~MainWindow()
{
    windows.removeAll(this);
    delete goActionDocFiles;
}

void MainWindow::setup()
{
    if(setupCompleted)
        return;

    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    statusBar()->message(tr("Initializing Qt Assistant..."));
    setupCompleted = true;
    helpDock->initialize();
    connect(ui.actionGoPrevious, SIGNAL(triggered()), tabs, SLOT(backward()));
    connect(ui.actionGoNext, SIGNAL(triggered()), tabs, SLOT(forward()));
    connect(ui.actionEditCopy, SIGNAL(triggered()), tabs, SLOT(copy()));
    connect(ui.actionFileExit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
    connect(ui.actionAddBookmark, SIGNAL(triggered()),
             helpDock, SLOT(addBookmark()));
    connect(helpDock, SIGNAL(showLink(const QString&)),
             this, SLOT(showLink(const QString&)));
    connect(helpDock, SIGNAL(showSearchLink(const QString&, const QStringList&)),
             this, SLOT(showSearchLink(const QString&, const QStringList&)));

    connect(ui.bookmarkMenu, SIGNAL(activated(QAction*)),
             this, SLOT(showBookmark(QAction*)));
    connect(ui.actionZoomIn, SIGNAL(triggered()), tabs, SLOT(zoomIn()));
    connect(ui.actionZoomOut, SIGNAL(triggered()), tabs, SLOT(zoomOut()));

    connect(ui.actionOpenPage, SIGNAL(triggered()), tabs, SLOT(newTab()));
    connect(ui.actionClosePage, SIGNAL(triggered()), tabs, SLOT(closeTab()));
    connect(ui.actionNextPage, SIGNAL(triggered()), tabs, SLOT(nextTab()));
    connect(ui.actionPrevPage, SIGNAL(triggered()), tabs, SLOT(previousTab()));


#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
    QAccel *acc = new QAccel(this);
//     acc->connectItem(acc->insertItem(Key_F5), browser, SLOT(reload()));
    acc->connectItem(acc->insertItem(QKeySequence("SHIFT+CTRL+=")), ui.actionZoomIn, SIGNAL(triggered()));
#endif

    QAccel *a = new QAccel(this, dw);
    a->connectItem(a->insertItem(QKeySequence("Ctrl+T")),
                    helpDock, SLOT(toggleContents()));
    a->connectItem(a->insertItem(QKeySequence("Ctrl+I")),
                    helpDock, SLOT(toggleIndex()));
    a->connectItem(a->insertItem(QKeySequence("Ctrl+B")),
                    helpDock, SLOT(toggleBookmarks()));
    a->connectItem(a->insertItem(QKeySequence("Ctrl+S")),
                    helpDock, SLOT(toggleSearch()));

    Config *config = Config::configuration();

    setupBookmarkMenu();
#if 0 /// ### port me
    ui.PopupMenu->addMenu(tr("Vie&ws"), createDockWindowMenu());
#endif
    helpDock->tabWidget()->setCurrentPage(config->sideBarPage());

    qApp->restoreOverrideCursor();
    ui.actionGoPrevious->setEnabled(false);
    ui.actionGoNext->setEnabled(false);
}

void MainWindow::setupGoActions()
{
    Config *config = Config::configuration();
    QStringList titles = config->docTitles();
    QAction *action = 0;

    static bool separatorInserted = false;

    foreach (QAction *a, goActions) {
        ui.goMenu->removeAction(a);
        ui.goActionToolbar->removeAction(a);
    }
    qDeleteAll(goActions);
    goActionDocFiles->clear();

    int addCount = 0;

    foreach (QString title, titles) {
        QPixmap pix = config->docIcon(title);
        if(!pix.isNull()) {
            if(!separatorInserted) {
                ui.goMenu->addSeparator();
                separatorInserted = true;
            }
            action = new QAction(this);
            action->setText(title);
            action->setIcon(QIconSet(pix));
            ui.goMenu->addAction(action);
            ui.goActionToolbar->addAction(action);
            goActions.append(action);
            goActionDocFiles->insert(action, config->indexPage(title));
            connect(action, SIGNAL(triggered()),
                     this, SLOT(showGoActionLink()));
            ++addCount;
        }
    }
    if(!addCount)
        ui.goActionToolbar->hide();
    else
        ui.goActionToolbar->show();

}

bool MainWindow::insertActionSeparator()
{
    ui.goMenu->addSeparator();
    ui.Toolbar->addSeparator();
    return true;
}

bool MainWindow::close()
{
    saveSettings();
    return QMainWindow::close();
}

void MainWindow::about()
{
    QMessageBox box(this);
    box.setText("<center><img src=\"splash.png\">"
                 "<p>Version " + QString(QT_VERSION_STR) + "</p>"
                 "<p>Copyright (C) 2000-$THISYEAR$ Trolltech AS. All rights reserved."
                 "</p></center><p></p>"
                 "<p>Qt Commercial Edition license holders: This program is"
                 " licensed to you under the terms of the Qt Commercial License"
                 " Agreement. For details, see the file LICENSE that came with"
                 " this software distribution.</p><p></p>"
                 "<p>Qt Free Edition users: This program is licensed to you"
                 " under the terms of the GNU General Public License Version 2."
                 " For details, see the file LICENSE.GPL that came with this"
                 " software distribution.</p><p>The program is provided AS IS"
                 " with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF"
                 " DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE."
                 "</p>");
    box.setWindowTitle(tr("Qt Assistant"));
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}

void MainWindow::on_actionAboutApplication_triggered()
{
    QString url = Config::configuration()->aboutURL();
    if (url == "about_qt") {
        QMessageBox::aboutQt(this, "Qt Assistant");
        return;
    }
    QString text;
    QFile file(url);
    if(file.exists() && file.open(IO_ReadOnly))
        text = QString(file.readAll());
    if(text.isNull())
        text = tr("Failed to open about application contents in file: '%1'").arg(url);

    QMessageBox box(this);
    box.setText(text);
    box.setWindowTitle(Config::configuration()->aboutApplicationMenuText());
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}

void MainWindow::on_actionEditFind_triggered()
{
    if (!findDialog)
        findDialog = new FindDialog(this);
    findDialog->reset();
    findDialog->show();
}

void MainWindow::on_actionEditFindAgain_triggered()
{
    if (!findDialog || !findDialog->hasFindExpression()) {
        on_actionEditFind_triggered();
        return;
    }
    findDialog->doFind(true);
}

void MainWindow::on_actionEditFindAgainPrev_triggered()
{
    if (!findDialog || !findDialog->hasFindExpression()) {
        on_actionEditFind_triggered();
        return;
    }
    findDialog->doFind(false);
}

void MainWindow::on_actionGoHome_triggered()
{
    showLink(Config::configuration()->homePage());
}

void MainWindow::on_actionFilePrint_triggered()
{
#if 0 // ### enable me
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);
    if (printer.setup(this)) {
        QPainter p;
        if (!p.begin(&printer))
            return;

        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
        qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);

        QPaintDeviceMetrics metrics(p.device());
        QTextBrowser *browser = tabs->currentBrowser();
        int dpiy = metrics.logicalDpiY();
        int margin = (int) ((2/2.54)*dpiy);
        QRect body(margin,
                    margin,
                    metrics.width() - 2 * margin,
                    metrics.height() - 2 * margin);
        QSimpleRichText richText(browser->text(), browser->QWidget::font(), browser->context(),
                                  browser->styleSheet(), browser->mimeSourceFactory(),
                                  body.height(), Qt::black, false);
        richText.setWidth(&p, body.width());
        QRect view(body);
        int page = 1;
        do {
            qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);

            richText.draw(&p, body.left(), body.top(), view, palette());
            view.moveBy(0, body.height());
            p.translate(0 , -body.height());
            p.drawText(view.right() - p.fontMetrics().width(QString::number(page)),
                        view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page));
            if (view.top() >= richText.height())
                break;
            printer.newPage();
            page++;
        } while (true);

        qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
        qApp->restoreOverrideCursor();
    }
#endif
}

void MainWindow::updateBookmarkMenu()
{
    for(QList<MainWindow*>::Iterator it = windows.begin(); it != windows.end(); ++it)
        (*it)->setupBookmarkMenu();
}

void MainWindow::setupBookmarkMenu()
{
    ui.bookmarkMenu->clear();
    bookmarks.clear();
    ui.bookmarkMenu->addAction(ui.actionAddBookmark);

    QFile f(QDir::homeDirPath() + "/.assistant/bookmarks." +
        Config::configuration()->profileName());
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream ts(&f);
    ui.bookmarkMenu->addSeparator();
    while (!ts.atEnd()) {
        QString title = ts.readLine();
        QString link = ts.readLine();
        bookmarks.insert(ui.bookmarkMenu->addAction(title), link);
    }
}

void MainWindow::showBookmark(QAction *action)
{
    if (bookmarks.contains(action))
        showLink(bookmarks.value(action));
}

void MainWindow::showLinkFromClient(const QString &link)
{
    setWindowState(windowState() & ~WindowMinimized);
    raise();
    setActiveWindow();
    showLink(link);
    if (isMinimized())
        showNormal();
}

void MainWindow::showLink(const QString &link)
{
    if(link.isEmpty()) {
        qWarning("The link is empty!");
    }

    int find = link.indexOf('#');
    QString name = find >= 0 ? link.left(find) : link;

    QString absLink = link;
    QFileInfo fi(name);
    if (fi.isRelative()) {
        if (find >= 0)
            absLink = fi.absFilePath() + link.right(link.length() - find);
        else
            absLink = fi.absFilePath();
    }
    if(fi.exists()) {
        tabs->setSource(absLink);
        tabs->currentBrowser()->setFocus();
    } else {
        // ### Default 404 site!
        statusBar()->message(tr("Failed to open link: '%1'").arg(link), 5000);
        tabs->currentBrowser()->setText(tr("<div align=\"center\"><h1>The page could not be found!</h1><br>"
                                             "<h3>'%1'</h3></div>").arg(link));
        tabs->updateTitle(tr("Error..."));
    }
}

void MainWindow::showLinks(const QStringList &links)
{
    if (links.size() == 0) {
        qWarning("MainWindow::showLinks() - Empty link");
        return;
    }

    if (links.size() == 1) {
        showLink(links.first());
        return;
    }

    pendingLinks = links;

    QStringList::ConstIterator it = pendingLinks.begin();
    // Initial showing, The tab is empty so update that without creating it first
    if (tabs->currentBrowser()->source().isEmpty()) {
        pendingBrowsers.append(tabs->currentBrowser());
        tabs->setTitle(tabs->currentBrowser(), pendingLinks.first());
    }
    ++it;

    while(it != pendingLinks.end()) {
        pendingBrowsers.append(tabs->newBackgroundTab(*it));
        ++it;
    }

    startTimer(50);
    return;
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    QString link = pendingLinks.first();
    HelpWindow *win = pendingBrowsers.first();
    pendingLinks.pop_front();
    pendingBrowsers.removeFirst();
    if (pendingLinks.size() == 0)
        killTimer(e->timerId());
    win->setSource(link);
}

void MainWindow::showQtHelp()
{
    showLink(QString(qInstallPathDocs()) + "/html/index.html");
}

void MainWindow::on_actionSettings_triggered()
{
    showSettingsDialog(-1);
}

void MainWindow::showWebBrowserSettings()
{
    showSettingsDialog(1);
}

void MainWindow::showSettingsDialog(int page)
{
    if (!settingsDia){
        settingsDia = new SettingsDialog(this);
    }
    QFontDatabase fonts;
    settingsDia->fontCombo()->clear();
    settingsDia->fontCombo()->insertStringList(fonts.families());
    settingsDia->fontCombo()->lineEdit()->setText(tabs->browserFont().family());
    settingsDia->fixedFontCombo()->clear();
    settingsDia->fixedFontCombo()->insertStringList(fonts.families());
    settingsDia->fixedFontCombo()->lineEdit()->setText(tabs->styleSheet()->item("pre")->fontFamily());
    settingsDia->linkUnderlineCB()->setChecked(tabs->linkUnderline());

    QPalette pal = settingsDia->colorButton()->palette();
    pal.setColor(QPalette::Active, settingsDia->colorButton()->backgroundRole(), tabs->palette().color(QPalette::Active, QPalette::Link));
    settingsDia->colorButton()->setPalette(pal);

    if (page != -1)
        settingsDia->settingsTab()->setCurrentPage(page);

    int ret = settingsDia->exec();

    if (ret != QDialog::Accepted)
        return;

    QObjectList lst = ui.Toolbar->children();
    for (int i = 0; i < lst.size(); ++i) {
        QObject *obj = lst.at(i);
        if (qstrcmp(obj->metaObject()->className(), "QToolBarSeparator") == 0) {
            delete obj;
            break;
        }
    }

    setupGoActions();

    QFont fnt(tabs->browserFont());
    fnt.setFamily(settingsDia->fontCombo()->currentText());
    tabs->setBrowserFont(fnt);
    tabs->setLinkUnderline(settingsDia->linkUnderlineCB()->isChecked());

    pal = tabs->palette();
    QColor lc = settingsDia->colorButton()->palette().color(backgroundRole());
    pal.setColor(QPalette::Active, QPalette::Link, lc);
    pal.setColor(QPalette::Inactive, QPalette::Link, lc);
    pal.setColor(QPalette::Disabled, QPalette::Link, lc);
    tabs->setPalette(pal);

    QString family = settingsDia->fixedFontCombo()->currentText();

    QStyleSheet *sh = tabs->styleSheet();
    sh->item("pre")->setFontFamily(family);
    sh->item("code")->setFontFamily(family);
    sh->item("tt")->setFontFamily(family);
    tabs->currentBrowser()->setText(tabs->currentBrowser()->text());
    showLink(tabs->currentBrowser()->source());
}

void MainWindow::hide()
{
    saveToolbarSettings();
    QMainWindow::hide();
}

MainWindow* MainWindow::newWindow()
{
    saveSettings();
    saveToolbarSettings();
    MainWindow *mw = new MainWindow;
    mw->move(geometry().topLeft());
    if (isMaximized())
        mw->showMaximized();
    else
        mw->show();
    mw->on_actionGoHome_triggered();
    return mw;
}

void MainWindow::saveSettings()
{
    Config *config = Config::configuration();
    config->setFontFamily(tabs->browserFont().family());
    config->setFontSize(tabs->currentBrowser()->font().pointSize());
    config->setFontFixedFamily(tabs->styleSheet()->item("pre")->fontFamily());
    config->setLinkUnderline(tabs->linkUnderline());
    config->setLinkColor(tabs->palette().color(QPalette::Active, QPalette::Link).name());
    config->setSideBarPage(helpDock->tabWidget()->currentIndex());
    config->setGeometry(QRect(x(), y(), width(), height()));
    config->setMaximized(isMaximized());

    // Create list of the tab urls
    QStringList lst;
    QList<HelpWindow*> browsers = tabs->browsers();
    foreach (HelpWindow *browser, browsers)
        lst << browser->source();
    config->setSource(lst);
    config->save();
}

void MainWindow::saveToolbarSettings()
{
#if 0 /// ### port me
    QString mainWindowLayout;
    QTextStream ts(&mainWindowLayout, IO_WriteOnly);
    ts << *this;
    Config::configuration()->setMainWindowLayout(mainWindowLayout);
#endif
}

TabbedBrowser* MainWindow::browsers() const
{
    return tabs;
}

void MainWindow::showSearchLink(const QString &link, const QStringList &terms)
{
    HelpWindow * hw = tabs->currentBrowser();
    hw->blockScrolling(true);
    hw->setCursor(waitCursor);
    if (hw->source() == link)
        hw->reload();
    else
        showLink(link);
    hw->sync();
    hw->setCursor(arrowCursor);

    hw->viewport()->setUpdatesEnabled(false);
    int minPar = INT_MAX;
    int minIndex = INT_MAX;
    foreach (QString term, terms) {
        int para = 0;
        int index = 0;
        bool found = hw->find(term, false, true, true, &para, &index);
        while (found) {
            if (para < minPar) {
                minPar = para;
                minIndex = index;
            }
            hw->setColor(red);
            found = hw->find(term, false, true, true);
        }
    }
    hw->blockScrolling(false);
    hw->viewport()->setUpdatesEnabled(true);
    hw->setCursorPosition(minPar, minIndex);
    hw->updateContents();
}


void MainWindow::showGoActionLink()
{
    const QObject *origin = sender();
    if(!origin ||
        origin->metaObject()->className() != QString("QAction"))
        return;

    QAction *action = (QAction*) origin;
    QString docfile = *(goActionDocFiles->find(action));
    showLink(docfile);
}

void MainWindow::on_actionHelpAssistant_triggered()
{
    showLink(Config::configuration()->assistantDocPath() + "/assistant.html");
}

HelpDialog* MainWindow::helpDialog() const
{
    return helpDock;
}

void MainWindow::backwardAvailable(bool enable)
{
    ui.actionGoPrevious->setEnabled(enable);
}

void MainWindow::forwardAvailable(bool enable)
{
    ui.actionGoNext->setEnabled(enable);
}

void MainWindow::updateProfileSettings()
{
    Config *config = Config::configuration();
#ifndef Q_WS_MAC
    setWindowIcon(config->applicationIcon());
#endif
    ui.helpMenu->clear();
    ui.helpMenu->addAction(ui.actionHelpAssistant);
    ui.helpMenu->addSeparator();
    ui.helpMenu->addAction(ui.helpAbout_Qt_AssistantAction);
    if (!config->aboutApplicationMenuText().isEmpty())
        ui.helpMenu->addAction(ui.actionAboutApplication);
    ui.helpMenu->addSeparator();
    ui.helpMenu->addAction(ui.actionHelpWhatsThis);

    ui.actionAboutApplication->setMenuText(config->aboutApplicationMenuText());

    if(!config->title().isNull())
        setWindowTitle(config->title());
}

void MainWindow::setupPopupMenu(QMenu *m)
{
    m->addAction(ui.actionNewWindow);
    m->addAction(ui.actionOpenPage);
    m->addAction(ui.actionClosePage);
    m->addSeparator();
    m->addAction(ui.actionGoPrevious);
    m->addAction(ui.actionGoNext);
    m->addAction(ui.actionGoHome);
    m->addSeparator();
    m->addAction(ui.actionZoomIn);
    m->addAction(ui.actionZoomOut);
    m->addSeparator();
    m->addAction(ui.actionEditCopy);
    m->addAction(ui.actionEditFind);
}

void MainWindow::on_actionClose_triggered()
{
    close();
}

void MainWindow::on_actionHelpWhatsThis_triggered()
{
    whatsThis();
}

