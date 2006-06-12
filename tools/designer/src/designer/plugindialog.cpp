/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtDesigner/QtDesigner>
#include <pluginmanager_p.h>

#include <QtGui/QtGui>

#include "plugindialog.h"

PluginDialog::PluginDialog(QDesignerFormEditorInterface *core, QWidget *parent)
    : QDialog(parent
#ifdef Q_WS_MAC
            , Qt::Tool
#endif
            ), m_core(core)
{
    ui.setupUi(this);

    ui.message->hide();

    QStringList headerLabels;
    headerLabels << tr("Components");

    ui.treeWidget->setAlternatingRowColors(false);
    ui.treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui.treeWidget->setHeaderLabels(headerLabels);
    ui.treeWidget->header()->hide();

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

        QTreeWidgetItem *pluginItem = new QTreeWidgetItem(ui.treeWidget);
        pluginItem->setText(0, fileInfo.fileName());
        pluginItem->setIcon(0, style()->standardPixmap(QStyle::SP_DirOpenIcon));
        ui.treeWidget->setItemExpanded(pluginItem, true);

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
                    item->setToolTip(0, p->toolTip());
                    item->setWhatsThis(0, p->whatsThis());
                }
            }

            QDesignerCustomWidgetInterface *p = qobject_cast<QDesignerCustomWidgetInterface*>(plugin);
            if (p != 0) {
                QTreeWidgetItem *item = new QTreeWidgetItem(pluginItem);
                item->setText(0, p->name());
                item->setIcon(0, pluginIcon(p->icon()));
                item->setToolTip(0, p->toolTip());
                item->setWhatsThis(0, p->whatsThis());
            }
        }
    }

    if (ui.treeWidget->topLevelItemCount() == 0) {
        ui.label->setText(tr("Qt Designer couldn't find any plugins"));
        ui.treeWidget->hide();
    } else {
        ui.label->setText(tr("Qt Designer found the following plugins"));
    }
}

QIcon PluginDialog::pluginIcon(const QIcon &icon)
{
    if (icon.isNull())
        return QIcon(":/trolltech/formeditor/images/qtlogo.png");

    return icon;
}

