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

#include "mainwindow.h"
#include "tabbedbrowser.h"
#include "helpdialog.h"
#include "finddialog.h"
#include "settingsdialog.h"
#include "config.h"

#include <qdockwidget.h>
#include <qdir.h>
#include <qtimer.h>
#include <qstatusbar.h>
#include <qshortcut.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qeventloop.h>
#include <qevent.h>
#include <qfontdatabase.h>
#include <qwhatsthis.h>
#include <qtextdocumentfragment.h>
#include <qlibraryinfo.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include <qabstracttextdocumentlayout.h>
#include <qtextdocument.h>
#include <qtextobject.h>
#include <qfiledialog.h>

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
    setupCompleted = false;

    goActions = QList<QAction*>();
    goActionDocFiles = new QMap<QAction*,QString>;

    windows.append(this);
    tabs = new TabbedBrowser(this);
    setCentralWidget(tabs);
    settingsDia = 0;

    Config *config = Config::configuration();

    updateProfileSettings();

    dw = new QDockWidget(this);
    dw->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dw->setWindowTitle(tr("Sidebar"));
    helpDock = new HelpDialog(dw, this);
    dw->setWidget(helpDock);

    addDockWidget(Qt::LeftDockWidgetArea, dw);

    // read geometry configuration
    setupGoActions();

    if (!config->isMaximized()) {
        QRect geom = config->geometry();
        if(geom.isValid()) {
            resize(geom.size());
            move(geom.topLeft());
        }
    }

    restoreState(config->mainWindowState());
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
    statusBar()->showMessage(tr("Initializing Qt Assistant..."));
    setupCompleted = true;
    helpDock->initialize();
    connect(ui.actionGoPrevious, SIGNAL(triggered()), tabs, SLOT(backward()));
    connect(ui.actionGoNext, SIGNAL(triggered()), tabs, SLOT(forward()));
    connect(ui.actionEditCopy, SIGNAL(triggered()), tabs, SLOT(copy()));
    connect(ui.actionFileExit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
    connect(ui.actionAddBookmark, SIGNAL(triggered()),
             helpDock, SLOT(addBookmark()));
    connect(helpDock, SIGNAL(showLink(QString)),
             this, SLOT(showLink(QString)));
    connect(helpDock, SIGNAL(showSearchLink(QString,QStringList)),
             this, SLOT(showSearchLink(QString,QStringList)));

    connect(ui.bookmarkMenu, SIGNAL(triggered(QAction*)),
             this, SLOT(showBookmark(QAction*)));
    connect(ui.actionZoomIn, SIGNAL(triggered()), tabs, SLOT(zoomIn()));
    connect(ui.actionZoomOut, SIGNAL(triggered()), tabs, SLOT(zoomOut()));

    connect(ui.actionOpenPage, SIGNAL(triggered()), tabs, SLOT(newTab()));
    connect(ui.actionClosePage, SIGNAL(triggered()), tabs, SLOT(closeTab()));
    connect(ui.actionNextPage, SIGNAL(triggered()), tabs, SLOT(nextTab()));
    connect(ui.actionPrevPage, SIGNAL(triggered()), tabs, SLOT(previousTab()));


#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
    QShortcut *acc = new QShortcut(tr("SHIFT+CTRL+="), this);
    connect(acc, SIGNAL(activated()), ui.actionZoomIn, SIGNAL(triggered()));
#endif

    connect(new QShortcut(tr("Ctrl+T"), this), SIGNAL(activated()), helpDock, SLOT(toggleContents()));
    connect(new QShortcut(tr("Ctrl+I"), this), SIGNAL(activated()), helpDock, SLOT(toggleIndex()));
    connect(new QShortcut(tr("Ctrl+B"), this), SIGNAL(activated()), helpDock, SLOT(toggleBookmarks()));
    connect(new QShortcut(tr("Ctrl+S"), this), SIGNAL(activated()), helpDock, SLOT(toggleSearch()));

    Config *config = Config::configuration();

    setupBookmarkMenu();

    QAction *viewsAction = createPopupMenu()->menuAction();
    viewsAction->setText(tr("Views"));
    ui.viewMenu->addAction(viewsAction);

    helpDock->tabWidget()->setCurrentIndex(config->sideBarPage());

    qApp->restoreOverrideCursor();
    ui.actionGoPrevious->setEnabled(false);
    ui.actionGoNext->setEnabled(false);
}

