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

#include "helpwindow.h"
#include "mainwindow.h"
#include "tabbedbrowser.h"
#include "helpdialog.h"
#include "config.h"

#include <QApplication>
#include <QClipboard>
#include <QUrl>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMenu>
#include <QAction>
#include <QFileInfo>
#include <QFont>
#include <QtEvents>
#include <QTextStream>
#include <QTextCodec>
#include <QStatusBar>
#include <QTextCursor>
#include <QTextObject>
#include <QTextLayout>
#include <QtDebug>

#if defined(Q_OS_WIN32)
#  include <windows.h>
#endif

HelpWindow::HelpWindow(MainWindow *w, QWidget *parent)
    : QTextBrowser(parent), mw(w), blockScroll(false),
      shiftPressed(false), newWindow(false),
      fwdAvail(false), backAvail(false)
{
    QFont f = font();
    f.setPointSizeF(Config::configuration()->fontPointSize());
    setFont(f);
    connect(this, SIGNAL(forwardAvailable(bool)), this, SLOT(updateForward(bool)));
    connect(this, SIGNAL(backwardAvailable(bool)), this, SLOT(updateBackward(bool)));
}

void HelpWindow::setSource(const QUrl &name)
{
    if (!name.isValid())
        return;

    shiftPressed = shiftPressed & hasFocus();

    if (newWindow || shiftPressed) {
        shiftPressed = false;
        QTextCursor c = textCursor();
        c.clearSelection();
        setTextCursor(c);
        mw->saveSettings();
        MainWindow *nmw = new MainWindow;

        nmw->setup();
        nmw->showLink(name.toString());
        nmw->move(mw->geometry().topLeft());
        if (mw->isMaximized())
            nmw->showMaximized();
        else
            nmw->show();
        return;
    }

    if (name.scheme() == QLatin1String("http") || name.scheme() == QLatin1String("ftp") || name.scheme() == QLatin1String("mailto")) {
        QString webbrowser = Config::configuration()->webBrowser();

#if defined(Q_OS_WIN32)
        if (webbrowser.isEmpty()) {
            QT_WA({
                ShellExecute(winId(), 0, (TCHAR*)name.toString().utf16(), 0, 0, SW_SHOWNORMAL);
            } , {
                ShellExecuteA(winId(), 0, name.toString().toLocal8Bit(), 0, 0, SW_SHOWNORMAL);
            });
            return;
        }
#endif

        if (webbrowser.isEmpty()) {
#if defined(Q_OS_MAC)
            webbrowser = "open";
#elif defined(Q_WS_X11)
            if (isKDERunning()) {
                webbrowser = "kfmclient";
            }
#endif
        }

        if (webbrowser.isEmpty()) {
            int result = QMessageBox::information(mw, tr("Help"),
                         tr("Currently no Web browser is selected.\nPlease use the settings dialog to specify one!\n"),
                         tr("Open"), tr("Cancel"));
            if (result == 0) {
                emit chooseWebBrowser();
                webbrowser = Config::configuration()->webBrowser();
            }

            if (webbrowser.isEmpty())
                return;
        }

        QProcess *proc = new QProcess(this);
        QObject::connect(proc, SIGNAL(finished(int)), proc, SLOT(deleteLater()));

        QStringList args;
        if (webbrowser == QLatin1String("kfmclient"))
            args.append(QLatin1String("exec"));
        args.append(name.toString());

        proc->start(webbrowser, args);
        return;
    }

    if (name.path().right(3) == QLatin1String("pdf")) {
        QString pdfbrowser = Config::configuration()->pdfReader();

#if defined(Q_OS_WIN32)
        if (pdfbrowser.isEmpty()) {
            QT_WA({
                ShellExecute(winId(), 0, (TCHAR*)name.toString().utf16(), 0, 0, SW_SHOWNORMAL);
            } , {
                ShellExecuteA(winId(), 0, name.toString().toLocal8Bit(), 0, 0, SW_SHOWNORMAL);
            });
            return;
        }
#endif

        if (pdfbrowser.isEmpty()) {
#if defined(Q_OS_MAC)
            pdfbrowser = "open";
#elif defined(Q_WS_X11)
            if (isKDERunning()) {
                pdfbrowser = "kfmclient";
            }
#endif
        }

        if (pdfbrowser.isEmpty()) {
            int result = QMessageBox::information(mw, tr("Help"),
                         tr("Currently no PDF viewer is selected.\nPlease use the settings dialog to specify one!\n"),
                         tr("Open"), tr("Cancel"));
            if (result == 0) {
                emit choosePDFReader();
                pdfbrowser = Config::configuration()->pdfReader();
            }

            if (pdfbrowser.isEmpty())
                return;
        }

        QFileInfo info(pdfbrowser);
        if(!info.exists()) {
            QMessageBox::information(mw,
                                      tr("Help"),
                                      tr("Qt Assistant is unable to start the PDF Viewer\n\n"
                                          "%1\n\n"
                                          "Please make sure that the executable exists and is located at\n"
                                          "the specified location.").arg(pdfbrowser));
            return;
        }
        QProcess *proc = new QProcess(this);
        QObject::connect(proc, SIGNAL(finished()), proc, SLOT(deleteLater()));

        QStringList args;
        if (pdfbrowser == QLatin1String("kfmclient"))
            args.append(QLatin1String("exec"));
        args.append(name.toString());

        proc->start(pdfbrowser, args);

        return;
    }

    if (name.scheme() == QLatin1String("file")) {
        QFileInfo fi(name.toLocalFile());
        if (!fi.exists()) {
            mw->statusBar()->showMessage(tr("Failed to open link: '%1'").arg(fi.absoluteFilePath()), 5000);
            setHtml(tr("<div align=\"center\"><h1>The page could not be found</h1><br>"
                "<h3>'%1'</h3></div>").arg(fi.absoluteFilePath()));
            mw->browsers()->updateTitle(tr("Error..."));
            return;
        }

        /*
        setHtml(QLatin1String("<body bgcolor=\"")
            + palette().color(backgroundRole()).name()
            + QLatin1String("\">"));
            */

        QTextBrowser::setSource(name);

        return;
    }
    mw->statusBar()->showMessage(tr("Failed to open link: '%1'").arg(name.toString()), 5000);
    setHtml(tr("<div align=\"center\"><h1>The page could not be found</h1><br>"
        "<h3>'%1'</h3></div>").arg(name.toString()));
    mw->browsers()->updateTitle(tr("Error..."));
}

