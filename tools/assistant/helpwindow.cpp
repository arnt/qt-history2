/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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

#include <qapplication.h>
#include <qclipboard.h>
#include <qurl.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qfile.h>
#include <qprocess.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qfileinfo.h>
#include <qevent.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#if defined(Q_OS_WIN32)
#include <windows.h>
#endif

HelpWindow::HelpWindow(MainWindow *w, QWidget *parent, const char *name)
    : QTextBrowser(parent, name), mw(w), blockScroll(false),
      shiftPressed(false), newWindow(false)
{
}

void HelpWindow::setSource(const QString &name)
{
    if (name.isEmpty())
        return;

    if (newWindow || shiftPressed) {
        removeSelection();
        mw->saveSettings();
        mw->saveToolbarSettings();
        MainWindow *nmw = new MainWindow;

        QFileInfo currentInfo(source());
        QFileInfo linkInfo(name);
        QString target = name;
        if(linkInfo.isRelative())
            target = currentInfo.dirPath(true) + "/" + name;

        nmw->setup();
        nmw->showLink(target);
        nmw->move(mw->geometry().topLeft());
        if (mw->isMaximized())
            nmw->showMaximized();
        else
            nmw->show();
        return;
    }

    if (name.left(7) == "http://" || name.left(6) == "ftp://") {
        QString webbrowser = Config::configuration()->webBrowser();
        if (webbrowser.isEmpty()) {
#if defined(Q_OS_WIN32)
            QT_WA({
                ShellExecute(winId(), 0, (TCHAR*)name.ucs2(), 0, 0, SW_SHOWNORMAL);
            } , {
                ShellExecuteA(winId(), 0, name.local8Bit(), 0, 0, SW_SHOWNORMAL);
            });
#elif defined(Q_OS_MAC)
            webbrowser = "/usr/bin/open";
#else
            int result = QMessageBox::information(mw, tr("Help"),
                         tr("Currently no Web browser is selected.\nPlease use the settings dialog to specify one!\n"),
                         "Open", "Cancel");
            if (result == 0) {
                emit chooseWebBrowser();
                webbrowser = Config::configuration()->webBrowser();
            }
#endif
            if (webbrowser.isEmpty())
                return;
        }
        QProcess *proc = new QProcess(this);
        proc->setCommunication(0);
        QObject::connect(proc, SIGNAL(processExited()), proc, SLOT(deleteLater()));
        proc->addArgument(webbrowser);
        proc->addArgument(name);
        proc->start();
        return;
    }

    if (name.right(3) == "pdf") {
        QString pdfbrowser = Config::configuration()->pdfReader();
        if (pdfbrowser.isEmpty()) {
#if defined(Q_OS_MAC)
            pdfbrowser = "/usr/bin/open";
#else
            QMessageBox::information(mw,
                                      tr("Help"),
                                      tr("No PDF Viewer has been specified\n"
                                          "Please use the settings dialog to specify one!\n"));
            return;
#endif
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
        proc->setCommunication(0);
        QObject::connect(proc, SIGNAL(processExited()), proc, SLOT(deleteLater()));
        proc->addArgument(pdfbrowser);
        proc->addArgument(name);
        proc->start();

        return;
    }

    QUrl u = QUrl(context()).resolved(name);
    if (!u.isLocalFile()) {
        QMessageBox::information(mw, tr("Help"), tr("Can't load and display non-local file\n"
                    "%1").arg(name));
        return;
    }

    setText("<body bgcolor=\"" + palette().color(backgroundRole()).name() + "\">");
    QTextBrowser::setSource(name);
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
    lastAnchor = QString::null;
}

void HelpWindow::openLinkInNewPage(const QString &link)
{
    lastAnchor = link;
    openLinkInNewPage();
}

QPopupMenu *HelpWindow::createPopupMenu(const QPoint& pos)
{
    QPopupMenu *m = new QPopupMenu(0);
    lastAnchor = anchorAt(pos);
    if (!lastAnchor.isEmpty()) {
        if (lastAnchor.at(0) == '#') {
            QString src = source();
            int hsh = src.indexOf(QChar('#'));
            lastAnchor = (hsh>=0 ? src.left(hsh) : src) + lastAnchor;
        }
        m->addAction(tr("Open Link in New Window\tShift+LMB"),
                       this, SLOT(openLinkInNewWindow()));
        m->addAction(tr("Open Link in New Tab"),
                       this, SLOT(openLinkInNewPage()));
    }
    mw->setupPopupMenu(m);
    return m;
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

void HelpWindow::contentsMousePressEvent(QMouseEvent *e)
{
    shiftPressed = (e->state() & ShiftButton);
    QTextBrowser::contentsMousePressEvent(e);
}

void HelpWindow::keyPressEvent(QKeyEvent *e)
{
    shiftPressed = (e->state() & ShiftButton);
    QTextBrowser::keyPressEvent(e);
}

void HelpWindow::copy()
{
    if (textFormat() == PlainText) {
        QTextEdit::copy();
    } else {
        TextFormat oldTf = textFormat();
        setTextFormat(PlainText);
        QString selectText = selectedText();
        selectText.replace("<br>", "\n");
        selectText.replace("\xa0", " ");
        selectText.replace("&gt;", ">");
        selectText.replace("&lt;", "<");
        selectText.replace("&amp;", "&");

        QClipboard *cb = QApplication::clipboard();
        if (cb->supportsSelection())
            cb->setText(selectText, QClipboard::Selection);
        cb->setText(selectText, QClipboard::Clipboard);
        setTextFormat(oldTf);
    }
}

