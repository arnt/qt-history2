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

#ifndef ABSTRACTSETTINGSDIALOG_H
#define ABSTRACTSETTINGSDIALOG_H

#include <QtGui/QMainWindow>
#include <QtCore/QHash>

class QAction;
class QActionGroup;

class AbstractSettingsDialog: public QMainWindow
{
    Q_OBJECT
public:
    AbstractSettingsDialog(QWidget *parent = 0);

    void addPage(QWidget *widget, const QIcon &icon);

private slots:
    void showSettings(QAction *action);

private:
    QToolBar *toolbar;
    QActionGroup *g;
    QHash<QAction*, QWidget*> pages;
};


#endif // ABSTRACTSETTINGSDIALOG_H