void HelpWindow::openLinkInNewWindow()
{
    if (lastAnchor.isEmpty())
        return;
    newWindow = true;
    setSource(lastAnchor);
    newWindow = false;
}

void HelpWindow::openLinkInNewWindow(const QString &link)
{
    lastAnchor = link;
    openLinkInNewWindow();
}

void HelpWindow::openLinkInNewPage()
{
    if(lastAnchor.isEmpty())
        return;
    mw->browsers()->newTab(lastAnchor);
    lastAnchor.clear();
}

void HelpWindow::openLinkInNewPage(const QString &link)
{
    lastAnchor = link;
    openLinkInNewPage();
}

bool HelpWindow::hasAnchorAt(const QPoint& pos)
{
    lastAnchor = anchorAt(pos);
    if (lastAnchor.isEmpty()) 
        return false;
    lastAnchor = source().resolved(lastAnchor).toString();
    if (lastAnchor.at(0) == QLatin1Char('#')) {
        QString src = source().toString();
        int hsh = src.indexOf(QLatin1Char('#'));
        lastAnchor = (hsh>=0 ? src.left(hsh) : src) + lastAnchor;
    }
    return true;
}

void HelpWindow::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *m = new QMenu(0);
    if (hasAnchorAt(e->pos())) {
        m->addAction(tr("Open Link in New Window\tShift+LMB"),
                       this, SLOT(openLinkInNewWindow()));
        m->addAction(tr("Open Link in New Tab"),
                       this, SLOT(openLinkInNewPage()));
    }
    mw->setupPopupMenu(m);
    m->exec(e->globalPos());
    delete m;
}

void HelpWindow::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MidButton && hasAnchorAt(e->pos())) {
        openLinkInNewPage();
        return;
    }
    QTextBrowser::mouseReleaseEvent(e);
}

void HelpWindow::blockScrolling(bool b)
{
    blockScroll = b;
}

void HelpWindow::ensureCursorVisible()
{
    if (!blockScroll)
        QTextBrowser::ensureCursorVisible();
}

void HelpWindow::mousePressEvent(QMouseEvent *e)
{
    shiftPressed = e->modifiers() & Qt::ShiftModifier;
    QTextBrowser::mousePressEvent(e);
}

void HelpWindow::keyPressEvent(QKeyEvent *e)
{
    shiftPressed = e->modifiers() & Qt::ShiftModifier;
	if (e->key() == Qt::Key_Return)
		followSelectedLink();
	else
		QTextBrowser::keyPressEvent(e);
}

void HelpWindow::updateForward(bool fwd)
{
    fwdAvail = fwd;
}

void HelpWindow::updateBackward(bool back)
{
    backAvail = back;
}

bool HelpWindow::isKDERunning() const
{
    return !qgetenv("KDE_FULL_SESSION").isEmpty();
}

void HelpWindow::followSelectedLink()
{
	QTextCursor c = textCursor();
    QTextBlock b = c.block();

    for (QTextBlock::iterator it = b.begin(); it != b.end(); ++it) {
        QTextFragment f = it.fragment();
        QTextCharFormat cf = f.charFormat();
		if (f.position() > c.position())
			break;
		if (f.position() + f.length() < c.anchor())
			continue;
        if (!cf.isAnchor())
            continue;
        QString anchor = cf.anchorHref();
        if (anchor.isEmpty())
            continue;
		QUrl url = source();	
        anchor = url.resolved(anchor).toString();
        if (anchor.at(0) == QLatin1Char('#')) {
            QString src = url.toString();
            int hsh = src.indexOf(QLatin1Char('#'));
            anchor = (hsh>=0 ? src.left(hsh) : src) + anchor;
        }
        setSource(anchor);
        return;
    }
}
