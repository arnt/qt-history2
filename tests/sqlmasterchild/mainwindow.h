#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qstring.h>
#include <qsqldatabase.h>
#include <qsqlfield.h>
#include <qsqlrecord.h>
#include <qsqlcursor.h>

#include "masterchildwindow.h"

class MainWindow : public MasterChildWindowBase
{
    Q_OBJECT
public:
    MainWindow ( QWidget * parent=0, const char * name=0, WFlags f=0 );
    ~MainWindow();
protected slots:
    void newMasterSelection( const QSqlRecord* fields );
};

#endif
