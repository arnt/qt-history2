
#ifndef TST_ABSTRACTFORMWINDOW_H
#define TST_ABSTRACTFORMWINDOW_H

#include <qtestcase.h>

//TESTED_CLASS=
//TESTED_FILES=ide/src/sdk/abstractformwindow.h ide/src/sdk/abstractformwindow.cpp

class tst_AbstractFormWindow: public QTestCase
{
    Q_OBJECT
public:
    tst_AbstractFormWindow(int &argc, char *argv[]);
    virtual ~tst_AbstractFormWindow();

private slots:
    void dirty_data(QTestTable &t);
    void dirty();

    void mainContainer_data(QTestTable &t);
    void mainContainer();
};


#endif // TST_ABSTRACTFORMWINDOW_H
