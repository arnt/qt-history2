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

#include <qevent.h>

class PluginDelegate : public QItemDelegate
{
public:
    PluginDelegate(QObject *parent);
    ~PluginDelegate();

    enum Roles {
        CheckedRole = QAbstractItemModel::DecorationRole
    };

    bool editorEvent(QEvent *event, const QStyleOptionViewItem &option, const QModelIndex &index);
};

PluginDelegate::PluginDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

PluginDelegate::~PluginDelegate()
{
}

bool PluginDelegate::editorEvent(QEvent *event, const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{
    bool typeOk = event && (event->type() == QEvent::MouseButtonPress
                            || event->type() == QEvent::MouseButtonDblClick);
    if (!typeOk || index.column() != 0)
        return QItemDelegate::editorEvent(event, option, index);

    if ((static_cast<QMouseEvent*>(event)->x() - option.rect.x()) < 20) {
        const QAbstractItemModel *model = index.model();
        bool checked = model->data(index, CheckedRole).toBool();
        const_cast<QAbstractItemModel*>(model)->setData(index, !checked, CheckedRole);
        return true;
    }
    return false;
}

class PluginPreferenceWidget : public QWidget
{
    Q_OBJECT
public:
    PluginPreferenceWidget(QWidget *parent = 0);

signals:
    void addPath(const QString &path);
    void removePath(const QString &path);
    void setPluginPathEnabled(const QString &path, bool enable);

public slots:
    void setupPlugins(const QStringList &registeredPlugins);

private slots:
    void addThisPath();
    void removeThisPath();
    void handleItemChanged(QTreeWidgetItem *item, int column);
    void changeButtonStatus(QTreeWidgetItem *item);

private:
    void addItem(const QString &str);
    QTreeWidget *m_tree;
    QPushButton *cmdRemove;
};

#include "pluginpreferences.moc"

PluginPreferenceWidget::PluginPreferenceWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    m_tree = new QTreeWidget(this);
    m_tree->setItemDelegate(new PluginDelegate(m_tree));
    m_tree->setColumnCount(2);
    m_tree->setHeaderLabels(QStringList() << tr("Enabled") << tr("Plugin") << tr("Path"));
    m_tree->header()->setResizeMode(QHeaderView::Custom, 2);
    m_tree->header()->resizeSection(2, 700);
    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this, SLOT(handleItemChanged(QTreeWidgetItem *, int)));
    connect(m_tree, SIGNAL(currentChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(changeButtonStatus(QTreeWidgetItem *)));
    mainLayout->addWidget(m_tree);
    QVBoxLayout *vLayout = new QVBoxLayout(mainLayout);
    QPushButton *cmd = new QPushButton(tr("Add path..."), this);
    connect(cmd, SIGNAL(clicked()), this, SLOT(addThisPath()));
    vLayout->addWidget(cmd);
    cmdRemove = new QPushButton(tr("Remove path"), this);
    cmdRemove->setEnabled(false);
    connect(cmdRemove, SIGNAL(clicked()), this, SLOT(removeThisPath()));
    vLayout->addWidget(cmdRemove);
}

void PluginPreferenceWidget::setupPlugins(const QStringList &registeredPlugins)
{
    m_tree->clear();  // yuck! Should at least remember what is open.
    foreach(QString str, registeredPlugins)
        addItem(str);
}

void PluginPreferenceWidget::addThisPath()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                                     tr("Add a directory to plugin searching"));
    if (path.isEmpty())
        return;
    addItem(path);
    emit addPath(path);
}

void PluginPreferenceWidget::addItem(const QString &path)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(m_tree);
    item->setData(0, PluginDelegate::CheckedRole, true);
    int lastSlash = path.lastIndexOf('/');
    QString realPath = path;
    if (lastSlash != -1) {
        QString pluginName = path.mid(lastSlash + 1);
        item->setText(1, pluginName);
        realPath = path.left(lastSlash);
    }
    item->setText(2, realPath);
}

void PluginPreferenceWidget::removeThisPath()
{
    QTreeWidgetItem *item = m_tree->currentItem();
    if (!item)
        return;
    QString path = item->text(2) + "/" + item->text(1);
    m_tree->takeTopLevelItem(m_tree->indexOfTopLevelItem(item));
    delete item;
    emit removePath(path);
}

void PluginPreferenceWidget::handleItemChanged(QTreeWidgetItem *item, int column)
{
    if (!item || column != 0)
        return;
    emit setPluginPathEnabled(item->text(2) + "/" + item->text(1), item->checkedState(0) == QCheckBox::On);
}

void PluginPreferenceWidget::changeButtonStatus(QTreeWidgetItem *item)
{
    cmdRemove->setEnabled(item != 0);
}

// PluginPreferences class implementation

PluginPreferences::PluginPreferences(QObject *parent)
    : QObject(parent), m_dirty(false)
{
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
    return m_pluginManager.syncSettings();
}

bool PluginPreferences::readSettings()
{
    emit setupPlugins(m_pluginManager.registeredPlugins());
    return true;
}

void PluginPreferences::addPath(const QString &path)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_dirty = true;
    m_pluginManager.addPluginPath(path);
    m_pluginManager.registerPath(path);
    emit setupPlugins(m_pluginManager.registeredPlugins());
    QApplication::restoreOverrideCursor();
}

void PluginPreferences::removePath(const QString &path)
{
    m_dirty = true;
    m_pluginManager.removePluginPath(path);
}

void PluginPreferences::setPluginPathEnabled(const QString &plugin, bool enable)
{
    m_dirty = true;
    if (enable)
        m_pluginManager.registerPlugin(plugin);
    else
        m_pluginManager.unregisterPlugin(plugin);
}

QWidget *PluginPreferences::createPreferenceWidget(QWidget *parent)
{
    PluginPreferenceWidget *pref = new PluginPreferenceWidget(parent);
    connect(this, SIGNAL(setupPlugins(const QStringList &)),
            pref, SLOT(setupPlugins(const QStringList &)));
    connect(pref, SIGNAL(addPath(const QString &)), this, SLOT(addPath(const QString &)));
    connect(pref, SIGNAL(removePath(const QString &)), this, SLOT(removePath(const QString &)));
    connect(pref, SIGNAL(setPluginPathEnabled(const QString &, bool)),
            this, SLOT(setPluginPathEnabled(const QString &, bool)));
    return pref;
}
