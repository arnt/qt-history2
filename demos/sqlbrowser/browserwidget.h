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

#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H

#include <qwidget.h>

class ConnectionWidget;
class QTableView;
class QPushButton;
class QTextEdit;

class BrowserWidget: public QWidget
{
    Q_OBJECT
public:
    BrowserWidget(QWidget *parent = 0);
    virtual ~BrowserWidget();

public slots:
    void exec();
    void addConnection();

signals:
    void statusMessage(const QString &message);

private:
    QTextEdit *edit;
    QTableView *view;
    QPushButton *submitButton;
    ConnectionWidget *dbc;
};

#endif

