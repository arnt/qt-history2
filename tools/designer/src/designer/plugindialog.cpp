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
    const QStringList fileNames = core()->pluginManager()->registeredPlugins();

    if (!fileNames.isEmpty()) {
        QTreeWidgetItem *topLevelItem = setTopLevelItem(QLatin1String("Loaded Plugins"));
        QFont boldFont = topLevelItem->font(0);

        foreach (QString fileName, fileNames) {
            QPluginLoader loader(fileName);
            QFileInfo fileInfo(fileName);

            QTreeWidgetItem *pluginItem = setPluginItem(topLevelItem, fileInfo.fileName(), boldFont);

            QObject *plugin = loader.instance();
            if (plugin != 0) {
                QDesignerCustomWidgetCollectionInterface *c = qobject_cast<QDesignerCustomWidgetCollectionInterface*>(plugin);
                if (c != 0) {
                    foreach (QDesignerCustomWidgetInterface *p, c->customWidgets())
                        setItem(pluginItem, p->name(), p->toolTip(), p->whatsThis(), p->icon()); 
                }

                QDesignerCustomWidgetInterface *p = qobject_cast<QDesignerCustomWidgetInterface*>(plugin);
                if (p != 0) {
                    setItem(pluginItem, p->name(), p->toolTip(), p->whatsThis(), p->icon());
                }
            }
        }
    }

    const QStringList notLoadedPlugins = core()->pluginManager()->failedPlugins();
    if (!notLoadedPlugins.isEmpty()) {

        QTreeWidgetItem *topLevelItem = setTopLevelItem(QLatin1String("Failed Plugins"));
        QFont boldFont = topLevelItem->font(0);
        foreach (const QString plugin, notLoadedPlugins)
        {
            QTreeWidgetItem *pluginItem = setPluginItem(topLevelItem, plugin, boldFont);
            setItem(pluginItem, core()->pluginManager()->failureReason(plugin), QLatin1String(""), QLatin1String(""), QIcon());
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
        return QIcon(QLatin1String(":/trolltech/formeditor/images/qtlogo.png"));

    return icon;
}

QTreeWidgetItem* PluginDialog::setTopLevelItem(const QString &itemName)
{
    QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(ui.treeWidget);
    topLevelItem->setText(0, itemName);
    ui.treeWidget->setItemExpanded(topLevelItem, true);
    topLevelItem->setIcon(0, style()->standardPixmap(QStyle::SP_DirOpenIcon));

    QFont boldFont = topLevelItem->font(0);
    boldFont.setBold(true);
    topLevelItem->setFont(0, boldFont);

    return topLevelItem;
}

QTreeWidgetItem* PluginDialog::setPluginItem(QTreeWidgetItem *topLevelItem, 
                                             const QString &itemName, const QFont &font)
{
    QTreeWidgetItem *pluginItem = new QTreeWidgetItem(topLevelItem);
    pluginItem->setFont(0, font);
    pluginItem->setText(0, itemName);
    ui.treeWidget->setItemExpanded(pluginItem, true);
    pluginItem->setIcon(0, style()->standardPixmap(QStyle::SP_DirOpenIcon));

    return pluginItem;
}

void PluginDialog::setItem(QTreeWidgetItem *pluginItem, const QString &name, 
                           const QString &toolTip, const QString &whatsThis, const QIcon &icon)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(pluginItem);
    item->setText(0, name);
    item->setToolTip(0, toolTip);
    item->setWhatsThis(0, whatsThis);
    item->setIcon(0, pluginIcon(icon));
}
