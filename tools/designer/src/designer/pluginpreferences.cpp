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
#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QFileDialog>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QItemDelegate>
#include "pluginpreferences.h"

#include <abstractformeditor.h>
#include <pluginmanager.h>

#include <qevent.h>

class AbstractFormEditor;

class PluginPreferenceWidget : public QWidget
{
    Q_OBJECT
public:
    PluginPreferenceWidget(PluginManager *pluginManager, QWidget *parent = 0);

public slots:
    void addPath();
    void removePath();
    
private slots:
    void populateTree();
    void updateButtons();

private:        
    QTreeWidget *m_tree;
    QPushButton *m_add_path_button, *m_remove_path_button;
    PluginManager *m_pluginManager;
};

#include "pluginpreferences.moc"

PluginPreferenceWidget::PluginPreferenceWidget(PluginManager *pluginManager, QWidget *parent)
    : QWidget(parent)
{
    m_pluginManager = pluginManager;
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    m_tree = new QTreeWidget(this);
    mainLayout->addWidget(m_tree);
    
    QVBoxLayout *vLayout = new QVBoxLayout(mainLayout);
    m_add_path_button = new QPushButton(tr("Add path..."), this);
    connect(m_add_path_button, SIGNAL(clicked()), this, SLOT(addPath()));
    vLayout->addWidget(m_add_path_button);
    m_remove_path_button = new QPushButton(tr("Remove path"), this);
    connect(m_remove_path_button, SIGNAL(clicked()), this, SLOT(removePath()));
    vLayout->addWidget(m_remove_path_button);
    m_remove_path_button->setEnabled(false);
    
    connect(m_tree, SIGNAL(itemSelectionChanged()),
                this, SLOT(updateButtons()));
    
    populateTree();
}

void PluginPreferenceWidget::populateTree()
{
    m_tree->clear();
    
    m_tree->setColumnCount(2);
    m_tree->setHeaderLabels(QStringList() << tr("Path") << tr("Enabled"));
    
    QStringList plugin_list = m_pluginManager->registeredPlugins();
    plugin_list.sort();
    
    QStringList path_list = m_pluginManager->pluginPaths();
    
    QTreeWidgetItem *cur_item = 0;
    foreach (QString plugin, plugin_list) {
        QFileInfo fi(plugin);
        QString path = fi.absolutePath();
        QString file = fi.fileName();
        
        if (cur_item == 0 || cur_item->text(0) != path) {
            cur_item = new QTreeWidgetItem(m_tree);
            cur_item->setText(0, path);
            cur_item->setData(1, QAbstractItemModel::CheckStateRole, true);
            m_tree->setItemOpen(cur_item, true);
            path_list.removeAll(path);
        }
        
        QTreeWidgetItem *item = new QTreeWidgetItem(cur_item);
        item->setText(0, file);
        item->setData(1, QAbstractItemModel::CheckStateRole, true);
    }
    
    foreach (QString path, path_list) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_tree);
        item->setText(0, path);
        item->setData(1, QAbstractItemModel::CheckStateRole, true);
    }

    m_tree->resizeColumnToContents(0);
}

void PluginPreferenceWidget::updateButtons()
{
    QList<QTreeWidgetItem*> sel_item_list = m_tree->selectedItems();
    if (sel_item_list.isEmpty()) {
        m_remove_path_button->setEnabled(false);
    } else {
        QTreeWidgetItem *item = sel_item_list.first();
        m_remove_path_button->setEnabled(item->parent() == 0);
    }
}

void PluginPreferenceWidget::addPath()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                                     tr("Add plugin directory"));
    if (path.isEmpty())
        return;

    m_pluginManager->addPluginPath(path);
    
    populateTree();
}

void PluginPreferenceWidget::removePath()
{
    QList<QTreeWidgetItem*> sel_item_list = m_tree->selectedItems();
    if (sel_item_list.isEmpty())
        return;
    QTreeWidgetItem *item = sel_item_list.first();
    if (item->parent() != 0)
        return;
    QString path = item->text(0);
    m_pluginManager->removePluginPath(path);
    
    populateTree();
}

// PluginPreferences class implementation

PluginPreferences::PluginPreferences(PluginManager *pluginManager, QObject *parent)
    : QObject(parent), m_dirty(false)
{
    m_pluginManager = pluginManager;
}

PluginPreferences::~PluginPreferences()
{
}

bool PluginPreferences::settingsChanged() const
{
    return m_dirty;
}

QString PluginPreferences::preferenceName() const
{
    return tr("Plugin Preferences");
}

QIcon PluginPreferences::preferenceIcon() const
{
    return QIcon();
}

bool PluginPreferences::saveSettings()
{
    return true;
}

bool PluginPreferences::readSettings()
{
    return true;
}

QWidget *PluginPreferences::createPreferenceWidget(QWidget *parent)
{
    PluginPreferenceWidget *pref = new PluginPreferenceWidget(m_pluginManager, parent);
    return pref;
}
