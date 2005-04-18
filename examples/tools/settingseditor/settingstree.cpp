#include <QtGui>

#include "settingstree.h"
#include "variantdelegate.h"

SettingsTree::SettingsTree(QWidget *parent)
    : QTreeWidget(parent)
{
    QStringList labels;
    labels << tr("Setting") << tr("Value");
    setHeaderLabels(labels);

    setItemDelegate(new VariantDelegate(this));

    settings = 0;

    groupIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
                        QIcon::Normal, QIcon::Off);
    groupIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
                        QIcon::Normal, QIcon::On);
    keyIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
}

void SettingsTree::setSettings(QSettings *settings)
{
    delete this->settings;
    this->settings = settings;
    clear();
    if (!settings)
        return;

    settings->setParent(this);
    updateChildItems(0);
}

QSize SettingsTree::sizeHint() const
{
    return QSize(500, 400);
}

void SettingsTree::sync()
{
    if (settings) {
        settings->sync();
        updateChildItems(0);
    }
}

void SettingsTree::updateChildItems(QTreeWidgetItem *parent)
{
    int dividerIndex = 0;

    foreach (QString group, settings->childGroups()) {
        QTreeWidgetItem *child;
        int childIndex = findChild(parent, group, dividerIndex);
        if (childIndex != -1) {
            child = childAt(parent, childIndex);
            // ### remove other fields
        } else {
            childIndex = childCount(parent);
            child = createItem(group, parent);
        }
        child->setIcon(0, groupIcon);
        moveItemForward(parent, childIndex, dividerIndex);
        ++dividerIndex;

        settings->beginGroup(group);
        updateChildItems(child);
        settings->endGroup();
    }

    foreach (QString key, settings->childKeys()) {
        QTreeWidgetItem *child;
        int childIndex = findChild(parent, key, 0);

        if (childIndex == -1 || childIndex >= dividerIndex) {
            if (childIndex != -1) {
                child = childAt(parent, childIndex);
                // ### delete children
            } else {
                childIndex = childCount(parent);
                child = createItem(key, parent);
            }
            child->setIcon(0, keyIcon);
            moveItemForward(parent, childIndex, dividerIndex);
            ++dividerIndex;
        } else {
            child = childAt(parent, childIndex);
        }

        QVariant value = settings->value(key);
        child->setData(1, Qt::DisplayRole, value);
    }

    while (dividerIndex < childCount(parent))
        delete childAt(parent, dividerIndex);
}

QTreeWidgetItem *SettingsTree::createItem(const QString &text,
                                          QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item;
    if (parent)
        item = new QTreeWidgetItem(parent);
    else
        item = new QTreeWidgetItem(this);

    item->setText(0, text);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    return item;
}

QTreeWidgetItem *SettingsTree::childAt(QTreeWidgetItem *parent, int index)
{
    if (parent)
        return parent->child(index);
    else
        return topLevelItem(index);
}

int SettingsTree::childCount(QTreeWidgetItem *parent)
{
    if (parent)
        return parent->childCount();
    else
        return topLevelItemCount();
}

int SettingsTree::findChild(QTreeWidgetItem *parent, const QString &text,
                            int startIndex)
{
    for (int i = startIndex; i < childCount(parent); ++i) {
        if (childAt(parent, i)->text(0) == text)
            return i;
    }
    return -1;
}

void SettingsTree::moveItemForward(QTreeWidgetItem *parent, int oldIndex,
                                   int newIndex)
{
    if (newIndex == oldIndex) // ###
        return;

    Q_ASSERT(newIndex <= oldIndex);

qDebug("insertChild(%d, takeChild(%d)), childCount = %d", newIndex, oldIndex, childCount(parent));
    if (parent) {
        parent->insertChild(newIndex, parent->takeChild(oldIndex));
    } else {
        insertTopLevelItem(newIndex, takeTopLevelItem(oldIndex));
    }
qDebug("OK");
}
