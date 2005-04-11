
#ifndef TST_ABSTRACTFORMWINDOW_H
#define TST_ABSTRACTFORMWINDOW_H

#include <qtestcase.h>

//TESTED_CLASS=
//TESTED_FILES=ide/src/sdk/abstractformwindow.h ide/src/sdk/abstractformwindow.cpp

class tst_QDesignerFormWindowInterface: public QTestCase
{
    Q_OBJECT
public:
    tst_QDesignerFormWindowInterface(int &argc, char *argv[]);
    virtual ~tst_QDesignerFormWindowInterface();

private slots:
    void dirty_data(QTestTable &t);
    void dirty();

    void mainContainer_data(QTestTable &t);
    void mainContainer();
};


#endif // TST_ABSTRACTFORMWINDOW_H
