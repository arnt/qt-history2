#ifndef RESULTWINDOW_H
#define RESULTWINDOW_H

#include <qtable.h>
#include <qstring.h>
#include <qsqlcursor.h>
#include <qsqlfield.h>
#include <qsqldatabase.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>

#include "sqlbrowsewindow.h"

class ResultWindow : public SqlBrowseWindowBase
{
    Q_OBJECT
public:
    ResultWindow ( QWidget * parent=0, const char * name=0, WFlags f=0 );
    ~ResultWindow();
public slots:
    void slotExec();
    void newSelection( const QSqlRecord* fields );
private:
    QSqlDatabase* db;
    QSqlCursor sql;
};

//test

#endif



