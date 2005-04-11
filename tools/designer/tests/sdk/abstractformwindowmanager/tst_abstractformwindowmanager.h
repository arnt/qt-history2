
#ifndef TST_ABSTRACTFORMWINDOWMANAGER_H
#define TST_ABSTRACTFORMWINDOWMANAGER_H

#include <qtestcase.h>

//TESTED_CLASS=
//TESTED_FILES=ide/src/sdk/abstractformwindow.h ide/src/sdk/abstractformwindow.cpp

class tst_QDesignerFormWindowManagerInterface: public QTestCase
{
    Q_OBJECT
public:
    tst_QDesignerFormWindowManagerInterface(int &argc, char *argv[]);
    virtual ~tst_QDesignerFormWindowManagerInterface();

private slots:
    void createFormWindow();
    void formWindowCount();
    void formWindowAdded();
    void formWindowRemoved();
};


#endif // TST_ABSTRACTFORMWINDOWMANAGER_H
