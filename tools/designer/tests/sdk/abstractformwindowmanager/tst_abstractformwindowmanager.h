
#ifndef TST_ABSTRACTFORMWINDOWMANAGER_H
#define TST_ABSTRACTFORMWINDOWMANAGER_H

#include <qtestcase.h>

//TESTED_CLASS=
//TESTED_FILES=ide/src/sdk/abstractformwindow.h ide/src/sdk/abstractformwindow.cpp

class tst_AbstractFormWindowManager: public QTestCase
{
    Q_OBJECT
public:
    tst_AbstractFormWindowManager(int &argc, char *argv[]);
    virtual ~tst_AbstractFormWindowManager();

private slots:
    void createFormWindow();
    void formWindowCount();
    void formWindowAdded();
    void formWindowRemoved();
};


#endif // TST_ABSTRACTFORMWINDOWMANAGER_H