void MainWindow::browserTabChanged()
{
    if (tabs->currentBrowser()) {
        ui.actionGoPrevious->setEnabled(tabs->currentBrowser()->isBackwardAvailable());
        ui.actionGoNext->setEnabled(tabs->currentBrowser()->isForwardAvailable());
    }
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
    goActions.clear();
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
            action->setIcon(QIcon(pix));
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

void MainWindow::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}

void MainWindow::about()
{
    QMessageBox box(this);
    box.setText(QLatin1String("<center><img src=\"splash.png\">"
                 "<p>Version ") + QLatin1String(QT_VERSION_STR) + QLatin1String("</p>"
                 "<p>Copyright (C) 2000-$THISYEAR$ Trolltech AS. All rights reserved."
                 "</p></center><p></p>"
                 "<p>Qt Commercial Edition license holders: This program is"
                 " licensed to you under the terms of the Qt Commercial License"
                 " Agreement. For details, see the file LICENSE that came with"
                 " this software distribution.</p><p></p>"
                 "<p>Qt Open Source Edition users: This program is licensed to you"
                 " under the terms of the GNU General Public License Version 2."
                 " For details, see the file LICENSE.GPL that came with this"
                 " software distribution.</p><p>The program is provided AS IS"
                 " with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF"
                 " DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE."
                 "</p>"));
    box.setWindowTitle(tr("Qt Assistant"));
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}

void MainWindow::on_actionAboutApplication_triggered()
{
    QString url = Config::configuration()->aboutURL();
    if (url == QLatin1String("about_qt")) {
        QMessageBox::aboutQt(this, QLatin1String("Qt Assistant"));
        return;
    }
    QString text;
    if (url.startsWith("file:"))
        url = url.mid(5);
    QFile file(url);
    if(file.exists() && file.open(QFile::ReadOnly))
        text = QString::fromAscii(file.readAll());
    if(text.isNull())
        text = tr("Failed to open about application contents in file: '%1'").arg(url);

    QMessageBox box(this);
    box.setText(text);
    box.setWindowTitle(Config::configuration()->aboutApplicationMenuText());
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}

void MainWindow::on_actionAboutAssistant_triggered()
{
    about();
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
    QString home = MainWindow::urlifyFileName(Config::configuration()->homePage());
    showLink(home);
}

QString MainWindow::urlifyFileName(const QString &fileName)
{
    QString name = fileName;
    QUrl url(name);
#if defined(Q_OS_WIN32)
    if (!url.isValid() || url.scheme().isEmpty() || url.scheme().toLower() != "file:") {
        name = name.toLower();
        foreach (QFileInfo drive, QDir::drives()) {
            if (name.startsWith(drive.absolutePath().toLower())) {
                name = "file:" + name;
                name = name.replace("\\", "/");
                break;
            }
        }
    }
#else
    if (!url.isValid() || url.scheme().isEmpty())
        name.prepend("file:");
#endif
    return name;
}

void MainWindow::on_actionFilePrint_triggered()
{
    // ### change back to highres, when it works
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setFullPage(true);

    QPrintDialog *dlg = new QPrintDialog(&printer, this);

    const int dpiy = printer.logicalDpiY();
    const int margin = (int) ((2/2.54)*dpiy);
    QRectF body(margin, margin, printer.width() - 2*margin, printer.height() - 2*margin);

    QTextDocument *doc = tabs->currentBrowser()->document()->clone();
    QAbstractTextDocumentLayout *layout = doc->documentLayout();
    QFont font(doc->defaultFont());
    font.setPointSize(10);
    doc->setDefaultFont(font);
    layout->setPaintDevice(&printer);
    doc->setPageSize(body.size());

    int maxPages = doc->pageCount();
    dlg->addEnabledOption(QPrintDialog::PrintPageRange);
    dlg->setMinMax(1, maxPages);
    dlg->setFromTo(1, maxPages);

    if (dlg->exec() == QDialog::Accepted) {
        int from = 1;
        int to = maxPages;
        if (dlg->printRange() & QPrintDialog::PageRange) {
            from = dlg->fromPage();
            to = dlg->toPage();
        }
        delete dlg;

        QRectF view(0, 0, body.width(), body.height()*from);
        QPainter p(&printer);
        if (!p.device() || view.top() >= layout->documentSize().height()) {
            delete doc;
            return;
        }

        p.translate(body.left(), body.top()-body.height()*(from-1));

        int page = from;
        do {
            QAbstractTextDocumentLayout::PaintContext ctx;
            p.setClipRect(view);
            ctx.clip = view.toRect();

            layout->draw(&p, ctx);

            p.setClipping(false);
            p.setFont(font);
            QString pageString = QString::number(page);
            p.drawText(qRound(view.right() - p.fontMetrics().width(pageString)),
                qRound(view.bottom() + p.fontMetrics().ascent() + 5*dpiy/72), pageString);

            view.translate(0, body.height());
            p.translate(0 , -body.height());

            if (view.top() >= layout->documentSize().height())
                break;

            page++;
            if (page <= to)
                printer.newPage();
        } while (page <= to);
    }
    delete doc;
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

    QFile f(QDir::homePath() + QLatin1String("/.assistant/bookmarks.") +
        Config::configuration()->profileName());
    if (!f.open(QFile::ReadOnly))
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
    setWindowState(windowState() & ~Qt::WindowMinimized);
    raise();
    activateWindow();
    QString l = MainWindow::urlifyFileName(link);
    showLink(l);
    if (isMinimized())
        showNormal();
}

