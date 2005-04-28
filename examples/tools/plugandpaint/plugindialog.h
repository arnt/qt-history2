#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;
class QStringList;
class QTreeWidget;
class QTreeWidgetItem;

class PluginDialog : public QDialog
{
    Q_OBJECT

public:
    PluginDialog(const QString &path, const QStringList &fileNames,
                 QWidget *parent = 0);

private:
    void addItems(QTreeWidgetItem *pluginItem, const char *interfaceName,
                  const QStringList &features);

    QLabel *label;
    QTreeWidget *treeWidget;
    QPushButton *okButton;
};

#endif
