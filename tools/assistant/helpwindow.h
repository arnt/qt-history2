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

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qtextbrowser.h>

class MainWindow;
class QKeyEvent;
class QMime;
class QMouseEvent;
class QMenu;

class HelpWindow : public QTextBrowser
{
    Q_OBJECT
public:
    HelpWindow( MainWindow *m, QWidget *parent = 0);
    void setSource( const QUrl &name );
    void blockScrolling( bool b );
    void openLinkInNewWindow( const QString &link );
    void openLinkInNewPage( const QString &link );
    void addMimePath( const QString &path );

    void mousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *);

    bool isBackwardAvailable() const { return backAvail; }
    bool isForwardAvailable() const { return fwdAvail; }

signals:
    void chooseWebBrowser();
    void choosePDFReader();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e);

protected slots:
    void ensureCursorVisible();

private slots:
    void openLinkInNewWindow();
    void openLinkInNewPage();
    void updateForward(bool);
    void updateBackward(bool);

    bool isKDERunning() const;

private:
    MainWindow *mw;
    QString lastAnchor;
    bool blockScroll;
    bool shiftPressed;
    bool newWindow;
    bool fwdAvail;
    bool backAvail;
};

#endif
