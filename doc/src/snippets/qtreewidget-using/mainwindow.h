#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

class QAction;
class QTreeWidget;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void insertItem();
    void removeItem();
    void sortAscending();
    void sortDescending();
    void updateMenus(QTreeWidgetItem *current);

private:
    void setupTreeItems();

    QAction *removeAction;
    QTreeWidget *treeWidget;
};

#endif
