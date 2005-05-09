/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QString>
#include <QTabWidget>

class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    GeneralTab(const QFileInfo &fileInfo, QWidget *parent = 0);
};


class PermissionsTab : public QWidget
{
    Q_OBJECT

public:
    PermissionsTab(const QFileInfo &fileInfo, QWidget *parent = 0);
};


class ApplicationsTab : public QWidget
{
    Q_OBJECT

public:
    ApplicationsTab(const QFileInfo &fileInfo, QWidget *parent = 0);
};


class TabDialog : public QDialog
{
    Q_OBJECT

public:
    TabDialog(const QString &fileName, QWidget *parent = 0);

private:
    QTabWidget *tabWidget;
};

#endif
