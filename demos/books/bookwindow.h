/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BOOKWINDOW_H
#define BOOKWINDOW_H

#include <QtGui>
#include <QtSql>

#include "ui_bookwindow.h"


class BookWindow: public QMainWindow
{
    Q_OBJECT
public:
    BookWindow();

private:
    void showError(const QSqlError &err);
    Ui::BookWindow ui;
    QSqlRelationalTableModel *model;
    int authorIdx, genreIdx;
};

#endif
