#ifndef SETTINGSTREE_H
#define SETTINGSTREE_H

#include <QIcon>
#include <QTreeWidget>

class QSettings;

class SettingsTree : public QTreeWidget
{
    Q_OBJECT

public:
    SettingsTree(QWidget *parent = 0);

    void setSettings(QSettings *settings);
    QSize sizeHint() const;

public slots:
    void sync();

private:
    void updateChildItems(QTreeWidgetItem *parent);
    QTreeWidgetItem *createItem(const QString &text, QTreeWidgetItem *parent);
    QTreeWidgetItem *childAt(QTreeWidgetItem *parent, int index);
    int childCount(QTreeWidgetItem *parent);
    int findChild(QTreeWidgetItem *parent, const QString &text, int startIndex);
    void moveItemForward(QTreeWidgetItem *parent, int oldIndex, int newIndex);

    QSettings *settings;
    QIcon groupIcon;
    QIcon keyIcon;
};

#endif
