#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qvariant.h>
#include <qwidget.h>
#include "dbconnection.h"

class QLabel;
class QListView;
class QListViewItem;
class TableInfo;
class QWidgetStack;
class QIconView;
class MainWindow : public QWidget
{ 
    Q_OBJECT

public:
    MainWindow( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~MainWindow();

private:
    QListViewItem* dbItem;
    QListViewItem* tablesFolder;
    QListViewItem* viewsFolder;
    QListViewItem* tableSpacesFolder;
    QListViewItem* usersFolder;
    QListViewItem* indexesFolder;

    QListView* DBView;
    QWidgetStack* detailsStack;
    TableInfo* tableInfo;
    QLabel* viewInfo;
    QLabel* indexInfo;
    QLabel* userInfo;
    QLabel* tableSpaceInfo;
    QIconView* iconView;

    DBConnection* dbConnection;

    void populate( void );

    QMap< QListViewItem*, ObjectInfo > objectMap;
    ObjectInfo popupInfo;
    QListViewItem* popupItem;

protected slots:
    void displayObject( QListViewItem* );
    void dbContext( QListViewItem*, const QPoint&, int );
    // popup menu slots
    void defragDatabase();
    void moveTable();
};

#endif // MAINWINDOW_H
