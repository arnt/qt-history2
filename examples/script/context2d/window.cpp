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
#include <QListView>
#include <QDir>
#include <QStandardItemModel>

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
    QListView *view = new QListView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    canvas = new QContext2DCanvas(this);
    canvas->setFixedSize(150, 150);
    hbox->addWidget(view);
    hbox->addWidget(canvas);

    QDir dir(scriptsDir());
    QFileInfoList entries = dir.entryInfoList(QStringList() << "*.js");
    scripts = new QStandardItemModel(this);
    for (int i = 0; i < entries.size(); ++i) {
        QString name = entries.at(i).fileName();
        scripts->appendRow(new QStandardItem(name));
    }
    view->setModel(scripts);
    connect(view, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(selectScript(const QModelIndex &)));
}

void Window::selectScript(const QModelIndex &index)
{
    QStandardItem *item = scripts->itemFromIndex(index);
    QFile file(scriptsDir() + "/" + item->text());
    file.open(QIODevice::ReadOnly);
    QString contents = file.readAll();
    file.close();
    canvas->setScriptContents(contents);
}
