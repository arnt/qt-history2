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
#include <qdebug.h>

class AbstractFormEditor;

class PluginPreferenceWidget : public QWidget
{
    Q_OBJECT
public:
    PluginPreferenceWidget(PluginPreferences *prefs, QWidget *parent = 0);
    
public slots:
    void addPath();
    void removePath();
    
private slots:
    void populateTree();
    void updateButtons();
    void itemChanged(QTreeWidgetItem *item, int col);

private:        
    QTreeWidget *m_tree;
    QPushButton *m_add_path_button, *m_remove_path_button;

    PluginPreferences *m_prefs;
};

#include "pluginpreferences.moc"

PluginPreferenceWidget::PluginPreferenceWidget(PluginPreferences *prefs, QWidget *parent)
    : QWidget(parent)
{
    m_prefs = prefs;
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    m_tree = new QTreeWidget(this);
    mainLayout->addWidget(m_tree);
    
    QHBoxLayout *hLayout = new QHBoxLayout(mainLayout);
    m_add_path_button = new QPushButton(tr("Add path..."), this);
    connect(m_add_path_button, SIGNAL(clicked()), this, SLOT(addPath()));
    hLayout->addWidget(m_add_path_button);
    m_remove_path_button = new QPushButton(tr("Remove path"), this);
    connect(m_remove_path_button, SIGNAL(clicked()), this, SLOT(removePath()));
    hLayout->addWidget(m_remove_path_button);
    m_remove_path_button->setEnabled(false);
    
    connect(m_tree, SIGNAL(itemSelectionChanged()),
                this, SLOT(updateButtons()));
    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
                this, SLOT(itemChanged(QTreeWidgetItem*, int)));
    
    populateTree();
}

void PluginPreferenceWidget::itemChanged(QTreeWidgetItem *item, int col)
{
    if (col != 1)
        return;
    if (item->parent() == 0)
        return;
        
    QString plugin = QDir::cleanPath(item->parent()->text(0) + "/" + item->text(0));

    if (item->checkState(1) != Qt::Checked) {
        if (!m_prefs->m_disabled_plugins.contains(plugin))
            m_prefs->m_disabled_plugins.append(plugin);
    } else {
        m_prefs->m_disabled_plugins.removeAll(plugin);
    }
    
    m_prefs->m_dirty = true;
    emit m_prefs->changed();
}

void PluginPreferenceWidget::populateTree()
{
    disconnect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
                this, SLOT(itemChanged(QTreeWidgetItem*, int)));
    m_tree->clear();
    
    m_tree->setColumnCount(2);
    m_tree->setHeaderLabels(QStringList() << tr("Path") << tr("Enabled"));

    foreach (QString path, m_prefs->m_plugin_paths) {
        QTreeWidgetItem *path_item = new QTreeWidgetItem(m_tree);
        path_item->setText(0, path);
        m_tree->setItemExpanded(path_item, true);
        
        QStringList plugin_list = m_prefs->m_plugin_manager->findPlugins(path);
        
        foreach (QString plugin, plugin_list) {
            QTreeWidgetItem *item = new QTreeWidgetItem(path_item);
            QFileInfo fi(plugin);
            item->setText(0, fi.fileName());
            bool enabled = !m_prefs->m_disabled_plugins.contains(plugin);
            item->setCheckState(1, enabled ? Qt::Checked : Qt::Unchecked);
        }
    }

    m_tree->resizeColumnToContents(0);
    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SLOT(itemChanged(QTreeWidgetItem*, int)));
    updateButtons();
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

    path = QDir::cleanPath(path);
        
    if (!m_prefs->m_plugin_paths.contains(path)) {
        m_prefs->m_plugin_paths.append(path);
        populateTree();
    }

    m_prefs->m_dirty = true;
    emit m_prefs->changed();
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

    if (m_prefs->m_plugin_paths.contains(path)) {
        m_prefs->m_plugin_paths.removeAll(path);
        populateTree();
    }

    m_prefs->m_dirty = true;
    emit m_prefs->changed();
}

// PluginPreferences class implementation

PluginPreferences::PluginPreferences(PluginManager *plugin_manager, QObject *parent)
    : PreferenceInterface(parent)
{
    m_dirty = false;
    m_plugin_manager = plugin_manager;
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
    m_plugin_manager->setPluginPaths(m_plugin_paths);
    m_plugin_manager->setDisabledPlugins(m_disabled_plugins);
    m_plugin_manager->syncSettings();
    m_dirty = false;
    
    return true;
}

bool PluginPreferences::readSettings()
{
    m_plugin_paths = m_plugin_manager->pluginPaths();
    m_disabled_plugins = m_plugin_manager->disabledPlugins();
    
    m_dirty = false;
    emit updateWidget();
    
    return true;
}

QWidget *PluginPreferences::createPreferenceWidget(QWidget *parent)
{
    PluginPreferenceWidget *w = new PluginPreferenceWidget(this, parent);
    connect(this, SIGNAL(updateWidget()), w, SLOT(populateTree()));
    return w;
}
