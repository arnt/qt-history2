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

#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <q3mainwindow.h>
#include <qstring.h>

class MyWidget : public Q3MainWindow
{
    Q_OBJECT

public:
    MyWidget( QWidget* parent=0, const char* name = 0 );

signals:
    void closed();

protected:
    void closeEvent(QCloseEvent*);

private:
    static void initChoices(QWidget* parent);
};

#endif
