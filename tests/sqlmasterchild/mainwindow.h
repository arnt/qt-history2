#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qstring.h>
#include <qsqldatabase.h>
#include <qsqlfield.h>
#include <qsqlview.h>

#include "masterchildwindow.h"

class MainWindow : public MasterChildWindowBase
{
    Q_OBJECT
public:
    MainWindow ( QWidget * parent=0, const char * name=0, WFlags f=0 );
    ~MainWindow();
public slots:
    void newMasterSelection( const QSqlFieldList* fields );
protected:
    void reloadChildTable( int masterIdx );
private:
    QSqlView masterView;
    QSqlView childView;
};

#endif



