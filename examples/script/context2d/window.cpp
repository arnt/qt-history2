/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "window.h"
#include "qcontext2dcanvas.h"
#include <QHBoxLayout>
#include <QListWidget>
#include <QDir>

static QString scriptsDir()
{
    if (QFile::exists("./scripts"))
        return "./scripts";
    return ":/scripts";
}

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);
    QListWidget *view = new QListWidget(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    canvas = new QContext2DCanvas(this);
    canvas->setFixedSize(150, 150);
    hbox->addWidget(view);
    hbox->addWidget(canvas);

    QDir dir(scriptsDir());
    QFileInfoList entries = dir.entryInfoList(QStringList() << "*.js");
    for (int i = 0; i < entries.size(); ++i)
        view->addItem(entries.at(i).fileName());
    connect(view, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(selectScript(QListWidgetItem*)));

    setWindowTitle(tr("Context 2D"));
}

void Window::selectScript(QListWidgetItem *item)
{
    QFile file(scriptsDir() + QDir::separator() + item->text());
    file.open(QIODevice::ReadOnly);
    QString contents = file.readAll();
    file.close();
    canvas->setScriptContents(contents);
}
