/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef THEMES_H
#define THEMES_H

#include <qmainwindow.h>
#include <qfont.h>

class QTabWidget;

class Themes: public QMainWindow
{
    Q_OBJECT

public:
    Themes( QWidget *parent = 0, const char *name = 0, WFlags f = WType_TopLevel );

protected:
    QTabWidget *tabwidget;

protected slots:
    void makeStyle(const QString &);
    void about();
    void aboutQt();

private:
    QFont appFont;
};


#endif
