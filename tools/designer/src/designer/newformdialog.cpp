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

#include "newformdialog.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>

NewFormDialog::NewFormDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Create New Form"));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mWidgetTree = new QTreeWidget(this);
    mWidgetTree->setColumnCount(1);
    mWidgetTree->header()->setResizeMode(QHeaderView::Stretch);
    mWidgetTree->header()->hide();

    QTreeWidgetItem *root = new QTreeWidgetItem(mWidgetTree);
    root->setText(0, tr("Standard Templates"));
    mWidgetTree->openItem(root);
    QTreeWidgetItem *item = new QTreeWidgetItem(root);
    item->setText(0, tr("Dialog"));
    item->setText(1, tr("QDialog"));
    item = new QTreeWidgetItem(root);
    item->setText(0, tr("Widget"));
    item->setText(1, tr("QWidget"));
    mWidgetTree->setSelected(item, true);
    mWidgetTree->setCurrentItem(item);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btn = new QPushButton(tr("&Open..."), this);
    connect(btn, SIGNAL(clicked()), this, SIGNAL(needOpen()));
    buttonLayout->addWidget(btn);
    btnCreate = new QPushButton(tr("&Create"), this);
    btnCreate->setEnabled(false);
    connect(btnCreate, SIGNAL(clicked()), this, SLOT(createThisOne()));
    buttonLayout->addSpacing(1);
    buttonLayout->addWidget(btnCreate);
    btn = new QPushButton(tr("Cancel"), this);
    connect(btn, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(btn);

    mainLayout->addWidget(mWidgetTree);
    mainLayout->addLayout(buttonLayout);

    connect(mWidgetTree, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
            this, SLOT(fixButton(QTreeWidgetItem*)));
    connect(mWidgetTree, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(handleDoubleClick(QTreeWidgetItem*)));
    fixButton(item);
}

NewFormDialog::~NewFormDialog()
{
}

void NewFormDialog::handleClass(const QString &str)
{
    emit itemPicked(str);
    accept();
}

void NewFormDialog::handleDoubleClick(QTreeWidgetItem *item)
{
    if (item && item->parent())
        handleClass(item->text(1));

}

void NewFormDialog::createThisOne()
{
    if (QTreeWidgetItem *item = mWidgetTree->currentItem()) {
        if (item->parent())
            handleClass(item->text(1));
    }
}

void NewFormDialog::fixButton(QTreeWidgetItem *item)
{
    bool enable = ((item && item->parent())
                    || (mWidgetTree->currentItem() && mWidgetTree->currentItem()->parent()));
    btnCreate->setDefault(enable);
    btnCreate->setEnabled(enable);
}
