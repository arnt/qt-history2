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

#include <QtCore/QSettings>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QSplitter>

#include <QtDesigner/abstractformeditor.h>

#include "preferencedialog.h"
#include "qdesigner_preferences.h"  // Someday we'll need to load most of these dynamically.
#include "pluginpreferences.h"

PreferenceDialog::PreferenceDialog(QDesignerFormEditorInterface *core, QWidget *parent)
    : QDialog(parent)
{
    m_core = core;

    setWindowTitle(tr("Qt Designer Preferences"));
    PreferenceInterface *iface = new DesignerPreferences(this);
    m_preferences.append(iface);

    iface = new PluginPreferences(m_core->pluginManager(), this);
    m_preferences.append(iface);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QSplitter *splitter = new QSplitter(this);
    mainLayout->addWidget(splitter);
    m_treeWidget = new QTreeWidget(splitter);
    m_stack = new QStackedWidget(splitter);
    m_treeWidget->setColumnCount(1);
    m_treeWidget->header()->setResizeMode(QHeaderView::Stretch);
    m_treeWidget->header()->hide();
    QTreeWidgetItem *root = new QTreeWidgetItem(m_treeWidget);
    root->setText(0, tr("Standard Preferences"));
    m_treeWidget->setItemOpen(root, true);
    connect(m_treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(changePane(QTreeWidgetItem *)));
    QTreeWidgetItem *item;
    for (int listIndex = 0; listIndex < m_preferences.size(); ++listIndex) {
        iface = m_preferences.at(listIndex);
        int index = m_stack->addWidget(iface->createPreferenceWidget(0));
        item = new QTreeWidgetItem(root);
        item->setText(0, iface->preferenceName());
        QIcon icon = iface->preferenceIcon();
        if (!icon.isNull())
            item->setIcon(0, icon);
        item->setText(1, QString::number(index));
        item->setText(2, QString::number(listIndex));
    }
    // add in all the dynamic things into the list here
    // ...

    QSettings settings;
    QList<QVariant> openItemList;
    openItemList.append(QVariant(0));
    openItemList = settings.value("preferenceDialog/openItems", openItemList).toList();
    foreach (QVariant v, openItemList) {
        item = m_treeWidget->topLevelItem(v.toInt());
        if (item)
            m_treeWidget->openItem(item);
    }

    QTreeWidgetItem *topLevelItem = m_treeWidget->topLevelItem(
                                        settings.value("preferenceDialog/parentIndex", 0).toInt());
    if (topLevelItem) {
        item = topLevelItem->child(settings.value("preferenceDialog/childIndex", 0).toInt());
        if (item) {
            m_treeWidget->setSelected(item, true);
            m_treeWidget->setCurrentItem(item);
        }
    }

    QBoxLayout *layout = new QHBoxLayout();
    mainLayout->addLayout(layout);
    layout->addStretch();
    QPushButton *cmdOK = new QPushButton(tr("OK"), this);
    cmdOK->setDefault(true);
    connect(cmdOK, SIGNAL(clicked()), this, SLOT(accept()));
    QPushButton *cmdCancel = new QPushButton(tr("Cancel"), this);
    connect(cmdCancel, SIGNAL(clicked()), this, SLOT(reject()));
#ifdef Q_WS_MAC
    layout->addWidget(cmdCancel);
    layout->addWidget(cmdOK);
#else
    layout->addWidget(cmdOK);
    layout->addWidget(cmdCancel);
#endif
}

PreferenceDialog::~PreferenceDialog()
{
    qDeleteAll(m_preferences);

    QSettings settings;
    QTreeWidgetItem *parentItem;
    QTreeWidgetItem *item = m_treeWidget->currentItem();
    if (item)
        parentItem = item->parent();
    if (item && parentItem)
        settings.setValue("preferenceDialog/childIndex", parentItem->indexOfChild(item));
    // now go through the top level items and make sure we remember which ones were opened
    QList<QVariant> list;
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        item = m_treeWidget->topLevelItem(i);
        if (item == parentItem)
            settings.setValue("preferenceDialog/parentIndex", i);
        if (m_treeWidget->isItemOpen(item))
            list.append(i);
    }
    settings.setValue("preferenceDialog/openItems", list);
}

void PreferenceDialog::changePane(QTreeWidgetItem *item)
{
    if (!item || !item->parent())
        return;

    m_stack->setCurrentIndex(item->text(1).toInt());
    PreferenceInterface *iface = m_preferences.at(item->text(2).toInt());
    if (!iface->settingsChanged())
        iface->readSettings();
}

void PreferenceDialog::reject()
{
    QDialog::reject();
}

void PreferenceDialog::accept()
{
    foreach (PreferenceInterface *iface, m_preferences) {
        if (iface->settingsChanged())
            iface->saveSettings();
    }
    QDialog::accept();
}

