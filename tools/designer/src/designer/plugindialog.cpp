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

#include <QtDesigner/QtDesigner>
#include <pluginmanager.h>

#include <QtGui/QtGui>

#include "plugindialog.h"

PluginDialog::PluginDialog(QDesignerFormEditorInterface *core, QWidget *parent)
    : QDialog(parent), m_core(core)
{
    label = new QLabel;
    label->setWordWrap(true);

    QStringList headerLabels;
    headerLabels << tr("Components");

    treeWidget = new QTreeWidget;
    treeWidget->setAlternatingRowColors(false);
    treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    treeWidget->setHeaderLabels(headerLabels);
    treeWidget->header()->hide();

    okButton = new QPushButton(tr("OK"));
    okButton->setDefault(true);

    connect(okButton, SIGNAL(clicked()), this, SLOT(close()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->addWidget(label, 0, 0, 1, 3);
    mainLayout->addWidget(treeWidget, 1, 0, 1, 3);
    mainLayout->addWidget(okButton, 2, 1);
    setLayout(mainLayout);

    interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
                            QIcon::Normal, QIcon::On);
    interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
                            QIcon::Normal, QIcon::Off);
    featureIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));

    setWindowTitle(tr("Plugin Information"));
    populateTreeWidget();
}

QDesignerFormEditorInterface *PluginDialog::core() const
{
    return m_core;
}

void PluginDialog::populateTreeWidget()
{
    QStringList fileNames = core()->pluginManager()->registeredPlugins();

    foreach (QString fileName, fileNames) {
        QPluginLoader loader(fileName);
        QFileInfo fileInfo(fileName);

        QObject *plugin = loader.instance();

        QTreeWidgetItem *pluginItem = new QTreeWidgetItem(treeWidget);
        pluginItem->setText(0, fileInfo.fileName());
        pluginItem->setIcon(0, style()->standardPixmap(QStyle::SP_DirOpenIcon));
        treeWidget->setItemExpanded(pluginItem, true);

        QFont boldFont = pluginItem->font(0);
        boldFont.setBold(true);
        pluginItem->setFont(0, boldFont);

        if (plugin != 0) {
            QDesignerCustomWidgetCollectionInterface *c = qobject_cast<QDesignerCustomWidgetCollectionInterface*>(plugin);
            if (c != 0) {
                foreach (QDesignerCustomWidgetInterface *p, c->customWidgets()) {
                    QTreeWidgetItem *item = new QTreeWidgetItem(pluginItem);
                    item->setText(0, p->name());
                    item->setIcon(0, pluginIcon(p->icon()));
                }
            }

            QDesignerCustomWidgetInterface *p = qobject_cast<QDesignerCustomWidgetInterface*>(plugin);
            if (p != 0) {
                QTreeWidgetItem *item = new QTreeWidgetItem(pluginItem);
                item->setText(0, p->name());
                item->setIcon(0, pluginIcon(p->icon()));
            }
        }
    }

    if (treeWidget->topLevelItemCount() == 0) {
        label->setText(tr("Qt Designer couldn't find any plugins"));
        treeWidget->hide();
    } else {
        label->setText(tr("Qt Designer found the following plugins"));
    }
}

QIcon PluginDialog::pluginIcon(const QIcon &icon)
{
    if (icon.isNull())
        return QIcon(":/trolltech/formeditor/images/qtlogo.png");

    return icon;
}

