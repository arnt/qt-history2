/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Assistant.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qtextbrowser.h>

class MainWindow;

class HelpWindow : public QTextBrowser
{
    Q_OBJECT
public:
    HelpWindow( MainWindow *m, QWidget *parent = 0, const char *name = 0 );
    void setSource( const QString &name );
    QPopupMenu *createPopupMenu( const QPoint& pos );
    void blockScrolling( bool b );

protected:
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );

protected slots:
    void ensureCursorVisible();

private slots:
    void openLinkInNewWindow();

private:
    MainWindow *mw;
    bool shiftPressed;
    QString lastAnchor;
    bool blockScroll;

};

#endif