void MainWindow::showLink(const QString &link)
{
    if(link.isEmpty()) {
        qWarning("The link is empty!");
    }

    QUrl url(link);
    QFileInfo fi(url.toLocalFile());
    tabs->setSource(url.toString());
    tabs->currentBrowser()->setFocus();
}

void MainWindow::showLinks(const QStringList &links)
{
    if (links.size() == 0) {
        qWarning("MainWindow::showLinks() - Empty link");
        return;
    }

    if (links.size() == 1) {
        showLink(MainWindow::urlifyFileName(links.first()));
        return;
    }

    pendingLinks = links;

    QStringList::ConstIterator it = pendingLinks.begin();
    // Initial showing, The tab is empty so update that without creating it first
    if (!tabs->currentBrowser()->source().isValid()) {
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

    win->setSource(MainWindow::urlifyFileName(link));
}

void MainWindow::showQtHelp()
{
    showLink(QLibraryInfo::location(QLibraryInfo::DocumentationPath) +
             QLatin1String("/html/index.html"));
}

void MainWindow::on_actionSettings_triggered()
{
    showSettingsDialog(-1);
}

void MainWindow::showPDFReaderSettings()
{
    showSettingsDialog(2);
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

    settingsDia->fontCombo()->lineEdit()->setText(tabs->browserFont().family());
    settingsDia->fixedFontCombo()->lineEdit()->setText(tabs->fixedFontFamily());

    QPalette pal = settingsDia->colorButton()->palette();
	pal.setColor(QPalette::Active, QPalette::Button, tabs->linkColor());
    settingsDia->colorButton()->setPalette(pal);


    if (page != -1)
        settingsDia->settingsTab()->setCurrentIndex(page);

    if (settingsDia->exec() != QDialog::Accepted)
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

    tabs->applySettings();

    tabs->currentBrowser()->reload();
    //showLink(tabs->currentBrowser()->source().toString());
}

MainWindow* MainWindow::newWindow()
{
    saveSettings();
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

    config->setFontFixedFamily(tabs->fixedFontFamily());
    config->setLinkUnderline(tabs->underlineLink());
    config->setLinkColor(tabs->linkColor().name());

    config->setSideBarPage(helpDock->tabWidget()->currentIndex());
    config->setGeometry(QRect(x(), y(), width(), height()));
    config->setMaximized(isMaximized());
    config->setMainWindowState(saveState());

    // Create list of the tab urls
    QStringList lst;
    QList<HelpWindow*> browsers = tabs->browsers();
    foreach (HelpWindow *browser, browsers)
        lst << browser->source().toString();
    config->setSource(lst);
    config->save();
}

TabbedBrowser* MainWindow::browsers() const
{
    return tabs;
}

void MainWindow::showSearchLink(const QString &link, const QStringList &terms)
{
    HelpWindow * hw = tabs->currentBrowser();
    hw->blockScrolling(true);
    hw->setCursor(Qt::WaitCursor);
    if (hw->source() == link)
        hw->reload();
    else
        showLink(link);
    hw->setCursor(Qt::ArrowCursor);

    hw->viewport()->setUpdatesEnabled(false);

    QTextCharFormat marker;
    marker.setForeground(Qt::red);

    QTextCursor firstHit;

    QTextCursor c = hw->textCursor();
    c.beginEditBlock();
    foreach (QString term, terms) {
        c.movePosition(QTextCursor::Start);
        hw->setTextCursor(c);

        bool found = hw->find(term, QTextDocument::FindWholeWords);
        while (found) {
            QTextCursor hit = hw->textCursor();
            if (firstHit.isNull() || hit.position() < firstHit.position())
                firstHit = hit;

            hit.mergeCharFormat(marker);
            found = hw->find(term, QTextDocument::FindWholeWords);
        }
    }

    if (firstHit.isNull()) {
        firstHit = hw->textCursor();
        firstHit.movePosition(QTextCursor::Start);
    }
    firstHit.clearSelection();
    c.endEditBlock();
    hw->setTextCursor(firstHit);

    hw->blockScrolling(false);
    hw->viewport()->setUpdatesEnabled(true);
}


void MainWindow::showGoActionLink()
{
    const QObject *origin = sender();
    if(!origin ||
        QLatin1String(origin->metaObject()->className()) != QLatin1String("QAction"))
        return;

    QAction *action = (QAction*) origin;
    QString docfile = *(goActionDocFiles->find(action));
    showLink(MainWindow::urlifyFileName(docfile));
}

void MainWindow::on_actionHelpAssistant_triggered()
{
    showLink(Config::configuration()->assistantDocPath() + QLatin1String("/assistant-manual.html"));
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
    //ui.helpMenu->addAction(ui.actionHelpAssistant);
    //ui.helpMenu->addSeparator();
    ui.helpMenu->addAction(ui.actionAboutAssistant);
    if (!config->aboutApplicationMenuText().isEmpty())
        ui.helpMenu->addAction(ui.actionAboutApplication);
    ui.helpMenu->addSeparator();
    ui.helpMenu->addAction(ui.actionHelpWhatsThis);

    ui.actionAboutApplication->setText(config->aboutApplicationMenuText());

    if(!config->title().isNull())
        setWindowTitle(config->title());
}

void MainWindow::setupPopupMenu(QMenu *m)
{
    m->addAction(ui.actionNewWindow);
    m->addAction(ui.actionOpenPage);
    m->addAction(ui.actionClosePage);
    m->addSeparator();
    m->addAction(ui.actionSaveAs);
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

void MainWindow::on_actionNewWindow_triggered()
{
    newWindow()->show();
}

void MainWindow::on_actionClose_triggered()
{
    close();
}

void MainWindow::on_actionHelpWhatsThis_triggered()
{
    QWhatsThis::enterWhatsThisMode();
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString fileName;
    QUrl url = tabs->currentBrowser()->source();
    if (url.isValid()) {
        QFileInfo fi(url.toLocalFile());
        fileName = fi.fileName();
    }
    fileName = QFileDialog::getSaveFileName(this, tr("Save Page"), fileName);
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Save Page"), tr("Cannot open file for writing!"));
        return;
    }

    QFileInfo fi(fileName);
    QString fn = fi.fileName();
    int i = fn.lastIndexOf('.');
    if (i > -1)
        fn = fn.left(i);
    QString relativeDestPath = fn + "_images";
    QDir destDir(fi.absolutePath() + QDir::separator() + relativeDestPath);
    bool imgDirAvailable = destDir.exists();
    if (!imgDirAvailable)
        imgDirAvailable = destDir.mkdir(destDir.absolutePath());

    // save images
    QTextDocument *doc = tabs->currentBrowser()->document()->clone();
    if (url.isValid() && imgDirAvailable) {
        QTextBlock::iterator it;
        for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
            for (it = block.begin(); !(it.atEnd()); ++it) {
                QTextFragment fragment = it.fragment();
                if (fragment.isValid()) {
                    QTextImageFormat fm = fragment.charFormat().toImageFormat();
                    if (fm.isValid() && !fm.name().isEmpty()) {
                        QUrl imagePath = tabs->currentBrowser()->source().resolved(fm.name());
                        if (!imagePath.isValid())
                            continue;
                        QString from = imagePath.toLocalFile();
                        QString destName = fm.name();
                        int j = destName.lastIndexOf('/');
                        if (j > -1)
                            destName = destName.mid(j+1);
                        QFileInfo info(from);
                        if (info.exists()) {
                            if (!QFile::copy(from, destDir.absolutePath()
                                + QDir::separator() + destName))
                                continue;
                            fm.setName("./" + relativeDestPath + "/" + destName);
                            QTextCursor cursor(doc);
                            cursor.setPosition(fragment.position());
                            cursor.setPosition(fragment.position() + fragment.length(),
                                QTextCursor::KeepAnchor);
                            cursor.setCharFormat(fm);
                        }
                    }
                }
            }
        }
    }
    QString src = doc->toHtml();
    QTextStream s(&file);
    s << src;
    s.flush();
    file.close();
}
