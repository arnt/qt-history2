/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef HELP_H
#define HELP_H

#if defined(HAVE_KDE)
#include <kmainwindow.h>
class KToolBar;
#else
class QToolBar;
#include <qmainwindow.h>
#endif

#include <qmap.h>

class QTextBrowser;
class MainWindow;
class HelpDialog;
class QPopupMenu;

#if defined(HAVE_KDE)
#define QMainWindow KMainWindow
#endif

class Help : public QMainWindow
{
    Q_OBJECT

#undef QMainWindow
    
public:
    Help( const QString& home, MainWindow* parent = 0, const char *name=0 );
    ~Help();

    void setSource( const QString& );

    void setupBookmarkMenu();
    QTextBrowser *viewer() const { return browser; }

private slots:
    void textChanged();
    void filePrint();
    void goTopics();
    void goHome();
    void goQt();
    void showLink( const QString &link, const QString &title );
    void showBookmark( int id );

private:
    void setupFileActions();
    void setupGoActions();

private:
    QTextBrowser* browser;
#if defined(HAVE_KDE)
    KToolBar *toolbar;
#else
    QToolBar *toolbar;
#endif
    MainWindow *mainWindow;
    HelpDialog *helpDialog;
    QPopupMenu *bookmarkMenu;
    QMap<int, QString> bookmarks;

};

#endif
