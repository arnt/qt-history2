#ifndef DBTEST_H
#define DBTEST_H

#include <qstring.h>
#include <qwidget.h>

class QSqlDatabase;
class QListView;
class QDataTable;
class QListViewItem;
class QTextEdit;

class SqlEx: public QWidget
{
    Q_OBJECT

public:

    SqlEx( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    virtual ~SqlEx();

    bool setConnection( QString conName );

private:
    QString con;
    QListView* lv;
    QDataTable* dt;
    QTextEdit* te;

private slots:
    void showTable( QListViewItem * item );
    void execQuery();

};

#endif //DBTEST_H
