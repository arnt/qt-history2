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

#include <qapplication.h>
#include <qclipboard.h>
#include <qurl.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qfile.h>
#include <qprocess.h>
#include <qmenu.h>
#include <qaction.h>
#include <qfileinfo.h>
#include <qevent.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qstatusbar.h>
#include <qtextcursor.h>
#include <qtextobject.h>
#include <qtextlayout.h>
#include <qdebug.h>

#if defined(Q_OS_WIN32)
#  include <windows.h>
#elif defined(Q_WS_X11)
#  include <QtGui/QX11Info>
#  include <QtGui/private/qt_x11_p.h>
#endif

HelpWindow::HelpWindow(MainWindow *w, QWidget *parent)
    : QTextBrowser(parent), mw(w), blockScroll(false),
      shiftPressed(false), newWindow(false),
      fwdAvail(false), backAvail(false)
{
    connect(this, SIGNAL(forwardAvailable(bool)), this, SLOT(updateForward(bool)));
    connect(this, SIGNAL(backwardAvailable(bool)), this, SLOT(updateBackward(bool)));
}

void HelpWindow::setSource(const QUrl &name)
{
    if (!name.isValid())
        return;

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

    if (name.scheme() == QLatin1String("http") || name.scheme() == QLatin1String("ftp")) {
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
        updateFormat();

        return;
    }
    mw->statusBar()->showMessage(tr("Failed to open link: '%1'").arg(name.toString()), 5000);
    setHtml(tr("<div align=\"center\"><h1>The page could not be found</h1><br>"
        "<h3>'%1'</h3></div>").arg(name.toString()));
    mw->browsers()->updateTitle(tr("Error..."));
}

void HelpWindow::updateFormat()
{
    QString fixedFontFamily = mw->browsers()->fixedFontFamily();
    QColor linkColor = mw->browsers()->linkColor();
    bool underlineLinks = mw->browsers()->underlineLink();

    QTextDocument *doc = QTextBrowser::document();
    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        QTextLayout *layout = block.layout();
        QString txt = block.text();
        QList<QTextLayout::FormatRange> overrides;

        for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment fragment = it.fragment();
            QTextCharFormat fmt = fragment.charFormat();
            if (fmt.isAnchor() && !fmt.anchorHref().isEmpty()) {
                QTextLayout::FormatRange range;
                range.start = fragment.position() - block.position();
                range.length = fragment.length();
                fmt.setForeground(linkColor);
                fmt.setFontUnderline(underlineLinks);
                range.format = fmt;
                overrides.append(range);
                continue;
            }
            if (fmt.fontFixedPitch()) {
                QTextLayout::FormatRange range;
                range.start = fragment.position() - block.position();
                range.length = fragment.length();
                fmt.setFontFamily(fixedFontFamily);
                range.format = fmt;
                overrides.append(range);
            }
        }
        layout->setAdditionalFormats(overrides);
        doc->markContentsDirty(block.position(), block.length());
    }
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

void HelpWindow::contextMenuEvent(QContextMenuEvent *e)
{
    const QPoint pos = e->pos();
    QMenu *m = new QMenu(0);
    lastAnchor = anchorAt(pos);
    if (!lastAnchor.isEmpty()) {
        lastAnchor = source().resolved(lastAnchor).toString();
        if (lastAnchor.at(0) == QLatin1Char('#')) {
            QString src = source().toString();
            int hsh = src.indexOf(QLatin1Char('#'));
            lastAnchor = (hsh>=0 ? src.left(hsh) : src) + lastAnchor;
        }
        m->addAction(tr("Open Link in New Window\tShift+LMB"),
                       this, SLOT(openLinkInNewWindow()));
        m->addAction(tr("Open Link in New Tab"),
                       this, SLOT(openLinkInNewPage()));
    }
    mw->setupPopupMenu(m);
    m->exec(e->globalPos());
    delete m;
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
#if defined(Q_WS_X11)
    Atom type;
    int format;
    unsigned long length, after;
    uchar *data;

    if (XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(), ATOM(KWIN_RUNNING),
                             0, 1, False, AnyPropertyType, &type, &format, &length,
                             &after, &data) == Success && length) {
        if (data)
            XFree((char *)data);

        return true;
    }
#endif

    return false;
}

