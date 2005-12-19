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

#ifndef DROPSITEWINDOW_H
#define DROPSITEWINDOW_H

#include <QWidget>
#include "dropsitewidget.h"

class QLabel;
class QMimeData;
class QPushButton;
class QStringList;
class QTableWidget;

class DropSiteWindow : public QWidget
{
    Q_OBJECT

public:
    DropSiteWindow();

public slots:
    void updateSupportedFormats(const QMimeData *mimeData = 0);

private:
    DropSiteWidget *dropSiteWidget;
    QLabel *abstractLabel;
    QTableWidget *supportedFormats;

    QPushButton *clearButton;
    QPushButton *quitButton;
};

#endif
